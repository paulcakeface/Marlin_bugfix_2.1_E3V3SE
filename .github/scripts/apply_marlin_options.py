#!/usr/bin/env python3
import argparse
import re
from pathlib import Path

def set_define(path: Path, macro: str, enabled: bool):
    text = path.read_text()
        
    #   #define MACRO
    #   //#define MACRO
    #   // #define MACRO
    #   #define MACRO //comentario...
    pattern = re.compile(
        r'^(\s*)(//\s*)?#define\s+' + re.escape(macro) + r'(\b.*)$',
        re.MULTILINE
    )

    def repl(m: re.Match):
        indent = m.group(1) or ""
        rest = m.group(3) or ""
        if enabled:
            # Activa: #define MACRO...
            return f"{indent}#define {macro}{rest}"
        else:
            # Disable: //#define MACRO...
            return f"{indent}//#define {macro}{rest}"

    if pattern.search(text):
        text = pattern.sub(repl, text)
   

    path.write_text(text)

def str_to_bool(v: str) -> bool:
    return str(v).lower() in ("1", "true", "yes", "on")

def main():
    parser = argparse.ArgumentParser(
        description="Toggle Marlin feature flags from GitHub Actions inputs."
    )
    # Inputs that will come from workflow_dispatch
    parser.add_argument("--x_routine_auto_offset", required=True)
    parser.add_argument("--d_routine_auto_offset", required=True)
    parser.add_argument("--dwin_render_thumbnail", required=True)
    parser.add_argument("--dwin_dimm_menu", required=True)
    parser.add_argument("--dwin_zhome_menu", required=True)
    parser.add_argument("--extra_preheat_labels", required=True)
    parser.add_argument("--dwin_custom_extrude", required=True)
    parser.add_argument("--skew_correction", required=True)
    parser.add_argument("--dwin_skew_menu", required=True)
    parser.add_argument("--input_shaping_x", required=True)
    parser.add_argument("--input_shaping_y", required=True)
    parser.add_argument("--dwin_input_shaping_menu", required=True)
    parser.add_argument("--one_click_print", required=True)

    args = parser.parse_args()

    # Routes
    cfg_base = Path("Marlin/Configuration.h")
    cfg_adv_base = Path("Marlin/Configuration_adv.h")
    cfg = Path("Marlin/Configuration.h")
    cfg_adv = Path("Marlin/Configuration_adv.h")

    # We ALWAYS start from the base templates
    cfg.write_text(cfg_base.read_text())
    cfg_adv.write_text(cfg_adv_base.read_text())

    # Booleans from inputs
    x_routine = str_to_bool(args.x_routine_auto_offset)
    d_routine = str_to_bool(args.d_routine_auto_offset)
    dwin_render = str_to_bool(args.dwin_render_thumbnail)
    dwin_dimm = str_to_bool(args.dwin_dimm_menu)
    dwin_zhome = str_to_bool(args.dwin_zhome_menu)
    extra_preheat = str_to_bool(args.extra_preheat_labels)
    dwin_custom_extrude = str_to_bool(args.dwin_custom_extrude)
    skew = str_to_bool(args.skew_correction)
    dwin_skew_menu = str_to_bool(args.dwin_skew_menu)
    ish_x = str_to_bool(args.input_shaping_x)
    ish_y = str_to_bool(args.input_shaping_y)
    dwin_ish_menu = str_to_bool(args.dwin_input_shaping_menu)
    one_click_print = str_to_bool(args.one_click_print)

    # Dependencies:
    # DWIN_SKEW_MENU only if SKEW_CORRECTION is active
    if not skew:
        dwin_skew_menu = False

    # DWIN_INPUT_SHAPING_MENU only if X and Y are active
    if not (ish_x and ish_y):
        dwin_ish_menu = False

    # ---Configuration.h ---
    set_define(cfg, "X_ROUTINE_AUTO_OFFSET", x_routine)
    set_define(cfg, "D_ROUTINE_AUTO_OFFSET", d_routine)
    set_define(cfg, "DWIN_RENDER_THUMBNAIL", dwin_render)
    set_define(cfg, "DWIN_DIMM_MENU", dwin_dimm)
    set_define(cfg, "DWIN_ZHOME_MENU", dwin_zhome)
    set_define(cfg, "EXTRA_PREHEAT_LABELS", extra_preheat)
    set_define(cfg, "DWIN_CUSTOM_EXTRUDE", dwin_custom_extrude)
    set_define(cfg, "SKEW_CORRECTION", skew)
    set_define(cfg, "DWIN_SKEW_MENU", dwin_skew_menu)

    # ---Configuration_adv.h ---
    set_define(cfg_adv, "INPUT_SHAPING_X", ish_x)
    set_define(cfg_adv, "INPUT_SHAPING_Y", ish_y)
    set_define(cfg_adv, "DWIN_INPUT_SHAPING_MENU", dwin_ish_menu)
    set_define(cfg_adv, "ONE_CLICK_PRINT", one_click_print)

if __name__ == "__main__":
    main()
