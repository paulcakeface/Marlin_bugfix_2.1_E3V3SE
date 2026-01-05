#include "../../../inc/MarlinConfig.h"

#if ENABLED(BED_SCALE)

#include "../../gcode.h"
#include "../../../module/bed_scale/bed_scale.h"

/**
 *  M770 - Report raw + grams
 *  M770        -> default avg 8
 *  M770 A16    -> avg 16 samples
 */
void GcodeSuite::M770() {
  const uint8_t samples = (uint8_t)parser.byteval('A', 8);

  const int32_t raw = bed_scale_read_raw_avg(samples);
  const float g = bed_scale_read_g(samples);

  SERIAL_ECHOPGM("BS raw=");
  SERIAL_ECHO(raw);
  SERIAL_ECHOPGM(" offset=");
  SERIAL_ECHO(bed_scale.offset_raw);
  SERIAL_ECHOPGM(" scale=");
  SERIAL_ECHO(bed_scale.scale_g, 9);
  SERIAL_ECHOPGM(" g=");
  SERIAL_ECHOLN(g, 3);
}

/**
 *  M771 - TARE
 *  M771        -> default avg 16
 *  M771 A32
 */
void GcodeSuite::M771() {
  const uint8_t samples = (uint8_t)parser.byteval('A', 16);

  bed_scale_tare(samples);

  SERIAL_ECHOPGM("BS TARE ok offset_raw=");
  SERIAL_ECHOLN(bed_scale.offset_raw);
}

/**
 *  M772 - Calibrate with known weight (grams)
 *  M772 S500
 *  M772 S200 A32
 */
void GcodeSuite::M772() {
  if (!parser.seen('S')) {
    SERIAL_ECHOLNPGM("BS CAL missing S<grams> (example: M772 S500)");
    return;
  }

  const float known_g = parser.value_float();
  const uint8_t samples = (uint8_t)parser.byteval('A', 16);

  bed_scale_calibrate(known_g, samples);

  SERIAL_ECHOPGM("BS CAL ok known_g=");
  SERIAL_ECHO(known_g, 1);
  SERIAL_ECHOPGM(" scale_g=");
  SERIAL_ECHOLN(bed_scale.scale_g, 9);
}

#endif
