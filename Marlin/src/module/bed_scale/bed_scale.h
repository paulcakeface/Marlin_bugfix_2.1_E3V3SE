#pragma once
#include "../../inc/MarlinConfig.h"

#if ENABLED(BED_SCALE)

struct BedScaleState {
  int32_t offset_raw = 0;     // TARE raw
  float   scale_g = 0.0f;     // grams per count (puede ser negativo)
  bool    enabled = false;
  bool    inited  = false;
};

extern BedScaleState bed_scale;

void bed_scale_init();
void bed_scale_enable(bool on);

int32_t bed_scale_read_raw_avg(uint8_t samples = 8);
void    bed_scale_tare(uint8_t samples = 16);
void    bed_scale_calibrate(float known_g, uint8_t samples = 16);
float   bed_scale_read_g(uint8_t samples = 8);

#endif
