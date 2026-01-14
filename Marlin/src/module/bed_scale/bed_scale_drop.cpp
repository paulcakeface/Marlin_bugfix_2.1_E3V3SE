#include "../../inc/MarlinConfig.h"

#if ENABLED(BED_SCALE)

#include "bed_scale.h"
#include "../../gcode/queue.h"

static void bed_scale_request_pause() {
    
}

bool bed_scale_check_drop_and_pause(const uint16_t layer, const float g_est) {
  // Debe estar calibrado y tener un valor previo
  if (!bed_scale.have_last_g) {
    bed_scale.last_g_est = g_est;
    bed_scale.have_last_g = true;
    bed_scale.last_layer = layer;
    return false;
  }

  // Ignora primeras capas
  if (layer <= bed_scale.drop_ignore_layers) {
    bed_scale.last_g_est = g_est;
    bed_scale.last_layer = layer;
    bed_scale.drop_hits = 0;
    return false;
  }

  // Cooldown para evitar spam
  if (bed_scale.cooldown) {
    bed_scale.cooldown--;
    bed_scale.last_g_est = g_est;
    bed_scale.last_layer = layer;
    bed_scale.drop_hits = 0;
    return false;
  }

  // Si ya disparó, no repitas
  if (bed_scale.drop_tripped) return true;

  // Caída entre capas
  const float drop_g = bed_scale.last_g_est - g_est;  // positiva = bajó el "peso visto"

  // Heurística: caída fuerte
  if (drop_g >= bed_scale.drop_threshold_g) {
    bed_scale.drop_hits++;
  }
  else {
    // Si no se confirma, resetea hits (o baja 1 si quieres más suave)
    bed_scale.drop_hits = 0;
  }

  // Actualiza memoria
  bed_scale.last_g_est = g_est;
  bed_scale.last_layer = layer;

  // Confirmación N veces seguidas
  if (bed_scale.drop_hits >= bed_scale.drop_confirm_n) {
    bed_scale.drop_tripped = true;
    bed_scale.cooldown = bed_scale.drop_cooldown_layers;

    SERIAL_ECHOPGM("BS DROP DETECTED on layer ");
    SERIAL_ECHO((int)layer);
    SERIAL_ECHOPGM(" drop_threshold_g=");
    SERIAL_ECHO(bed_scale.drop_threshold_g, 1);
    SERIAL_ECHOLNPGM(" -> PAUSE");

    bed_scale_request_pause();
    return true;
  }

  return false;
}

#endif
