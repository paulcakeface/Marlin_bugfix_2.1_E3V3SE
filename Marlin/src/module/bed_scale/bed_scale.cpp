#include "../../inc/MarlinConfig.h"
#include "bed_scale.h"
#include "../AutoOffset.h"  

#if ENABLED(BED_SCALE)
ProbeAcq cell;


BedScaleState bed_scale = {
  /*offset_raw*/ 0,
  /*enabled*/ false,
  /*inited*/ false,

  // drop defaults (ajustables)
  /*drop_threshold_g*/ 25.0f,
  /*drop_confirm_n*/ 2,
  /*drop_ignore_layers*/ 3,
  /*drop_cooldown_layers*/ 20,

  /*last_layer*/ 0,
  /*last_g_est*/ 0,
  /*have_last_g*/ false,
  /*last_delta*/ 0,

  /*drop_hits*/ 0,
  /*cooldown*/ 0,
  /*drop_tripped*/ false,

  // (delta, grams, set)
  /*p*/ { {0,0,false}, {0,0,false}, {0,0,false} }
};

// ----HX711 safety: if it is NOT ready, do not return garbage ----
static int32_t hx711_read_once() {
  const int32_t v = (int32_t)cell.hx711.getVal(false);
  return v;
}

void bed_scale_init() {
  if (bed_scale.inited) return;
  cell.hx711.init(HX711_SCK_PIN, HX711_SDO_PIN);
  bed_scale.inited = true;
}

int32_t bed_scale_read_raw_avg(uint8_t samples) {
  bed_scale_init();

  int64_t acc = 0;
  uint8_t got = 0;

  for (uint8_t i = 0; i < samples; i++) {
    const int32_t v = hx711_read_once();
    // HX711 read failed
    acc += v;
    got++;
  }

  return got ? (int32_t)(acc / got) : INT32_MIN;
}

 // raw - offset
int32_t bed_scale_read_delta_avg(uint8_t samples) {
  const int32_t raw = bed_scale_read_raw_avg(samples);
  if (raw == INT32_MIN) return INT32_MIN;
  return raw - bed_scale.offset_raw;
}

// TARE
void bed_scale_tare(uint8_t samples) {
  bed_scale_init();
  const int32_t raw = bed_scale_read_raw_avg(samples);
  if (raw != INT32_MIN) {
    bed_scale.offset_raw = raw;
    bed_scale.have_last_g = false;
  }
}

// Utility function to swap two calibration points
static void swap_points(CalPoint &a, CalPoint &b) { CalPoint t = a; a = b; b = t; }

// Sort calibration points by delta
void bed_scale_sort_points() {
  // sort by ascending delta
  for (uint8_t i = 0; i < 3; i++)
    for (uint8_t j = i + 1; j < 3; j++)
      if (bed_scale.p[i].set && bed_scale.p[j].set && bed_scale.p[i].delta > bed_scale.p[j].delta)
        swap_points(bed_scale.p[i], bed_scale.p[j]);
}

// Save point (idx = 1..3)
bool bed_scale_set_point(uint8_t idx, float grams, uint8_t samples) {
  if (idx < 1 || idx > 3) return false;
  if (grams <= 0) return false;

  // Read delta
  const int32_t delta = bed_scale_read_delta_avg(samples);
  if (delta == INT32_MIN) return false;

  CalPoint &cp = bed_scale.p[idx - 1];
  cp.delta = delta;
  cp.grams = grams;
  cp.set = true;

  bed_scale_sort_points();
  return true;
}

static bool have_2_points(CalPoint &a, CalPoint &b) {
  return a.set && b.set && a.delta != b.delta && a.grams > 0 && b.grams > 0;
}


// Linear interpolation helper
static float lerp(float x, float x0, float y0, float x1, float y1) {
  return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

// Estimate grams using piecewise curve (requires at least 2 points)
float bed_scale_estimate_grams_from_delta(int32_t delta) {
  // We need at least 2 points
  CalPoint &p0 = bed_scale.p[0];
  CalPoint &p1 = bed_scale.p[1];
  CalPoint &p2 = bed_scale.p[2];

  // Case: only 2 points (p0 and p1)
  if (have_2_points(p0, p1) && !p2.set) {
    // clamp
    if ((delta <= p0.delta && p0.delta <= p1.delta) || (delta >= p0.delta && p0.delta >= p1.delta))
      return p0.grams;
    if ((delta >= p1.delta && p0.delta <= p1.delta) || (delta <= p1.delta && p0.delta >= p1.delta))
      return p1.grams;
    return lerp((float)delta, (float)p0.delta, p0.grams, (float)p1.delta, p1.grams);
  }

  // Case: 3 points (better)
  if (have_2_points(p0, p1) && have_2_points(p1, p2)) {

    // clamp at extremes
    if (delta <= p0.delta) return p0.grams;
    if (delta >= p2.delta) return p2.grams;

    // slice 1
    if (delta <= p1.delta)
      return lerp((float)delta, (float)p0.delta, p0.grams, (float)p1.delta, p1.grams);

    // slice 2
    return lerp((float)delta, (float)p1.delta, p1.grams, (float)p2.delta, p2.grams);
  }

  // If there are not enough valid points
  return 0;
}

#endif
