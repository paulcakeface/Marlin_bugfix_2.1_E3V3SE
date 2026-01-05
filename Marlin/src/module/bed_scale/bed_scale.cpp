#include "../../inc/MarlinConfig.h"
#include "bed_scale.h"
#include "../AutoOffset.h"  
#if ENABLED(BED_SCALE)

ProbeAcq cell;
BedScaleState bed_scale;

void bed_scale_init() {
  if (bed_scale.inited) return;

  // init HX711 
  cell.hx711.init(HX711_SCK_PIN, HX711_SDO_PIN);
  bed_scale.inited = true;
}

void bed_scale_enable(bool on) {
  bed_scale_init();
  bed_scale.enabled = on;
}

int32_t bed_scale_read_raw_avg(uint8_t samples) {
  bed_scale_init();

  int64_t acc = 0;
  for (uint8_t i = 0; i < samples; i++) {
    acc += (int32_t)cell.hx711.getVal(false);
  }
  return (int32_t)(acc / samples);
}

void bed_scale_tare(uint8_t samples) {
  bed_scale_init();
  bed_scale.offset_raw = bed_scale_read_raw_avg(samples);
}

void bed_scale_calibrate(float known_g, uint8_t samples) {
  bed_scale_init();
  if (known_g <= 0) return;

  const int32_t raw = bed_scale_read_raw_avg(samples);
  const int32_t delta = raw - bed_scale.offset_raw;
  if (!delta) return;


  bed_scale.scale_g = known_g / (float)delta; // grams per count
}

float bed_scale_read_g(uint8_t samples) {
  bed_scale_init();

  if (bed_scale.scale_g == 0) return 0;

  const int32_t raw = bed_scale_read_raw_avg(samples);
  return (raw - bed_scale.offset_raw) * bed_scale.scale_g;
}

#endif
