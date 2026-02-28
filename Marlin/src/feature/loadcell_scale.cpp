/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware
 *
 * Load-cell weight scale helper for boards that already use USE_AUTOZ_TOOL_2.
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(USE_AUTOZ_TOOL_2)

#include "loadcell_scale.h"
#include "../MarlinCore.h"
#include "../core/serial.h"

#include <math.h>

LoadCellScale loadcell_scale;

void LoadCellScale::sort_float(float *vals, const uint8_t n) {
  for (uint8_t i = 1; i < n; ++i) {
    const float key = vals[i];
    int8_t j = i - 1;
    while (j >= 0 && vals[j] > key) {
      vals[j + 1] = vals[j];
      --j;
    }
    vals[j + 1] = key;
  }
}

float LoadCellScale::median_of(float *vals, const uint8_t n) {
  if (!n) return 0;
  sort_float(vals, n);
  if (n & 1) return vals[n >> 1];
  return 0.5f * (vals[(n >> 1) - 1] + vals[n >> 1]);
}

bool LoadCellScale::sample_raw(float &raw, uint8_t sample_count) {
  if (sample_count < 7) sample_count = 7;
  if (sample_count > 63) sample_count = 63;

  if (!inited) {
    hx711.init(HX711_SCK_PIN, HX711_SDO_PIN);
    inited = true;
  }

  // Warm-up reads to settle the HX711 stream.
  for (uint8_t i = 0; i < 6; ++i) {
    (void)hx711.getVal(false);
    marlin.idle();
  }

  float vals[63];
  for (uint8_t i = 0; i < sample_count; ++i) {
    vals[i] = (float)hx711.getVal(false);
    marlin.idle();
  }

  float work[63];
  for (uint8_t i = 0; i < sample_count; ++i) work[i] = vals[i];
  const float med = median_of(work, sample_count);

  for (uint8_t i = 0; i < sample_count; ++i) work[i] = fabsf(vals[i] - med);
  const float mad = median_of(work, sample_count);
  const float gate = _MAX(3.0f * mad, 8.0f); // reject occasional spikes

  float sum = 0;
  uint8_t cnt = 0;
  for (uint8_t i = 0; i < sample_count; ++i) {
    if (fabsf(vals[i] - med) <= gate) {
      sum += vals[i];
      ++cnt;
    }
  }

  raw = cnt ? (sum / cnt) : med;
  return true;
}

void LoadCellScale::reset() {
  tared = false;
  calibrated = false;
  tare_raw = 0;
  point_count = 0;
  for (uint8_t i = 0; i < 3; ++i) {
    points[i].grams = 0;
    points[i].raw = 0;
  }
}

bool LoadCellScale::tare(const uint8_t sample_count) {
  float raw = 0;
  if (!sample_raw(raw, sample_count)) return false;
  reset();
  tare_raw = raw;
  tared = true;
  return true;
}

bool LoadCellScale::validate_points() const {
  if (!tared || point_count < 3) return false;

  // Points must be strictly increasing by known grams.
  if (!(points[1].grams > points[0].grams && points[2].grams > points[1].grams))
    return false;

  const float d01 = points[1].raw - points[0].raw;
  const float d12 = points[2].raw - points[1].raw;
  const float d0t = points[0].raw - tare_raw;

  // Require usable span and monotonic direction.
  if (fabsf(d01) < 20 || fabsf(d12) < 20 || fabsf(d0t) < 20) return false;
  const bool up = d01 > 0;
  if ((d12 > 0) != up) return false;
  if ((d0t > 0) != up) return false;

  return true;
}

bool LoadCellScale::add_calibration_point(const float grams, const uint8_t sample_count) {
  if (!tared || !(grams > 0)) return false;

  float raw = 0;
  if (!sample_raw(raw, sample_count)) return false;

  // Replace an existing close point by grams.
  for (uint8_t i = 0; i < point_count; ++i) {
    if (fabsf(points[i].grams - grams) <= 0.5f) {
      points[i].raw = raw;
      calibrated = (point_count == 3) && validate_points();
      return true;
    }
  }

  if (point_count >= 3) return false;

  // Insert sorted by grams.
  uint8_t pos = point_count;
  while (pos > 0 && points[pos - 1].grams > grams) {
    points[pos] = points[pos - 1];
    --pos;
  }
  points[pos].grams = grams;
  points[pos].raw = raw;
  ++point_count;

  calibrated = (point_count == 3) && validate_points();
  return true;
}

float LoadCellScale::grams_from_raw(const float raw) const {
  if (!is_ready()) return NAN;

  const float xr[4] = { tare_raw, points[0].raw, points[1].raw, points[2].raw };
  const float yg[4] = { 0.0f,     points[0].grams, points[1].grams, points[2].grams };

  const bool inc = xr[3] >= xr[0];
  uint8_t seg = 2; // Default to last segment

  if (inc) {
    if (raw <= xr[1]) seg = 0;
    else if (raw <= xr[2]) seg = 1;
    else seg = 2;
  }
  else {
    if (raw >= xr[1]) seg = 0;
    else if (raw >= xr[2]) seg = 1;
    else seg = 2;
  }

  const float x0 = xr[seg], x1 = xr[seg + 1];
  const float y0 = yg[seg], y1 = yg[seg + 1];
  const float dx = x1 - x0;
  if (fabsf(dx) < 0.0001f) return y0;

  float grams = y0 + (raw - x0) * (y1 - y0) / dx;
  if (grams < 0) grams = 0; // Tare noise can produce tiny negatives
  return grams;
}

bool LoadCellScale::read_weight(float &grams, float &raw, const uint8_t sample_count) {
  if (!is_ready()) return false;
  if (!sample_raw(raw, sample_count)) return false;
  grams = grams_from_raw(raw);
  return !isnan(grams) && !isinf(grams);
}

void LoadCellScale::report() const {
  SERIAL_ECHOLNPGM("M770 LoadCellScale report:");
  SERIAL_ECHOLNPGM("  tared=", tared ? "1" : "0",
                   " calibrated=", calibrated ? "1" : "0",
                   " points=", point_count);
  if (tared)
    SERIAL_ECHOLNPGM("  tare_raw=", getStr(tare_raw));
  for (uint8_t i = 0; i < point_count; ++i)
    SERIAL_ECHOLNPGM("  p", i + 1, ": W=", getStr(points[i].grams), "g raw=", getStr(points[i].raw));
}

#endif // USE_AUTOZ_TOOL_2

