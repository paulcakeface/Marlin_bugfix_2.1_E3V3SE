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
  /*drop_threshold_g*/ 20.0f,
  /*drop_confirm_n*/ 1,
  /*drop_ignore_layers*/ 3,
  /*drop_cooldown_layers*/ 20,

  /*last_layer*/ 0,
  /*last_g_est*/ 0,
  /*have_last_g*/ false,
  /*last_delta*/ 0,

  /*drop_hits*/ 0,
  /*cooldown*/ 0,
  /*drop_tripped*/ false,

  /*window_g_ref*/ 0.0f,
  /*window_layer_ref*/ 0,
  /*drop_window_layers*/ 2,

  // (delta, grams, set)
  /*p*/ { {0,0,false}, {0,0,false}, {0,0,false} }
};

static inline int32_t hx711_read_once() {
  return (int32_t)cell.hx711.getVal(false);
}

void bed_scale_init() {
  if (bed_scale.inited) return;
  cell.hx711.init(HX711_SCK_PIN, HX711_SDO_PIN);
  bed_scale.inited = true;
}

void bed_scale_enable(bool on) {
  bed_scale_init();
  bed_scale.enabled = on;
}

// Simple in-place sort for small N
static void sort_i32(int32_t *a, uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    for (uint8_t j = i + 1; j < n; j++) {
      if (a[i] > a[j]) {
        const int32_t t = a[i];
        a[i] = a[j];
        a[j] = t;
      }
    }
  }
}

// Robust average: trimmed mean
// - Collect N samples
// - Sort
// - Drop extremes (25% each side for N>=16, else 1 each side for N>=6)
// - Average the middle
int32_t bed_scale_read_raw_avg(uint8_t samples) {
  bed_scale_init();

  int64_t acc = 0;
  uint8_t got = 0;

  for (uint8_t i = 0; i < samples; i++) {
    const int32_t v = (int32_t)cell.hx711.getVal(false);
    if (v == INT32_MIN) continue;  // discard invalid
    acc += v;
    got++;
  }

  // if no valid readings, report invalid
  if (!got) return INT32_MIN;

  return (int32_t)(acc / got);
}

int32_t bed_scale_read_delta_avg(uint8_t samples) {
  const int32_t raw = bed_scale_read_raw_avg(samples);
  if (raw == INT32_MIN) return INT32_MIN;
  return raw - bed_scale.offset_raw;
}


void bed_scale_tare(uint8_t samples) {
  bed_scale_init();

  const int32_t raw = bed_scale_read_raw_avg(samples);
  if (raw == INT32_MIN) {
    SERIAL_ECHOLNPGM("BS TARE failed (invalid raw)");
    return;
  }

  bed_scale.offset_raw = raw;

  bed_scale.have_last_g = false;
  bed_scale.last_g_est = 0;
  bed_scale.last_delta = 0;
  bed_scale.drop_hits = 0;
  bed_scale.cooldown = 0;
  bed_scale.drop_tripped = false;
}


// Save calibration point
bool bed_scale_set_point(uint8_t idx, float grams, uint8_t samples) {
  if (idx < 1 || idx > 3) return false;
  if (grams <= 0) return false;

  const int32_t delta = bed_scale_read_delta_avg(samples);
  if (delta == INT32_MIN) return false;

  CalPoint &cp = bed_scale.p[idx - 1];
  cp.delta = delta;
  cp.grams = grams;
  cp.set = true;

  return true;
}


// -------- Estimator (DESCENDING by delta) --------
static inline bool bs_ok(const CalPoint &p) { return p.set && p.grams > 0; }
static inline void bs_swap(CalPoint &a, CalPoint &b) { CalPoint t = a; a = b; b = t; }

static inline float bs_lerp(const float x, const float x0, const float y0, const float x1, const float y1) {
  if (x1 == x0) return y0;
  return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

float bed_scale_estimate_grams_from_delta(const int32_t delta) {

  // Build compact list: anchor + valid points
  CalPoint v[4];
  v[0] = { 0, 0.0f, true };

  uint8_t k = 1;
  for (uint8_t i = 0; i < 3; i++)
    if (bs_ok(bed_scale.p[i]))
      v[k++] = bed_scale.p[i];

  if (k < 2) return 0.0f;

  // Sort by delta DESC: 0, -9k, -16k, -25k
  for (uint8_t i = 0; i < k; i++)
    for (uint8_t j = i + 1; j < k; j++)
      if (v[i].delta < v[j].delta)
        bs_swap(v[i], v[j]);

  // Outlier guard (optional but safe): if delta magnitude is crazy, keep last
  const int32_t most_negative = v[k - 1].delta;
  if (most_negative != 0) {
    const int32_t abs_heavy = ABS(most_negative);
    const int32_t abs_d     = ABS(delta);
    if (abs_d > abs_heavy * 8L) {
      return bed_scale.last_g_est;
    }
  }

  // Light clamp
  if (delta >= 0) return 0.0f;

  // Heavy clamp
  if (delta <= v[k - 1].delta) return v[k - 1].grams;

  // Interpolate between segment where: a.delta >= delta >= b.delta
  for (uint8_t i = 0; i < k - 1; i++) {
    const CalPoint &a = v[i];
    const CalPoint &b = v[i + 1];

    if (delta <= a.delta && delta >= b.delta) {
      return bs_lerp((float)delta, (float)a.delta, a.grams, (float)b.delta, b.grams);
    }
  }

  return 0.0f;
}

#endif // ENABLED(BED_SCALE)
