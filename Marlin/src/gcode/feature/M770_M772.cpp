/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware
 *
 * M770 - Load-cell scale calibration workflow
 * M771 - Load-cell scale reset
 * M772 - Load-cell scale read
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(USE_AUTOZ_TOOL_2)

#include "../gcode.h"
#include "../../feature/loadcell_scale.h"

static uint8_t parse_samples() {
  int n = parser.seenval('N') ? parser.value_int() : 24;
  if (n < 7) n = 7;
  if (n > 63) n = 63;
  return (uint8_t)n;
}

/**
 * M770: Calibrate load-cell scale
 *
 *  T      - Tare (required before calibration points)
 *  W<g>   - Capture one known weight point in grams (repeat 3 times with different weights)
 *  N<n>   - Optional sample count (7..63), default 24
 *
 * Examples:
 *  M770 T
 *  M770 W100
 *  M770 W250
 *  M770 W500
 */
void GcodeSuite::M770() {
  const uint8_t samples = parse_samples();
  const bool do_tare = parser.seen('T');
  const bool do_weight = parser.seenval('W');

  if (do_tare) {
    if (loadcell_scale.tare(samples))
      SERIAL_ECHOLNPGM("M770: tare complete.");
    else
      SERIAL_ECHOLNPGM("M770: tare failed.");
  }

  if (do_weight) {
    const float grams = parser.value_float();
    if (!(grams > 0)) {
      SERIAL_ECHOLNPGM("M770: W must be > 0.");
      return;
    }
    if (loadcell_scale.add_calibration_point(grams, samples)) {
      SERIAL_ECHOLNPGM("M770: captured W=", getStr(grams), "g");
      if (loadcell_scale.is_ready())
        SERIAL_ECHOLNPGM("M770: calibration ready.");
      else
        SERIAL_ECHOLNPGM("M770: add more weights (3 total after tare).");
    }
    else {
      SERIAL_ECHOLNPGM("M770: capture failed (tare first, use 3 different weights, or reset with M771).");
    }
  }

  if (!do_tare && !do_weight)
    loadcell_scale.report();
}

/**
 * M771: Reset load-cell scale calibration
 */
void GcodeSuite::M771() {
  loadcell_scale.reset();
  SERIAL_ECHOLNPGM("M771: load-cell scale reset.");
}

/**
 * M772: Read current weight
 *
 *  N<n> - Optional sample count (7..63), default 24
 */
void GcodeSuite::M772() {
  const uint8_t samples = parse_samples();
  float grams = 0, raw = 0;
  if (!loadcell_scale.read_weight(grams, raw, samples)) {
    SERIAL_ECHOLNPGM("M772: not calibrated. Run M770 T then M770 W<g> x3.");
    return;
  }

  SERIAL_ECHOLNPGM("M772: RAW=", getStr(raw), "  WEIGHT=", getStr(grams), "g");
}

#endif // USE_AUTOZ_TOOL_2

