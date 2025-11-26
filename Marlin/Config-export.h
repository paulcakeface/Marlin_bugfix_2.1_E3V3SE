/**
 * Config.h - Marlin Firmware distilled configuration
 * Usage: Place this file in the 'Marlin' folder with the name 'Config.h'.
 *
 * Exported by Marlin build on 2025-11-26 at 18:13:21.
 */

//
// Info
//
#define STRING_CONFIG_H_AUTHOR                   "MarlinFirmware, Creality, Kraplax & Navaismo"

//
// Machine
//
#define MOTHERBOARD                              BOARD_CREALITY_V3_GD303
#define CUSTOM_MACHINE_NAME                      "Ender-3 V3 SE"

//
// EEPROM
//
#define EEPROM_BOOT_SILENT
#define EEPROM_SETTINGS
#define EEPROM_AUTO_INIT

//
// Stepper Drivers
//
#define X_DRIVER_TYPE                            TMC2208
#define Y_DRIVER_TYPE                            TMC2208
#define Z_DRIVER_TYPE                            TMC2208
#define E0_DRIVER_TYPE                           TMC2208
#define X_ENABLE_ON                              LOW
#define Y_ENABLE_ON                              LOW
#define Z_ENABLE_ON                              LOW
#define E_ENABLE_ON                              LOW

//
// Extruder
//
#define EXTRUDERS                                1
#define INVERT_E0_DIR                            false
#define DEFAULT_NOMINAL_FILAMENT_DIA             1.75
#define ENABLE_AUTO_OFF_DISPLAY
#define LIN_ADVANCE
#define ADVANCE_K                                0.0

//
// Geometry
//
#define X_BED_SIZE                               220
#define X_MIN_POS                                -13
#define X_MAX_POS                                X_BED_SIZE
#define Y_BED_SIZE                               220
#define Y_MIN_POS                                -15
#define Y_MAX_POS                                Y_BED_SIZE + 7
#define Z_MIN_POS                                0
#define Z_MAX_POS                                250
#define MIN_SOFTWARE_ENDSTOPS
#define MAX_SOFTWARE_ENDSTOPS
#define MIN_SOFTWARE_ENDSTOP_X
#define MIN_SOFTWARE_ENDSTOP_Y
#define MIN_SOFTWARE_ENDSTOP_Z
#define MAX_SOFTWARE_ENDSTOP_X
#define MAX_SOFTWARE_ENDSTOP_Y
#define MAX_SOFTWARE_ENDSTOP_Z
#define XY_BED_MIN_ZERO                          0

//
// Homing
//
#define X_HOME_DIR                               -1
#define Y_HOME_DIR                               -1
#define Z_HOME_DIR                               -1
#define HOMING_FEEDRATE_MM_M                     { (50*60), (50*60), (12*60) }
#define HOMING_BUMP_DIVISOR                      { 2, 2, 4 }
#define VALIDATE_HOMING_ENDSTOPS
#define HOMING_BUMP_MM                           { 5, 5, 2 }
#define Z_SAFE_HOMING_X_POINT                    X_CENTER
#define Z_AFTER_HOMING                           10
#define Z_SAFE_HOMING_Y_POINT                    Y_CENTER
#define Z_CLEARANCE_FOR_HOMING                   10
#define Z_SAFE_HOMING

//
// Kinematics
//
#define X_MIN_ENDSTOP_HIT_STATE                  LOW
#define Y_MIN_ENDSTOP_HIT_STATE                  LOW
#define Z_MIN_ENDSTOP_HIT_STATE                  HIGH
#define Z_MIN_PROBE_ENDSTOP_HIT_STATE            HIGH

//
// Motion
//
#define DEFAULT_AXIS_STEPS_PER_UNIT              { 80, 80, 400, 424.9 }
#define AXIS_RELATIVE_MODES                      { false, false, false, false }
#define DEFAULT_MAX_FEEDRATE                     { 500, 500, 20, 30 }
#define DEFAULT_MAX_ACCELERATION                 { 5000, 5000, 5000, 5000 }
#define INVERT_X_DIR                             true
#define INVERT_Y_DIR                             false
#define INVERT_Z_DIR                             true
#define STEP_STATE_E                             HIGH
#define STEP_STATE_X                             HIGH
#define STEP_STATE_Y                             HIGH
#define STEP_STATE_Z                             HIGH
#define JUNCTION_DEVIATION_MM                    0.1
#define DEFAULT_ACCELERATION                     3000
#define DEFAULT_TRAVEL_ACCELERATION              3000
#define DEFAULT_RETRACT_ACCELERATION             3000
#define DEFAULT_MINIMUMFEEDRATE                  0.0
#define DEFAULT_MINTRAVELFEEDRATE                0.0
#define MIN_STEPS_PER_SEGMENT                    4
#define DEFAULT_MINSEGMENTTIME                   20000
#define DEFAULT_EJERK                            10.0
#define JD_HANDLE_SMALL_SEGMENTS
#define DEFAULT_STEPPER_TIMEOUT_SEC              120
#define DISABLE_IDLE_X
#define DISABLE_IDLE_Y
#define DISABLE_IDLE_Z
#define DISABLE_IDLE_E
#define SLOWDOWN
#define SLOWDOWN_DIVISOR                         2
#define MAX_FEEDRATE_EDIT_VALUES                 { 1000, 1000, 40, 60 }
#define MAX_ACCEL_EDIT_VALUES                    { 8000, 8000, 8000, 8000 }
#define EDITABLE_STEPS_PER_UNIT
#define LIMITED_MAX_FR_EDITING
#define LIMITED_MAX_ACCEL_EDITING
#define MULTISTEPPING_LIMIT                      16

//
// Motion Control
//
#define SHAPING_ZETA_X                           0.15
#define SHAPING_ZETA_Y                           0.15
#define INPUT_SHAPING_X
#define INPUT_SHAPING_Y
#define SHAPING_FREQ_X                           42.0
#define SHAPING_FREQ_Y                           40.0

//
// Endstops
//
#define ENDSTOPPULLUPS
#define ENDSTOP_INTERRUPTS_FEATURE

//
// Filament Runout Sensors
//
#define FIL_RUNOUT_ENABLED_DEFAULT               true
#define FIL_RUNOUT_PULLUP
#define FILAMENT_RUNOUT_SCRIPT                   "M600"
#define FILAMENT_RUNOUT_SENSOR
#define NUM_RUNOUT_SENSORS                       1
#define FIL_RUNOUT_STATE                         HIGH

//
// Probes
//
#define PROBING_MARGIN                           3
#define XY_PROBE_FEEDRATE                        (160*60)
#define Z_CLEARANCE_BETWEEN_PROBES               3
#define Z_CLEARANCE_DEPLOY_PROBE                 10
#define Z_CLEARANCE_MULTI_PROBE                  3
#define Z_PROBE_FEEDRATE_SLOW                    (Z_PROBE_FEEDRATE_FAST / 2)
#define PROBE_OFFSET_ZMAX                        10
#define Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN
#define NOZZLE_TO_PROBE_OFFSET                   { -24.25, -15, 0 }
#define Z_AFTER_PROBING                          10
#define EXTRA_PROBING                            0
#define USE_PROBE_FOR_Z_HOMING
#define MULTIPLE_PROBING                         2
#define Z_PROBE_FEEDRATE_FAST                    (10*60)
#define Z_PROBE_ERROR_TOLERANCE                  3
#define Z_PROBE_LOW_POINT                        -2
#define PROBE_OFFSET_ZMIN                        -10
#define BLTOUCH

//
// BLTouch
//
#define BLTOUCH_HS_EXTRA_CLEARANCE               7
#define BLTOUCH_HS_MODE                          true
#define BLTOUCH_DELAY                            350

//
// Leveling
//
#define G29_HALT_ON_FAILURE
#define PROBING_MARGIN_FRONT                     PROBING_MARGIN
#define PROBING_MARGIN_BACK                      PROBING_MARGIN+10
#define G29_RETRY_AND_RECOVER
#define G29_MAX_RETRIES                          3
#define G29_FAILURE_COMMANDS                     "M117 Bed leveling failed.\nG0 Z10\nM300 P25 S880\nM300 P50 S0\nM300 P25 S880\nM300 P50 S0\nM300 P25 S880\nM300 P50 S0\nG4 S1"
#define PROBING_MARGIN_RIGHT                     PROBING_MARGIN+10
#define G29_RECOVER_COMMANDS                     "M117 Probe failed. Rewiping.\nG28\nG12 P0 S12 T0"
#define PROBING_MARGIN_LEFT                      PROBING_MARGIN

//
// Temperature
//
#define THERMAL_PROTECTION_HYSTERESIS            4
#define THERMAL_PROTECTION_PERIOD                40
#define TEMP_SENSOR_0                            1
#define TEMP_HYSTERESIS                          3
#define HEATER_0_MINTEMP                         0
#define HEATER_0_MAXTEMP                         300
#define PREHEAT_1_TEMP_HOTEND                    200
#define BED_OVERSHOOT                            10
#define HOTEND_OVERSHOOT                         15
#define PREHEAT_1_FAN_SPEED                      0
#define PREHEAT_1_LABEL                          "PLA"
#define PREHEAT_1_TEMP_BED                       60
#define TEMP_BED_HYSTERESIS                      3
#define TEMP_BED_RESIDENCY_TIME                  10
#define TEMP_BED_WINDOW                          1
#define TEMP_RESIDENCY_TIME                      10
#define TEMP_WINDOW                              1
#define TEMP_SENSOR_BED                          14
#define THERMAL_PROTECTION_BED_HYSTERESIS        2
#define WATCH_BED_TEMP_INCREASE                  2
#define WATCH_BED_TEMP_PERIOD                    180
#define WATCH_TEMP_INCREASE                      2
#define WATCH_TEMP_PERIOD                        40
#define PREHEAT_3_LABEL                          "PETG"
#define PREHEAT_3_TEMP_CHAMBER                   35
#define PREHEAT_4_FAN_SPEED                      0
#define PREHEAT_4_TEMP_CHAMBER                   35
#define PREHEAT_4_LABEL                          "ABS"
#define PREHEAT_4_TEMP_BED                       110
#define PREHEAT_3_FAN_SPEED                      0
#define PREHEAT_4_TEMP_HOTEND                    260
#define PREHEAT_3_TEMP_BED                       90
#define PREHEAT_2_LABEL                          "TPU"
#define PREHEAT_3_TEMP_HOTEND                    240
#define PREHEAT_2_TEMP_BED                       70
#define PREHEAT_2_FAN_SPEED                      0
#define BED_MINTEMP                              0
#define PREHEAT_2_TEMP_HOTEND                    230
#define BED_MAXTEMP                              120
#define THERMAL_PROTECTION_BED_PERIOD            180

//
// Hotend Temp
//
#define PIDTEMP
#define PID_K1                                   0.95
#define PID_MAX                                  255
#define DEFAULT_Kd                               26.68
#define DEFAULT_Ki                               3.93
#define DEFAULT_Kp                               20.49

//
// PID Temp
//
#define PID_FUNCTIONAL_RANGE                     10

//
// Bed Temp
//
#define MAX_BED_POWER                            255
#define DEFAULT_bedKi                            14.29
#define PIDTEMPBED
#define DEFAULT_bedKp                            123.71
#define DEFAULT_bedKd                            713.77

//
// Fans
//
#define E0_AUTO_FAN_PIN                          PC1
#define EXTRUDER_AUTO_FAN_SPEED                  255
#define EXTRUDER_AUTO_FAN_TEMPERATURE            50
#define FAN_MIN_PWM                              50

//
// Advanced Pause
//
#define FILAMENT_CHANGE_SLOW_LOAD_FEEDRATE       6
#define PAUSE_PARK_RETRACT_LENGTH                2
#define FILAMENT_UNLOAD_PURGE_RETRACT            13
#define FILAMENT_CHANGE_FAST_LOAD_LENGTH         0
#define FILAMENT_UNLOAD_PURGE_FEEDRATE           25
#define FILAMENT_CHANGE_UNLOAD_FEEDRATE          10
#define ADVANCED_PAUSE_FEATURE
#define ADVANCED_PAUSE_PURGE_LENGTH              50
#define PAUSE_PARK_NOZZLE_TIMEOUT                60
#define ADVANCED_PAUSE_PURGE_FEEDRATE            3
#define FILAMENT_CHANGE_UNLOAD_LENGTH            100
#define PAUSE_PARK_RETRACT_FEEDRATE              60
#define FILAMENT_CHANGE_FAST_LOAD_ACCEL          25
#define PAUSE_PARK_NO_STEPPER_TIMEOUT
#define PARK_HEAD_ON_PAUSE
#define FILAMENT_CHANGE_ALERT_BEEPS              15
#define FILAMENT_LOAD_UNLOAD_GCODES
#define FILAMENT_CHANGE_SLOW_LOAD_LENGTH         0
#define FILAMENT_UNLOAD_PURGE_LENGTH             8
#define ADVANCED_PAUSE_RESUME_PRIME              0
#define FILAMENT_CHANGE_FAST_LOAD_FEEDRATE       6
#define FILAMENT_CHANGE_UNLOAD_ACCEL             25
#define FILAMENT_UNLOAD_PURGE_DELAY              5000

//
// Calibrate
//
#define DEFAULT_LEVELING_FADE_HEIGHT             10.0
#define SEGMENT_LEVELED_MOVES
#define GRID_MAX_POINTS_Y                        GRID_MAX_POINTS_X
#define ENABLE_LEVELING_FADE_HEIGHT
#define EXTRAPOLATE_BEYOND_GRID
#define G26_XY_FEEDRATE_TRAVEL                   100
#define LEVELING_BED_TEMP                        60
#define MESH_TEST_NOZZLE_SIZE                    0.4
#define MESH_TEST_BED_TEMP                       60
#define XY_DIAG_AC                               282.84
#define XY_DIAG_BD                               282.84
#define G26_RETRACT_MULTIPLIER                   1.0
#define MESH_TEST_HOTEND_TEMP                    205
#define GRID_MAX_POINTS_X                        6
#define G26_MESH_VALIDATION
#define LEVELED_SEGMENT_LENGTH                   1.0
#define G26_XY_FEEDRATE                          20
#define AUTO_BED_LEVELING_BILINEAR
#define SKEW_CORRECTION
#define ABL_BILINEAR_SUBDIVISION
#define SKEW_CORRECTION_GCODE
#define XY_SIDE_AD                               200.69
#define ENABLE_LEVELING_AFTER_G28
#define MESH_TEST_LAYER_HEIGHT                   0.2
#define PREHEAT_BEFORE_LEVELING
#define LEVELING_NOZZLE_TEMP                     170
#define BILINEAR_SUBDIVISIONS                    4

//
// Media
//
#define SDSUPPORT

//
// LCD
//
#define SHOW_BOOTSCREEN
#define ENCODER_100X_STEPS_PER_SEC               130
#define ENCODER_10X_STEPS_PER_SEC                80
#define ENCODER_RATE_MULTIPLIER
#define DWIN_CREALITY_LCD
#define SPEED_EDIT_MAX                           999
#define SET_INTERACTION_TIME
#define AUTOZ_TOOL_Z                             -5
#define SPEED_EDIT_MIN                           10
#define M73_REPORT
#define SHOW_INTERACTION_TIME
#define EDITABLE_DISPLAY_TIMEOUT
#define SOUND_ON_DEFAULT
#define BABYSTEP_MILLIMETER_UNITS
#define BABYSTEP_MULTIPLICATOR_Z                 0.01
#define BETWEEN_Z                                6
#define FINE_MANUAL_MOVE                         0.025
#define AUTOZ_BRUSH_W                            6.0
#define AUTOZ_BRUSH_X                            AUTOZ_TOOL_X + TOOL_BRUSH_X
#define AUTOZ_BRUSH_Y                            AUTOZ_TOOL_Y + TOOL_BRUSH_Y
#define SD_PROCEDURE_DEPTH                       1
#define SHOW_REMAINING_TIME
#define SET_REMAINING_TIME
#define SET_PROGRESS_MANUALLY
#define BABYSTEPPING
#define FLOW_EDIT_MAX                            999
#define BABYSTEP_ZPROBE_OFFSET
#define FLOW_EDIT_MIN                            10
#define SCROLL_LONG_FILENAMES
#define PLR_ENABLED_DEFAULT                      true
#define M73_REPORT_SD_ONLY
#define ENCODER_5X_STEPS_PER_SEC                 30
#define SHOW_ELAPSED_TIME
#define MANUAL_FEEDRATE                          { 50*60, 50*60, 4*60, 2*60 }
#define LONG_FILENAME_WRITE_SUPPORT
#define AUTOTOOL_PRINT
#define POWER_LOSS_RECOVERY
#define BABYSTEP_MULTIPLICATOR_XY                1
#define LONG_FILENAME_HOST_SUPPORT
#define AUTOZ_BRUSH_H                            6.0
#define AUTOZ_BRUSH_Z                            0
#define TOOL_BRUSH_X                             -5
#define TOOL_BRUSH_Y                             50
#define SD_FINISHED_STEPPERRELEASE               true
#define AUTOZ_TOOL_X                             25
#define AUTOZ_TOOL_Y                             28
#define SD_FINISHED_RELEASECOMMAND               "G1 X0 Y215\nM84"
#define BOOTSCREEN_TIMEOUT                       3000
#define SET_PROGRESS_PERCENT
#define DWIN_LCD_BEEP
#define USE_AUTOZ_TOOL_2
#define SHOW_PROGRESS_PERCENT
#define EVENT_GCODE_SD_ABORT                     "G28XY\nG1 X0 Y215\nM84"
#define SDCARD_RATHERRECENTFIRST
#define AUTOTOOL_RESULT
#define POWER_LOSS_MIN_Z_CHANGE                  0.05
#define POWER_LOSS_ZRAISE                        5

//
// Interface
//
#define LCD_FEEDBACK_FREQUENCY_HZ                5000
#define ENCODER_PULSES_PER_STEP                  4
#define ENCODER_STEPS_PER_MENU_ITEM              1
#define LCD_FEEDBACK_FREQUENCY_DURATION_MS       2

//
// Nozzle Park
//
#define NOZZLE_PARK_FEATURE
#define NOZZLE_PARK_Z_RAISE_MIN                  2
#define NOZZLE_PARK_XY_FEEDRATE                  100
#define NOZZLE_PARK_POINT                        { (0), (0), 0 }
#define NOZZLE_PARK_Z_FEEDRATE                   5

//
// G-code
//
#define BLOCK_BUFFER_SIZE                        16
#define FASTER_GCODE_PARSER
#define DEBUG_FLAGS_GCODE

//
// Serial
//
#define SERIAL_PORT                              1
#define BAUDRATE                                 115200
#define BUFSIZE                                  16
#define MAX_CMD_SIZE                             96
#define PROPORTIONAL_FONT_RATIO                  1.0
#define SERIAL_OVERRUN_PROTECTION
#define TX_BUFFER_SIZE                           32
#define CREALITY_LEVEL_COMPENSATION_ALGORITHM    1
#define USER_LEVEL_CHECK                         1
#define WUHAN_CHENGE_PLATFORM                    1
#define USE_BEEPER                               1
#define COMPEN_FACTOR_1                          0.01
#define COMPEN_FACTOR_2                          0.02
#define COMPEN_FACTOR_3                          0.03
#define COMPEN_FACTOR_5                          0.05
#define COMPEN_FACTOR_8                          0.08
#define PLATFORM_OFFSET                          1
#define COMPEN_FACTOR_10                         0.10
#define COMPEN_FACTOR_15                         0.15
#define LEVEL_ALGORITHM_MAX                      0.7
#define COMPACT_GRID_VALUES                      1
#define LEVEL_ALGORITHM_MIN                      -0.7
#define HIGH_SPEED_1                             1
#define USE_SWITCH_POWER_200W                    0
#define K8_EXTRUDER                              0
#define ALGORITHM_INFO_PRINT                     0
#define HIGH_SPEED                               1
#define SHOW_GRID_VALUES                         1
#define EMERGENCY_PARSER

//
// Host
//
#define BUSY_WHILE_HEATING
#define DEFAULT_KEEPALIVE_INTERVAL               2
#define HOST_KEEPALIVE_FEATURE
#define PRINTJOB_TIMER_AUTOSTART
#define HOST_PROMPT_SUPPORT
#define HOST_ACTION_COMMANDS
#define HOST_STATUS_NOTIFICATIONS

//
// Reporting
//
#define AUTO_REPORT_TEMPERATURES
#define EXTENDED_CAPABILITIES_REPORT
#define CAPABILITIES_REPORT

//
// Safety
//
#define USE_WATCHDOG
#define THERMAL_PROTECTION_HOTENDS
#define EXTRUDE_MAXLENGTH                        1000
#define EXTRUDE_MINTEMP                          170
#define PREVENT_COLD_EXTRUSION
#define PREVENT_LENGTHY_EXTRUDE
#define EXTRUDE_MAXLENGTH_e                      999.9
#define THERMAL_PROTECTION_BED

//
// Servos
//
#define SERVO_DELAY                              { 50 }

//
// Stats
//
#define PRINTCOUNTER_SAVE_INTERVAL               60
#define PRINTCOUNTER
#define SERVICE_WARNING_BUZZES                   3

//
// Extras
//
#define ARC_SUPPORT
#define MAX_ARC_SEGMENT_MM                       1.0
#define MIN_ARC_SEGMENT_MM                       0.1
#define MIN_CIRCLE_SEGMENTS                      24
#define N_ARC_CORRECTION                         25

//
// TMC_Smart
//
#define X_CURRENT                                550
#define INTERPOLATE                              false
#define Y_MICROSTEPS                             16
#define EDGE_STEPPING
#define E0_RSENSE                                0.15
#define E5_HYBRID_THRESHOLD                      30
#define E0_SLAVE_ADDRESS                         0x03
#define E0_HYBRID_THRESHOLD                      30
#define W_HYBRID_THRESHOLD                       3
#define X2_HYBRID_THRESHOLD                      100
#define Y_CURRENT                                550
#define X_HOLD_MULTIPLIER                        0.5
#define Z3_HYBRID_THRESHOLD                      3
#define Z_CHAIN_POS                              -1
#define K_HYBRID_THRESHOLD                       3
#define Y_CHAIN_POS                              -1
#define E0_HOLD_MULTIPLIER                       1
#define X_CHAIN_POS                              -1
#define Y_SLAVE_ADDRESS                          0x03
#define Y_RSENSE                                 0.15
#define U_HYBRID_THRESHOLD                       3
#define E3_HYBRID_THRESHOLD                      30
#define Z_MICROSTEPS                             16
#define Z_HYBRID_THRESHOLD                       3
#define X_SLAVE_ADDRESS                          0x03
#define STEALTHCHOP_Z
#define Z_RSENSE                                 0.15
#define E0_CHAIN_POS                             -1
#define I_HYBRID_THRESHOLD                       3
#define Y_CURRENT_HOME                           Y_CURRENT
#define CHOPPER_TIMING_E                         CHOPPER_DEFAULT_24V
#define X_RSENSE                                 0.15
#define Z_HOLD_MULTIPLIER                        0.5
#define STEALTHCHOP_XY
#define E1_HYBRID_THRESHOLD                      30
#define X_HYBRID_THRESHOLD                       100
#define Y2_HYBRID_THRESHOLD                      100
#define E6_HYBRID_THRESHOLD                      30
#define X_CURRENT_HOME                           X_CURRENT
#define Z4_HYBRID_THRESHOLD                      3
#define HYBRID_THRESHOLD
#define STEALTHCHOP_E
#define Z_CURRENT_HOME                           Z_CURRENT
#define X_INTERPOLATE                            false
#define X_MICROSTEPS                             16
#define V_HYBRID_THRESHOLD                       3
#define E4_HYBRID_THRESHOLD                      30
#define E0_CURRENT                               600
#define Z2_HYBRID_THRESHOLD                      3
#define E0_MICROSTEPS                            16
#define J_HYBRID_THRESHOLD                       3
#define Z_SLAVE_ADDRESS                          0x03
#define CHOPPER_TIMING_X                         USER_X_24V
#define CHOPPER_TIMING_Y                         USER_Y_24V
#define CHOPPER_TIMING_Z                         USER_Z_24V
#define HOLD_MULTIPLIER                          0.5
#define Z_CURRENT                                800
#define E2_HYBRID_THRESHOLD                      30
#define Y_HYBRID_THRESHOLD                       100
#define Y_HOLD_MULTIPLIER                        0.5
#define E7_HYBRID_THRESHOLD                      30
