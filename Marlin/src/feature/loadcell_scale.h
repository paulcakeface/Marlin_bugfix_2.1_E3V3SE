/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware
 *
 * Load-cell weight scale helper for boards that already use USE_AUTOZ_TOOL_2.
 */

#pragma once

#include "../inc/MarlinConfigPre.h"

#if ENABLED(USE_AUTOZ_TOOL_2)

#include "../module/AutoOffset.h" // Reuse HX711 + getStr helpers

class LoadCellScale {
  public:
    void reset();
    bool tare(const uint8_t sample_count=24);
    bool add_calibration_point(const float grams, const uint8_t sample_count=24);
    bool read_weight(float &grams, float &raw, const uint8_t sample_count=24);
    void report() const;
    bool is_ready() const { return tared && calibrated && point_count == 3; }

  private:
    struct CalPoint {
      float grams;
      float raw;
    };

    HX711 hx711;
    bool inited = false;
    bool tared = false;
    bool calibrated = false;
    float tare_raw = 0;
    CalPoint points[3] = {};
    uint8_t point_count = 0;

    bool sample_raw(float &raw, uint8_t sample_count);
    bool validate_points() const;
    float grams_from_raw(const float raw) const;

    static void sort_float(float *vals, const uint8_t n);
    static float median_of(float *vals, const uint8_t n);
};

extern LoadCellScale loadcell_scale;

#endif // USE_AUTOZ_TOOL_2

