#pragma once
#include "../../inc/MarlinConfig.h"

#if ENABLED(BED_SCALE)

struct CalPoint {
  int32_t delta;
  float grams;
  bool set;
};

struct BedScaleState {
  int32_t offset_raw;
  bool enabled;
  bool inited;

  // --- Drop detection ---
  float drop_threshold_g;
  uint8_t drop_confirm_n;
  uint16_t drop_ignore_layers;
  uint16_t drop_cooldown_layers;

  uint16_t last_layer;
  float last_g_est;
  bool have_last_g;
  int32_t last_delta;

  uint8_t drop_hits;
  uint16_t cooldown;
  bool drop_tripped;

  // 3 puntos
  CalPoint p[3];
};

extern BedScaleState bed_scale;

void bed_scale_init();
int32_t bed_scale_read_raw_avg(uint8_t samples = 8);
void bed_scale_tare(uint8_t samples = 16);

bool bed_scale_set_point(uint8_t idx, float grams, uint8_t samples = 16);
float bed_scale_estimate_grams_from_delta(int32_t delta);

bool bed_scale_check_drop_and_pause(const uint16_t layer, const float g_est);

#endif
