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
// M9000 L<layers> T<sec> P<0..100>
// M9000 F1               -> Print Finished
// ---------------------------------------------------------------------------
void GcodeSuite::M9000() {
  // A1 -> Connected
  if (parser.seen('A')) {
    serial_connection_active = parser.value_bool();
    if (serial_connection_active) Goto_MainMenu(); // Go to main menu on connect
  }

  // L/T/P -> Set Layers/Time/Progress
  if (parser.seen('L')) { ui.set_total_layers(parser.value_int()); }
  if (parser.seen('T')) { ui.set_print_time(60 * parser.value_ulong()); }
  if (parser.seen('P')) { 
    ui.set_progress((PROGRESS_SCALE) > 1
        ? parser.value_float() * (PROGRESS_SCALE)
        : parser.value_byte()
      );  
  }

  // S1 -> clear LCD + render print window
  if (parser.boolval('S')) {    
    Clear_Thumb_UpperArea();
    Goto_ThumbPrint();
  }else if (parser.boolval('F'))
  {
    Goto_ThumbFinish();
  }
  
}// End M9000


// ---------------------------------------------------------------------------
// M9002: Receive Octoprint Thumbnail Image Data
// M9002 START            -> Initialize image reception
// M9002 END              -> Finalize image transmission
// M9002 CHUNK<line>,<pixel_offset>|<color1>,<color2>,...,<colorN>
//    -> Send a chunk of pixel data for the specified line starting at pixel_offset
// ---------------------------------------------------------------------------
static uint16_t current_line = 0;  // Track the current line
static uint16_t received_pixels = 0; // Track received pixels for the line

void GcodeSuite::M9001() {
    
    if (parser.string_arg && parser.string_arg[0] != '\0') {
        // START: Initialize the image reception
        if (strncmp(parser.string_arg, "START", 5) == 0) {
            initializeImageMap();
            Clear_Title_Bar();
            Draw_OctoTitle("Receiving Thumbnail, wait...");
            current_line = 0;
            received_pixels = 0;
            SERIAL_ECHOLN("M9001 START: Ready to receive lines.");
            return;
        }

        // END: Finalize the image transmission
        if (strcmp(parser.string_arg, "END") == 0) {
            SERIAL_ECHOLN("M9001 thumbnail-rendered");
            Clear_Title_Bar();
            return;
        }

        // CHUNK: Receive a portion of a line
        if (strncmp(parser.string_arg, "CHUNK", 5) == 0) {
            char *arg = parser.string_arg + 5;
            while (*arg && isspace(*arg)) arg++;

            // Tokenize the string for chunk parsing
            char *token = strtok(arg, ",");
            if (token != nullptr) {
                uint16_t line_number = atoi(token);

                // Tokenize for pixel offset
                token = strtok(nullptr, "|");
                if (token != nullptr) {
                    uint16_t pixel_offset = atoi(token);

                    // Start parsing the pixel data
                    token = strtok(nullptr, ",");
                    while (token != nullptr && pixel_offset < OctoIMAGE_WIDTH) {
                        // Ensure proper color value assignment
                        OctoImageLine[pixel_offset] = static_cast<uint16_t>(atoi(token));
                        received_pixels++;
                        pixel_offset++;

                        token = strtok(nullptr, ",");
                    }

                    // Debugging output for checking chunk data
                    // SERIAL_ECHOLNPAIR("O9002 CHUNK: Received line ", line_number);
                    // SERIAL_ECHOLNPAIR("Current received_pixels: ", received_pixels);

                    // Check if entire line has been received
                    if (received_pixels >= OctoIMAGE_WIDTH) {
                        // Call render function for the line
                        DWIN_RenderOctoLine(line_number);
                        received_pixels = 0; // Reset for next line
                    }
                }
            }
            return;
        }
    }
}// end M9001




#endif // OCTOPRINT_PLUGIN