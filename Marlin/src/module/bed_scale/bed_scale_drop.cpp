#include "../../inc/MarlinConfig.h"

#if ENABLED(BED_SCALE)

#include "bed_scale.h"
#include "../../gcode/queue.h"

static void bed_scale_request_pause() {
    #if ENABLED(OCTOPRINT_PLUGIN)
      SERIAL_ECHOLNPGM("M9000 pause-job-bs"); // Octoprint Plugin pause command
    #else
      queue.inject_P(PSTR("M25")); // Pause SD print
    #endif
}

bool bed_scale_check_drop_and_pause(const uint16_t layer, const float g_est) {

  // We need history
  if (!bed_scale.have_last_g) {
    bed_scale.last_g_est = g_est;
    bed_scale.have_last_g = true;
    bed_scale.last_layer = layer;

    // init window
    bed_scale.window_g_ref = g_est;
    bed_scale.window_layer_ref = layer;
    return false;
  }

  // Ignore first layers
  if (layer <= bed_scale.drop_ignore_layers) {
    bed_scale.last_g_est = g_est;
    bed_scale.last_layer = layer;
    bed_scale.drop_hits = 0;

    bed_scale.window_g_ref = g_est;
    bed_scale.window_layer_ref = layer;
    return false;
  }

  // Cooldown
  if (bed_scale.cooldown) {
    bed_scale.cooldown--;
    bed_scale.last_g_est = g_est;
    bed_scale.last_layer = layer;
    bed_scale.drop_hits = 0;

    // refresh window also to avoid triggering when exiting cooldown
    bed_scale.window_g_ref = g_est;
    bed_scale.window_layer_ref = layer;
    return false;
  }

  // Instant drop
  const float drop_inst = bed_scale.last_g_est - g_est;

  // Accumulated drop in window
  // if N layers have passed since the reference, move the window
  if ((uint16_t)(layer - bed_scale.window_layer_ref) >= bed_scale.drop_window_layers) {
    bed_scale.window_g_ref = bed_scale.last_g_est;          
    bed_scale.window_layer_ref = bed_scale.last_layer;      
  }
  const float drop_win = bed_scale.window_g_ref - g_est;

  // Decide if it counts as a hit
  const bool hit = (drop_inst >= bed_scale.drop_threshold_g) || (drop_win >= bed_scale.drop_threshold_g);

  if (hit) bed_scale.drop_hits++;
  else     bed_scale.drop_hits = 0;

  // Update memory
  bed_scale.last_g_est = g_est;
  bed_scale.last_layer = layer;

  // Confirmation
  if (bed_scale.drop_hits >= bed_scale.drop_confirm_n) {
    bed_scale.drop_tripped = true;
    bed_scale.cooldown = bed_scale.drop_cooldown_layers;

    SERIAL_ECHOPGM("BedScale DROP DETECTED on layer ");
    SERIAL_ECHO((int)layer);
    SERIAL_ECHOPGM(" drop_inst=");
    SERIAL_ECHO(drop_inst, 2);
    SERIAL_ECHOPGM(" drop_win=");
    SERIAL_ECHO(drop_win, 2);
    SERIAL_ECHOPGM(" thr=");
    SERIAL_ECHOLN(bed_scale.drop_threshold_g, 2);

    bed_scale_request_pause();
    return true;
  }

  return false;
}


#endif
