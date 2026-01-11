#include "../../inc/MarlinConfig.h"
#include "../../MarlinCore.h"
#include "../gcode.h"
#include "../parser.h"
#include "../../lcd/e3v2/creality/dwin.h"
#include "../../lcd/marlinui.h"

#include <stdio.h>
#include <string.h>

#if ENABLED(OCTOPRINT_PLUGIN)
// ---------------------------------------------------------------------------
// M9000: Set Octoprint Connection and Print Details
// M9000 A1               -> Connected
// M9000 S1               -> Clear LCD + render print window
// M9000 L<layers> T<min> P<0..100>
// M9000 F1               -> Print Finished
// ---------------------------------------------------------------------------
void GcodeSuite::M9000()
{
  // A1 -> Connected
  if (parser.seen('A'))
  {
    serial_connection_active = parser.value_bool();
    if (serial_connection_active)
      Goto_MainMenu(); // Go to main menu on connect
  }

  // L/T/P -> Set Layers/Time/Progress
  if (parser.seen('L'))
  {
    ui.set_total_layers(parser.value_int());
  }
  if (parser.seen('T'))
  {
    ui.set_print_time(60 * parser.value_ulong());
  }
  if (parser.seen('P'))
  {
    ui.set_progress((PROGRESS_SCALE) > 1
                        ? parser.value_float() * (PROGRESS_SCALE)
                        : parser.value_byte());
  }

  // S1 -> clear LCD + render print window
  if (parser.boolval('S'))
  {
    Clear_Thumb_UpperArea();
    Goto_ThumbPrint();
  }
  else if (parser.boolval('F'))
  {
    Goto_ThumbFinish();
  }

} // End M9000


// Function to convert a single hex character to its integer value
inline uint8_t hex2val(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}
// Helper function to skip spaces
static inline void skip_spaces(char *&p) { while (*p && isspace(*p)) p++; }
// Helper function to parse an unsigned 16-bit integer from a string
static inline uint16_t parse_u16(char *&p) {
  uint16_t v = 0;
  while (*p && isdigit(*p)) { v = v * 10 + (*p - '0'); p++; }
  return v;
}

// ---------------------------------------------------------------------------
// M9002: Receive Octoprint Thumbnail Image Data
// M9002 START            -> Initialize image reception
// M9002 END              -> Finalize image transmission
// M9002 C <line>,<pixel_offset>|<color1>,<color2>,...,<colorN>
//    -> Send a chunk of pixel data for the specified line starting at pixel_offset
// ---------------------------------------------------------------------------
void GcodeSuite::M9001() {
  uint16_t pixel_count = 0; // Track drawn pixels for batched delay
  char *p = parser.command_ptr;
  if (!p) return;

  // Move p to after "M9001"
  while (*p && !isspace(*p)) p++;   // skip "M9001"
  skip_spaces(p);                  // now points to 'C' / 'START' / 'END'

  if (!*p) return;

  // START
  if (strncmp(p, "START", 5) == 0) {
    initializeImageMap();
    SERIAL_ECHOLNPGM("M9001 START");
    return;
  }

  // END
  if (strncmp(p, "END", 3) == 0) {
    SERIAL_ECHOLNPGM("M9001 END");
    SERIAL_ECHOLN("M9001 thumbnail-rendered");
    Clear_Title_Bar();
    return;
  }

  // C <line> <offset> <hex>
  if (*p == 'C') {
    p++; skip_spaces(p);

    const uint16_t line_number = parse_u16(p);
    skip_spaces(p);

    uint16_t pixel_offset = parse_u16(p);
    skip_spaces(p);

    const uint16_t start_offset = pixel_offset;
    uint16_t decoded = 0;

    while (pixel_offset < 96) {
      if (!p[0] || !p[1] || !p[2] || !p[3]) break;

      const uint16_t color =
        (uint16_t(hex2val(p[0])) << 12) |
        (uint16_t(hex2val(p[1])) <<  8) |
        (uint16_t(hex2val(p[2])) <<  4) |
        (uint16_t(hex2val(p[3])));

      OctoImageLine[pixel_offset] = color;

      pixel_offset++;
      decoded++;
      p += 4;
    }

    // Render only if line is complete
    if (start_offset + decoded >= 96) {
      DWIN_RenderOctoLine(line_number);
    }
    
    return;
  }
}



#endif // OCTOPRINT_PLUGIN