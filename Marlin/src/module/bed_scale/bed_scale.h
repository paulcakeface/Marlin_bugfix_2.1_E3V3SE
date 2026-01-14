#pragma once
#include "../../inc/MarlinConfig.h"

#if ENABLED(BED_SCALE)



// ---- Drop detection config/state ----
float drop_threshold_g = 25.0f;     // threshold: minimum drop to suspect (adjustable)
uint8_t drop_confirm_n = 2;         // how many consecutive layers to confirm
uint16_t drop_ignore_layers = 3;    // ignore first layers (due to vibration/initial adhesion)
uint16_t drop_cooldown_layers = 20; // after pausing, do not trigger again quickly

uint16_t last_layer = 0;
float last_g_est = 0;
bool have_last_g = false;

uint8_t drop_hits = 0;
uint16_t cooldown = 0;
bool drop_tripped = false;

struct CalPoint {
  int32_t delta = 0;   // raw -offset_raw (can be negative)
  float   grams = 0;   // known weight
  bool    set = false;
};

struct BedScaleState {
  int32_t offset_raw = 0;
  bool    enabled = false;
  bool    inited  = false;

  // For reporting during print job
  int32_t last_delta = 0;
  bool    have_last  = false;

  // 3 calibration points
  CalPoint p[3];
};

extern BedScaleState bed_scale;


void bed_scale_init();
int32_t bed_scale_read_raw_avg(uint8_t samples = 8);     // returns INT32_MIN if invalid
int32_t bed_scale_read_delta_avg(uint8_t samples = 8);   // raw - offset
void bed_scale_tare(uint8_t samples = 16);

// Save point (idx = 1..3)
bool bed_scale_set_point(uint8_t idx, float grams, uint8_t samples = 16);

// Estimate grams using piecewise curve (requires at least 2 points)
float bed_scale_estimate_grams_from_delta(int32_t delta);

// Utility: sorts points by delta
void bed_scale_sort_points();

// Check for drop and request pause
bool bed_scale_check_drop_and_pause(const uint16_t layer, const float g_est);

#endif
