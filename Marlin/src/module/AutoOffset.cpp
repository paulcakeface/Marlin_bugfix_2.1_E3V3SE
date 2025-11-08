#include "AutoOffset.h"

#if ENABLED(USE_AUTOZ_TOOL_2)

/*
 *Function Name: getStr(float f)
 *Purpose: Convert floating point to string (Note: ftoa() or %f is not used here because of a BUG in some Marlin versions that will cause floating point conversion to fail)
 *Params: (float)f The floating point value to be converted
 *Return: (char*) converted string
 *Attention: The number of reload calls should not exceed 16
 */
char *getStr(float f)
{
  static char str[16][16];
  static int index = 0;

  memset(str[index % 16], '\0', 16);
  sprintf(str[index % 16], (f >= 0 ? "+%d.%03d" : "-%d.%03d"), (int)fabs(f), ((int)(fabs(f) * 1000)) % 1000);

  return str[index++ % 16];
}

/*
 *Function Name: ckGpioIsInited(int pin)
 *Purpose: Detect whether the given pin has been initialized to avoid repeated initialization of clk, which may cause timing confusion.
 *Params: (int)pin pin to be detected
 *Return: (bool) true=pin has been initialized; false=pin has not been initialized.
 */
bool HX711::ckGpioIsInited(int pin)
{
  static int pinList[32] = {0};
  FOR_LOOP_TIMES(i, 0, 32, {CHECK_AND_RETURN((pinList[i] == pin), true);CHECK_AND_RUN_AND_RETURN((pinList[i] == 0), {pinList[i] = pin;}, false); });
  return true;
}

/*
 *Function Name: init(int clkPin, int sdoPin)
 *Purpose: HX711 driver initialization (80HZ sampling rate)
 *Params: (int)clkPin The clock signal corresponding to HX711
 *(int)Data signal corresponding to sdoPin HX711
 *Return: None
 */
void HX711::init(int clkPin, int sdoPin)
{
  this->clkPin = clkPin;
  this->sdoPin = sdoPin;
  CHECK_AND_RUN((!HX711::ckGpioIsInited(sdoPin)), GPIO_SET_MODE(sdoPin, 0));
  CHECK_AND_RUN((!HX711::ckGpioIsInited(clkPin)), {GPIO_SET_MODE(clkPin, 1); GPIO_SET_VAL(clkPin, 0); });
}

/*
 *Function Name: getVal(bool isShowMsg)
 *Purpose: Block and read the pressure value, the maximum blocking time is 20ms
 *Params: (bool)isShowMsg whether to print debugging information
 *Return: (int) the pressure value read
 *Attention: To achieve a sampling rate of 80HZ, ensure that the frequency configuration pin of HX711 has been set to the corresponding level.
 */
int HX711::getVal(bool isShowMsg)
{
  static unsigned int lastTickMs = 0;
  int count = 0;
  unsigned int ms = GET_TICK_MS();

  GPIO_SET_VAL(clkPin, 0);

  while (GPIO_GET_VAL(sdoPin) == 1 && (GET_TICK_MS() - ms <= 20)) // The sampling rate is 80 hz (12ms period), and the maximum delay here is 20ms.
    MARLIN_CORE_IDLE();

  DISABLE_ALL_ISR();
  for (int i = 0; i < 24; i++)
  {
    GPIO_SET_VAL(clkPin, 1);
    count = count << 1;
    GPIO_SET_VAL(clkPin, 0);
    CHECK_AND_RUN((GPIO_GET_VAL(sdoPin) == 1), (count++));
  }

  GPIO_SET_VAL(clkPin, 1);
  count |= ((count & 0x00800000) != 0 ? 0xFF000000 : 0); // 24-bit signed, converted to 32-bit signed
  GPIO_SET_VAL(clkPin, 0);
  ENABLE_ALL_ISR();

  // CHECK_AND_RUN(isShowMsg, {PRINTF("T=%08d, S=%08d\n", (int)(GET_TICK_MS() -lastTickMs), (int)count);lastTickMs = GET_TICK_MS(); });
  if (isShowMsg)
  {
    lastTickMs = GET_TICK_MS();
    SERIAL_ECHOLNPGM_P("T=", (int)(GET_TICK_MS() - lastTickMs), ", S=", (int)count);
  }
  return count;
}

/*
*Function Name: hFilter(double *vals, int count, double cutFrqHz, double acqFrqHz)
*Purpose: High-pass filter the data
*Params: (double*)vals data to be filtered
                  (int)count length of data to be filtered
                  (double)cutFrqHz filter cutoff frequency
*(double)acqFrqHz filter sampling frequency (equivalent to the sampling frequency of HX711 80HZ)
*Return: None
*/
void Filters::hFilter(double *vals, int count, double cutFrqHz, double acqFrqHz)
{
  double rc = 1.0f / 2.0f / PI / cutFrqHz;
  double coff = rc / (rc + 1 / acqFrqHz);
  double vi = vals[0], viPrev = vals[0], vo = 0, voPrev = 0;
  FOR_LOOP_TIMES(i, 0, count, {
    vi = vals[i];
    vo = (vi - viPrev + voPrev) * coff;
    voPrev = vo;
    viPrev = vi;
    vals[i] = fabs(vo);
  });
}
/**Bubble sort
 *Ascending order
 */
// static void BubbleSort(double arr[], int len)
// {
//   int i, j;
//   double tem;
//   for (i = len - 1; i > 0; i--)
//   {
//     for (j = 0; j < i; j++)
//     {
//       if (arr[j] > arr[j + 1])
//       {
//         tem = arr[j];
//         arr[j] = arr[j + 1];
//         arr[j + 1] = tem;
//       }
//     }
//   }
// }

/*
 *Function Name: tFilter(double *vals, int count)
 *Purpose: perform burr filtering on data
 *Params: (double)cutFrqHz filter cutoff frequency
 *(double)acqFrqHz filter sampling frequency (equivalent to the sampling frequency of HX711 80HZ)
 *Return: None
 */
void Filters::tFilter(double *vals, int count)
{
  // Bubble sort(vals,count);
  FOR_LOOP_TIMES(i, 0, count - 3, {
   double  minVal = (fabs(vals[i]) < fabs(vals[i+1]) ? vals[i] : vals[i+1]);
    vals[i] = fabs(minVal) < fabs(vals[i+2]) ? minVal : vals[i+2]; });
}

/*
*Function Name: lFilter(double *vals, int count, double k1New)
*Purpose: Low-pass filter the data
*Params: (double*)vals data to be filtered
                  (int)count length of data to be filtered
                  (double)k1New first-order filter parameters
*Return: None
*/
void Filters::lFilter(double *vals, int count, double k1New)
{
  FOR_LOOP_TIMES(i, 1, count, vals[i] = vals[i - 1] * (1 - k1New) + vals[i] * k1New);
}

/*
 *Function Name: readBase()
 *Purpose: Get the maximum, minimum, and average pressure values within a given number (BASE_COUNT/2).
 *Params: None
 *Return: (xyz_long_t) x=MIN; y=AVG; z=MAX;
 */
xyz_long_t ProbeAcq::readBase()
{
  static double vals[PI_COUNT / 2] = {0};

  double minVal = +0x00FFFFFF, avgVal = 0, maxVal = -0x00FFFFFF; // min avg max
  FOR_LOOP_TIMES(i, 0, PI_COUNT / 2, { this->hx711.getVal(false); });
  FOR_LOOP_TIMES(i, 0, PI_COUNT / 2, { vals[i] = this->hx711.getVal(false); });

  Filters::tFilter(vals, PI_COUNT / 2);
  Filters::lFilter(vals, PI_COUNT / 2, LFILTER_K1_NEW);

  ARY_MIN(minVal, vals, PI_COUNT / 2);
  ARY_MAX(maxVal, vals, PI_COUNT / 2);
  ARY_AVG(avgVal, vals, PI_COUNT / 2);
#if ENABLED(SHOW_MSG)
  PRINTF("\n***BASE:MIN=%d, AVG=%d, MAX=%d***\n\n", (int)minVal, (int)avgVal, (int)maxVal);
#endif
  xyz_long_t xyz = {(int)minVal, (int)avgVal, (int)maxVal};
  return xyz;
}

/*
 *Function Name: checHx711()
 *Purpose: Check whether the HX711 module is working properly
 *Params: None
 *Return: true=normal; false=abnormal;
 *Attention: Theoretically, the difference between the maximum and maximum pressure of HX711 within a certain period of time should be greater than 100 and less than MIN_HOLD.
 */
bool ProbeAcq::checHx711()
{
  xyz_long_t bv = readBase();
  return (abs(bv.x - bv.z) < 100 || abs(bv.x - bv.z) > MIN_HOLD) ? false : true;
}
// probeTimes(0, basePos_mm, 0.02, -10, 0, MIN_HOLD, MAX_HOLD));
float ProbeAcq::probeTimes(int max_times, xyz_float_t rdy_pos, float step_mm, float min_dis_mm, float max_z_err, int min_hold, int max_hold)
{

  // SERIAL_ECHOLNPGM("***Starting probeTimes***");
  // SERIAL_ECHOLNPGM(" min_dis_mm: ", min_dis_mm, ", max_z_err: ", max_z_err, ", min_hold: ", min_hold, ", max_hold: ", max_hold);
  ProbeAcq pa;
  float mm0 = 0, mm1 = 0;
  pa.hx711.init(HX711_SCK_PIN, HX711_SDO_PIN);
  pa.minZ_mm = min_dis_mm;
  pa.basePos_mm = rdy_pos;
  pa.baseSpdXY_mm_s = 100;
  pa.baseSpdZ_mm_s = 5;
  pa.step_mm = step_mm;
  pa.minHold = min_hold;
  pa.maxHold = max_hold;

  // SERIAL_ECHOLNPGM("***Starting probe loop***");
  FOR_LOOP_TIMES(i, 0, (max_times <= 0 ? 1 : max_times), {
    mm0 = pa.probePointByStep()->outVal_mm;
    CHECK_AND_RETURN(max_times <= 0, mm0);
    mm1 = pa.probePointByStep()->outVal_mm;
    CHECK_AND_RETURN(fabs(mm0 - mm1) <= max_z_err, (mm0 + mm1) / 2);
  });
  return (mm0 + mm1) / 2;
}

/*
 *Function Name: shakeZAxis(int times)
 *Purpose: Vibrate the Z-axis to eliminate gap stress
 *Params: (int)times the number of vibrations
 */
void ProbeAcq::shakeZAxis(int times)
{
  // Version compatible with 2.1.x using scheduler (not raw STEP)
  const float z0 = current_position[Z_AXIS];
  const float amp = 0.20f; // 0.2 mm up/down per cycle
  FOR_LOOP_TIMES(i, 0, times, {
    DO_BLOCKING_MOVE_TO_Z(z0 + amp, 120);
    DO_BLOCKING_MOVE_TO_Z(z0, 120);
  });
}

/*
 *Function Name: calMinZ()
 *Purpose: Calculate the corresponding z-axis height based on the pressure value in the pressure queue after triggering
 *Params: None
 *Return: None
 */
void ProbeAcq::calMinZ()
{
  double *valP_t = &this->valP[PI_COUNT]; // rock_ does not start with *2, and the array is out of bounds using 20230204
  double *posZ_t = &this->posZ[PI_COUNT]; //

  // 1. Filter rock
  Filters::tFilter(this->valP, PI_COUNT * 2);
  Filters::hFilter(this->valP, PI_COUNT * 2, RC_CUTE_FRQ, 80);
  Filters::lFilter(this->valP, PI_COUNT * 2, LFILTER_K1_NEW);

// 2. Print the results
#if ENABLED(SHOW_MSG)
  PRINTF("%s", "\nx=[");
  FOR_LOOP_TIMES(i, 0, PI_COUNT, PRINTF((i == (PI_COUNT - 1) ? "%s]\n\n" : "%s,"), getStr(posZ_t[i])));
  PRINTF("%s", "y=[");
  FOR_LOOP_TIMES(i, 0, PI_COUNT, PRINTF((i == (PI_COUNT - 1) ? "%s]\n\n" : "%s,"), getStr(valP_t[i])));
#endif
  // 3. Normalize data to facilitate processing
  double valMin = +0x00FFFFFF, valMax = -0x00FFFFFF;
  ARY_MIN(valMin, valP_t, PI_COUNT);
  ARY_MAX(valMax, valP_t, PI_COUNT);
  FOR_LOOP_TIMES(i, 0, PI_COUNT, { valP_t[i] = (valP_t[i] - valMin) / (valMax - valMin); });

  // 4. Calculate and flip the data at a given angle to facilitate calculation of the earliest trigger point
  double angle = atan((valP_t[PI_COUNT - 1] - valP_t[0]) / PI_COUNT);
  double sinAngle = sin(-angle), cosAngle = cos(-angle);
  FOR_LOOP_TIMES(i, 0, PI_COUNT, valP_t[i] = (i - 0) * sinAngle + (valP_t[i] - 0) * cosAngle + 0); // Rotate the angle clockwise along the origin (0,0). The x-axis coordinate value does not need to be considered here.

  // 5. Find the minimum index after flipping
  valMin = +0x00FFFFFF;
  ARY_MIN_INDEX(valMin, this->outIndex, valP_t, PI_COUNT);
  this->outVal_mm = posZ[this->outIndex + PI_COUNT];
#if ENABLED(SHOW_MSG)
  PRINTF("***CalZ Idx=%d, Z=%s***\n", this->outIndex, getStr(this->outVal_mm));
#endif
}

/*
 *Function Name: checkTrigger(int fitVal, int unfitDifVal)
 *Purpose: Trigger status detection, used to detect whether the nozzle is in normal contact with the pressure sensor
 *Params: (int)fitVal RC low-pass filtered pressure value, used for feature detection
 *(int)unfitDifVal Unfiltered but zero-removed pressure value, used for overpressure detection
 *Return: (bool) true=Trigger or abort detected; false=Trigger condition not met.
 */
bool ProbeAcq::checkTrigger()
{
  // SERIAL_ECHOLNPGM("***Starting checkTrigger***");
  static double vp_t[PI_COUNT * 2] = {0};
  static double *valP_t = &vp_t[PI_COUNT]; // Rock 20230204

#if ENABLED(SHOW_MSG)
// static long long lastTickMs = 0, lastUnfitDifVal = 0;
// PRINTF("T=%08d , S=%08d , U=%08d \n", (int)(GET_TICK_MS() -lastTickMs), fitVal, unfitDifVal);
// lastTickMs = GET_TICK_MS();
#endif

  // 0. Make a copy for testing to prevent any impact on the source data.
  FOR_LOOP_TIMES(i, 0, PI_COUNT * 2, vp_t[i] = this->valP[i]);

  // 1. Data filtering rock
  Filters::tFilter(vp_t, PI_COUNT * 2);
  Filters::hFilter(vp_t, PI_COUNT * 2, RC_CUTE_FRQ, 80);
  Filters::lFilter(vp_t, PI_COUNT * 2, LFILTER_K1_NEW);

  // 2. The highest priority. If it reaches the minimum position and has not been triggered, it will be forced to stop.
  CHECK_AND_RETURN((this->posZ[PI_COUNT - 1] <= this->minZ_mm), true);

  // 3. The highest priority, if the pressure is greater than the maximum pressure, it will stop unconditionally.
  CHECK_AND_RETURN((abs(valP_t[PI_COUNT - 1]) > this->maxHold && abs(valP_t[PI_COUNT - 2]) > this->maxHold && abs(valP_t[PI_COUNT - 3]) > this->maxHold), true);

  // 4. The number of detection points does not meet the minimum requirements
  CHECK_AND_RETURN(this->valP[0] == 0, false);

  // 5. Check whether the last 3 points are increasing in order.
  CHECK_AND_RETURN((!((valP_t[PI_COUNT - 1] > valP_t[PI_COUNT - 2]) && (valP_t[PI_COUNT - 2] > valP_t[PI_COUNT - 3]))), false);

  // 6. Check whether the absolute value of the last 3 points is greater than any other point
  FOR_LOOP_TIMES(i, 0, PI_COUNT - 3, CHECK_AND_RETURN((valP_t[PI_COUNT - 1] < valP_t[i] || valP_t[PI_COUNT - 2] < valP_t[i] || valP_t[PI_COUNT - 3] < valP_t[i]), false));

  // 6. Normalize data to facilitate processing
  double valMin = +0x00FFFFFF, valMax = -0x00FFFFFF, valAvg = 0;
  float valP_t_f[PI_COUNT] = {0};
  ARY_MIN(valMin, valP_t, PI_COUNT);
  ARY_MAX(valMax, valP_t, PI_COUNT);
  ARY_AVG(valAvg, valP_t, PI_COUNT);
  FOR_LOOP_TIMES(i, 0, PI_COUNT, valP_t_f[i] = ((double)valP_t[i] - valMin) / (valMax - valMin));

  // 7. Ensure that the slope of all points relative to the last point is greater than 40 degrees to prevent false triggering caused by over sensitivity.
  FOR_LOOP_TIMES(i, 0, PI_COUNT - 1, CHECK_AND_RETURN(((valP_t_f[PI_COUNT - 1] - valP_t_f[i]) / ((32 - i) * 1.0f / 32) < 0.8), false));

  // 8. Limit the minimum value to prevent false triggering.
  CHECK_AND_RETURN(!(valP_t[PI_COUNT - 1] > this->minHold && valP_t[PI_COUNT - 2] > (this->minHold / 2) && valP_t[PI_COUNT - 3] > (this->minHold / 3)), false);
  return true;
}

/*
 *Function Name: probePointByStep()
 *Purpose: Single step method measurement and return measurement results.
 *Params: None
 *Return: (ProbeAcq) this pointer
 */
ProbeAcq *ProbeAcq::probePointByStep()
{
  // SERIAL_ECHOLNPGM("***Starting probePointByStep***");

  // Split Output
  this->outIndex = PI_COUNT - 1;
  this->outVal_mm = 0;

// (Optional) Parameter log
#if ENABLED(SHOW_MSG)
  PRINTF("\nPROBE: rdyX=%s, rdyY=%s, rdyZ=%s, spdXY_mm_s=%s, spdZ_mm_s=%s",
         getStr(this->basePos_mm.x), getStr(this->basePos_mm.y), getStr(this->basePos_mm.z),
         getStr(this->baseSpdXY_mm_s), getStr(this->baseSpdZ_mm_s));
  PRINTF("len_mm=%s, baseCount=%d, minHold=%d, maxHold=%d, step_mm=%s\n\n",
         getStr(this->minZ_mm), PI_COUNT, this->minHold, this->maxHold, getStr(this->step_mm));
#endif

  // 0) Ensure home position in XY and Z base
  DO_BLOCKING_MOVE_TO_XY(this->basePos_mm.x, this->basePos_mm.y, this->baseSpdXY_mm_s);
  DO_BLOCKING_MOVE_TO_Z(this->basePos_mm.z, this->baseSpdZ_mm_s);

  // 1) Small vibration with planner to release slack
  {
    const float z0 = current_position[Z_AXIS];
    const float ampZ = 0.20f;
    DO_BLOCKING_MOVE_TO_Z(z0 + ampZ, 120);
    DO_BLOCKING_MOVE_TO_Z(z0, 120);
  }

  // 2) Fast baseline (simple average) of HX711 to subtract offset
  int unfitAvgVal = 0;
  {
    const uint8_t S = 8;
    long acc = 0;
    FOR_LOOP_TIMES(i, 0, S, { acc += this->hx711.getVal(false); MARLIN_CORE_IDLE(); });
    unfitAvgVal = (int)(acc / (long)S);
  }

// 3) Enable negative Z: disable soft-endstops only during search
#if ENABLED(SOFT_ENDSTOPS)
  extern bool soft_endstops_enabled;
  const bool prev_soft = soft_endstops_enabled;
  soft_endstops_enabled = false; // = M211 S0
#endif

  // 4) Initialize pressure/height queues
  FOR_LOOP_TIMES(i, 0, PI_COUNT * 2, { this->valP[i] = 0; this->posZ[i] = 0; });

  // 5) Step descent with the planner (PORTABLE in 2.1.x)
  const float step_mm = this->step_mm; // p.e. 0.03
  const float z_limit = this->minZ_mm; // P.ej. -10 (Proletvo a Basepos_mm.z)
  float relZ = 0.0f;

  while (relZ > z_limit)
  {
    // take a step down
    DO_BLOCKING_MOVE_TO_Z(current_position[Z_AXIS] - step_mm, max(2.0f, this->baseSpdZ_mm_s));
    relZ = current_position[Z_AXIS] - this->basePos_mm.z;

    // takes reading and feeds queues
    const int nowVal = this->hx711.getVal(false);
    FOR_LOOP_TIMES(i, 0, PI_COUNT * 2 - 1, this->valP[i] = this->valP[i + 1]);
    FOR_LOOP_TIMES(i, 0, PI_COUNT * 2 - 1, this->posZ[i] = this->posZ[i + 1]);
    this->valP[PI_COUNT * 2 - 1] = nowVal - unfitAvgVal;
    this->posZ[PI_COUNT * 2 - 1] = relZ;

    // Contact condition or abort due to limits of your check?
    if (checkTrigger())
    {
      calMinZ();
      break;
    }

    MARLIN_CORE_IDLE();
  }

// 6) Restore soft-endstops
#if ENABLED(SOFT_ENDSTOPS)
  soft_endstops_enabled = prev_soft; // = M211 S1
#endif

  return this;
}

/*
 *Function Name: clearByBrush(xyz_float_t basePos_mm, float norm, float minTemp, float maxTemp)
 *Purpose: Trigger status detection, used to detect whether the nozzle is in normal contact with the pressure sensor
 *Params: (xyz_float_t)basePos_mm The coordinates of the brush during the nozzle wiping process
 *(float)norm During the nozzle wiping process, the distance the nozzle moves forward, backward, left, and right is related to the size of the brush.
 *(float)minTemp Only at this temperature and below can the filament no longer leak out.
 *(float)maxTemp Only at this temperature and above can the filament be guaranteed to have melted
 *Return: (bool) true=wiping success; false=wiping failure.
 *Attention: Since the consumables need to be melted during the nozzle wiping process, the minTemp and maxTemp here need to correspond to the temperature in the GCODE, or to the type of consumables used last time
 */
bool clearByBed(xyz_float_t startPos, xyz_float_t endPos, float minTemp, float maxTemp)
{
  // SERIAL_ECHOLNPGM_P("=== Starting Clear by Bed ===");
  // SERIAL_ECHOLNPGM("startPos xyz: ", startPos.x, ", ", startPos.y, ", ", startPos.z);
  // SERIAL_ECHOLNPGM("endPos xyz: ", endPos.x, ", ", endPos.y, ", ", endPos.z);
  ProbeAcq pa;
  pa.hx711.init(HX711_SCK_PIN, HX711_SDO_PIN);
  DO_BLOCKING_MOVE_TO_Z(startPos.z, 5);
  DO_BLOCKING_MOVE_TO_XY(startPos.x, startPos.y - 10, 50);
  Popup_Window_Height(Nozz_Hot); // Refresh the height page display_nozzle heating
  // SERIAL_ECHOLNPGM_P("Heating nozzle & bed for wiping...");
  SET_HOTEND_TEMP(maxTemp, 0);
  SET_BED_TEMP(60); // Temporarily cancel bed heating

  // SERIAL_ECHOLNPGM_P("Waiting for nozzle to reach target temperature...");
  WAIT_HOTEND_TEMP(60 * 5 * 1000, 5); // Wait for the temperature to reach the set value
  WAIT_BED_TEMP(60 * 5 * 1000, 2);
  Popup_Window_Height(Nozz_Clear); // Refresh the high page display_wipe the nozzle
  In_out_feedtock_level(LEVEL_DISTANCE,FEEDING_DEF_SPEED,false); //Withdraw 50mm
  // SERIAL_ECHOLNPGM_P("Starting nozzle wipe...");
  DO_BLOCKING_MOVE_TO_XY(startPos.x, startPos.y, 50);
  float start_mm = ProbeAcq::probeTimes(3, startPos, 0.03, -10, 0.2, MIN_HOLD, MAX_HOLD / 2);
  // Serial echolnpgm p("clear nozzle start z:",start mm);
  float end_mm = ProbeAcq::probeTimes(3, endPos, 0.03, -10, 0.2, MIN_HOLD, MAX_HOLD / 2);
  // Serial echolnpgm p("clear nozzle end z:",end mm);
  startPos.z = start_mm;
  endPos.z = end_mm;
  // Print log("clear nozzle start z:",start mm,"clear nozzle start z:",end mm);
  // SERIAL_ECHOLNPGM_P("Wiping action...");
  DO_BLOCKING_MOVE_TO_XYZ(startPos.x, startPos.y + 3, startPos.z, 50);
  // SET_HOTEND_TEMP(maxTemp, 0);
  // WAIT_HOTEND_TEMP(60 *5 *1000, 5);//Wait for the temperature to reach the set value
  // RUN_AND_WAIT_GCODE_STR("G1 F500 X%s Y%s z%s", true, getStr(startPos.x), getStr(startPos.y),getStr(startPos.z));
  // DO_BLOCKING_MOVE_TO_XYZ(endPos.x, endPos.y, endPos.z-0.1, 5); //Move forward 3mm to prevent consumables from the last time. adhesion again
  // SERIAL_ECHOLNPGM_P("Wiping forward...");
  DO_BLOCKING_MOVE_TO_XYZ(endPos.x, endPos.y - 3, endPos.z - 0.1, 5);
  endPos.x -= 10;
  endPos.y -= 10;
  // SERIAL_ECHOLNPGM_P("Wiping backward...");
  DO_BLOCKING_MOVE_TO_XYZ(endPos.x, endPos.y, endPos.z - 0.1, 5); // Pull it back 45° and remove the remaining material
  // SERIAL_ECHOLNPGM_P("Wiping complete. Returning to home position...");
  RUN_AND_WAIT_GCODE_CMD("G28 Z", true);

  return true;
}

/*
 *Function Name: probeByPress(xyz_float_t basePos_mm, float*outZ)
 *Purpose: Use pressure sensor to measure nozzle height
 *Params: (xyz_float_t)basePos_mm During the measurement process, the coordinates of the nozzle at the center of the pressure sensor
 *(float*)outZ measurement result, measured twice, averaged.
 *Return: (bool) true=measurement successful; false=measurement failed.
 *Attention: If the nozzle is not wiped clean and there are consumables stuck on it, the measurement will fail.
 */
bool probeByPress(xyz_float_t basePos_mm, float *outZ)
{

  // In order to save time, it was changed from 5 times to 3 times.
  float outZ_mm[2] = {0};
  // CHECK_AND_RETURN((!(pa.checHx711() || pa.checHx711())), false); //Check whether the module is working normally
  FOR_LOOP_TIMES(i, 0, 3, outZ_mm[i] = ProbeAcq::probeTimes(0, basePos_mm, 0.02, -10, 0, MIN_HOLD, MAX_HOLD));
  ARY_SORT(outZ_mm, 3);
#if ENABLED(SHOW_MSG)
  PRINTF("\n***PROBE BY PRESS: z=%s, zs={%s, %s, %s}***\n", getStr(outZ_mm[1]), getStr(outZ_mm[0]), getStr(outZ_mm[1]), getStr(outZ_mm[2]));
#endif
  *outZ = outZ_mm[1];
  PRINTF("\n***PROBE BY PRESS: press_z=%s***\n", getStr(*outZ));
  return true;
}

/*
 *Function Name: probeByTouch(xyz_float_t rdyPos_mm, float*outZ)
 *Purpose: Use CR-TOUCH to measure trigger height
 *Params: (xyz_float_t)basePos_mm During the measurement process, the coordinates of the CR-TOUCH at the center of the pressure sensor
 *(float*)outZ measurement result, measured twice in total, take the second time.
 *Return: (bool) true=measurement successful; false=measurement failed.
 *Attention: The measurement process of CR-TOUCH must be consistent with the measurement process of HOME point.
 */
bool probeByTouch(xyz_float_t rdyPos_mm, float *outZ)
{
  ProbeAcq pa;
  pa.shakeZAxis(20);
  xyz_float_t touchOftPos = CRTOUCH_OFT_POS;
  int oldNozTmp = GET_NOZZLE_TAR_TEMP(0);
  int oldBedTmp = GET_BED_TAR_TEMP();

  DO_BLOCKING_MOVE_TO_Z(rdyPos_mm.z, 5);
  DO_BLOCKING_MOVE_TO_XY(rdyPos_mm.x - touchOftPos.x, rdyPos_mm.y - touchOftPos.y, 100);
  *outZ = PROBE_PPINT_BY_TOUCH(rdyPos_mm.x - touchOftPos.x, rdyPos_mm.y - touchOftPos.y); // Call marlin’s inherent cr touch measurement interface
  PRINTF("\n***PROBE BY TOUCH: touch_z=%s***\n", getStr(*outZ));

  SET_HOTEND_TEMP(oldNozTmp, 0);
  SET_BED_TEMP(oldBedTmp);

  probe.stow(); // Rock
  return true;
}

/*
 *Function Name: printTestResult(float *zTouch, float *zPress)
 *Purpose: Print test results
 *Params: (float*) measurement result of zTouch TOUCH sensor
 *(float*)zPress pressure sensor measurement results
 *Return: None
 *Attention: Only a maximum of 128 sets of test results can be printed.
 */
void printTestResult(float *zTouch, float *zPress)
{
  static float acqVals[128][3] = {0}; // Save up to 128 sets of measurement data
  static int acqValIndex = 0;

  PRINTF("\n***GET Z OFFSET: zTouch={%s, %s, %s}, zPress={%s, %s, %s}, zOffset={%s, %s, %s}***\n",
         getStr(zTouch[0]), getStr(zTouch[1]), getStr(zTouch[2]), getStr(zPress[0]), getStr(zPress[1]), getStr(zPress[2]),
         getStr(zPress[0] - zTouch[0]), getStr(zPress[1] - zTouch[1]), getStr(zPress[2] - zTouch[2]));

  float zt_avg = 0, zp_avg = 0;
  ARY_AVG(zt_avg, zTouch, 3);
  ARY_AVG(zp_avg, zPress, 3);

  acqVals[acqValIndex][0] = zp_avg;
  acqVals[acqValIndex][1] = zt_avg;
  acqVals[acqValIndex][2] = zp_avg - zt_avg;
  acqValIndex = (acqValIndex >= 127 ? 127 : (acqValIndex + 1));
  FOR_LOOP_TIMES(i, 0, acqValIndex, PRINTF("%d\t%s\t%s\t%s\n", i, getStr(acqVals[i][0]), getStr(acqVals[i][1]), getStr(acqVals[i][2])));
}
// High once
float Hight_One(xyz_float_t pressPos)
{
  float temp_value = 0, zoffset_avg = 0;
  float zTouch[1] = {0};
  float zPress[1] = {0};

  bool isRunProByPress = true, isRunProByTouch = true;
  SET_BED_LEVE_ENABLE(false);
  // CHECK_AND_RUN(isRunProByTouch, {FOR_LOOP_TIMES(i, 0, 3, {probeByTouch(rdyPos[i], &zTouch[i]); rdyPos[i].z = zTouch[i];})});
  CHECK_AND_RUN(isRunProByTouch, {FOR_LOOP_TIMES(i, 0, 1, {probeByTouch(pressPos, &zTouch[0]); pressPos.z = zTouch[0]; })});
  // 3. Use a pressure sensor to measure the height of the nozzle

  SET_BED_LEVE_ENABLE(false);
  // CHECK_AND_RUN(isRunProByPress, FOR_LOOP_TIMES(i, 0, 3, {probeByPress(rdyPos[i], &zPress[i]); zPress[i] += NOZ_TEMP_OFT_MM;}));
  CHECK_AND_RUN(isRunProByPress, FOR_LOOP_TIMES(i, 0, 1, {probeByPress(pressPos, &zPress[0]); zPress[0] += NOZ_TEMP_OFT_MM; }));
  // 4. Processing results
  temp_value = (zPress[0] - zTouch[0]);
  printTestResult(zTouch, zPress);
  DO_BLOCKING_MOVE_TO_Z(5, 5);
  return temp_value;
}

// Multiple high-altitude function Multiple. For the time being, implement two high-altitude operations.
float Multiple_Hight(bool isRunProByPress, bool isRunProByTouch)
{
  float zoffset_value[3] = {0};
  uint8_t loop_max = 0, loop_num = 0;
  xyz_float_t pressPos = PRESS_XYZ_POS;
  float temp_value = 0, temp_zoffset = 0, temp_zoffset1 = 0, zoffset_avg = 0;
  for (loop_num = 0; loop_num < ZOFFSET_REPEAT_NIN; loop_num++)
  {
    pressPos.y -= (loop_num * 5);
    zoffset_value[loop_num] = Hight_One(pressPos);
    PRINTF("\n***OUTPUT_ZOFFSET: zOffset=%s***\n", getStr(zoffset_value[loop_num]));
    RUN_AND_WAIT_GCODE_CMD("G28", true); // Get the home point first before measuring
  }
  temp_zoffset = fabs(zoffset_value[0]) - fabs(zoffset_value[1]);
  if (fabs(temp_zoffset) <= ZOFFSET_COMPARE) // The collected values ​​are unified
  {
    ARY_AVG(zoffset_avg, zoffset_value, 2);
    SET_BED_LEVE_ENABLE(true); // Turn on automatic leveling
    In_out_feedtock_level(LEVEL_DISTANCE,FEEDING_DEF_SPEED,true); //feed 50mm
    SET_HOTEND_TEMP(140, 0);
    SET_FAN_SPD(255);                   // Turn on the model cooling fan to ensure faster cooling
    WAIT_HOTEND_TEMP(60 * 5 * 1000, 5); // Wait for the temperature to reach the set value
    Popup_Window_Height(Nozz_Finish);   // Refresh the page when the height is completed
    return zoffset_avg;
  }
  else // If it is not consistent, debug it a few more times.
  {
    for (loop_max = 2; loop_max < ZOFFSET_REPEAT_NAX; loop_max++)
    {
      pressPos.y -= (loop_max - 2) * 5;
      zoffset_value[2] = Hight_One(pressPos);
      temp_zoffset = fabs(zoffset_value[2]) - fabs(zoffset_value[0]);
      temp_zoffset1 = fabs(zoffset_value[2]) - fabs(zoffset_value[1]);
      if ((fabs(temp_zoffset) > ZOFFSET_COMPARE) && (fabs(temp_zoffset1) > ZOFFSET_COMPARE)) // Both are out of range
      {
        continue; // Return to continue collecting
      }
      else if ((fabs(temp_zoffset) <= ZOFFSET_COMPARE) || (fabs(temp_zoffset1) <= ZOFFSET_COMPARE))
      {
        if ((fabs(temp_zoffset) <= ZOFFSET_COMPARE))
        {
          zoffset_value[1] = zoffset_value[2];
        }
        else
        {
          zoffset_value[0] = zoffset_value[2];
        }
        ARY_AVG(zoffset_avg, zoffset_value, 2);
        SET_BED_LEVE_ENABLE(true); // Turn on automatic leveling
        In_out_feedtock_level(LEVEL_DISTANCE,FEEDING_DEF_SPEED,true); //feed 50mm
        SET_HOTEND_TEMP(140, 0);
        SET_FAN_SPD(255);                   // Turn on the model cooling fan to ensure faster cooling
        WAIT_HOTEND_TEMP(60 * 5 * 1000, 5); // Wait for the temperature to reach the set value
        Popup_Window_Height(Nozz_Finish);   // Refresh the page when the height is completed
        return zoffset_avg;
      }
    }
    // Prevent the z-axis compensation from being 0.04
    if (ZOFFSET_REPEAT_NAX == loop_max) // To prevent the output from being 0, count it directly as one in the end.
    {
      /*  If there is a pop-up window prompting the customer that leveling failed, cancel this section and keep getting the final value.
      ARY_AVG(zoffset_avg,zoffset_value,2);
      SET_BED_LEV_ENABLE(true); //Turn on automatic leveling
      In_out_feedtock_level(LEVEL_DISTANCE,FEEDING_DEF_SPEED,true); //feed 50mm
      SET_HOTEND_TEMP(140, 0);
      SET_FAN_SPD(255); //Turn on the model cooling fan to ensure faster cooling
      WAIT_HOTEND_TEMP(60 *5 *1000, 5); //Wait for the temperature to reach the set value
      Popup_Window_Height(Nozz_Finish); //Refresh the page when the height is completed
      */
      return zoffset_avg;
    }
    return zoffset_avg; // Failed against high
  }
  //  CHECK_AND_RUN((isRunProByPress && isRunProByTouch), SET_Z_OFFSET((zPress[0] -zTouch[0]), false));
}
/*
 *Function Name: getZOffset(float*outOffset)
 *Purpose: includes three parts: nozzle wiping type, pressure sensor measurement of nozzle height, CR-TOUCH height measurement, and returns to Z-OFFSET
 *Params: (float*)outOffset measurement result
 *Return: (bool) true=measurement successful, outZ can be used to set system compensation; false=measurement failed.
 *Attention: If the measurement fails, do not set the system Z-OFFSET.
 */
bool getZOffset(bool isNozzleClr, bool isRunProByPress, bool isRunProByTouch, float *outOffset)
{
  SERIAL_ECHOLNPGM_P("=== Starting Get Z Offset ===");
  xyz_float_t pressPos = PRESS_XYZ_POS;
  xyz_float_t touchOftPos = CRTOUCH_OFT_POS;
  xyz_float_t rdyPos[3] = {{0, 0, HIGHT_UPRAISE_Z}, {0, 0, HIGHT_UPRAISE_Z}, {0, 0, HIGHT_UPRAISE_Z}};
  rdyPos[0].x = rdyPos[2].x = (pressPos.x < (BED_SIZE_X_MM / 2) ? (touchOftPos.x < 0) : (touchOftPos.x > 0)) ? pressPos.x : touchOftPos.x + 10;
  rdyPos[0].x -= 9;
  rdyPos[0].y = rdyPos[1].y = ((pressPos.y < (BED_SIZE_Y_MM / 2) ? (touchOftPos.y < 0) : (touchOftPos.y > 0)) ? pressPos.y : touchOftPos.y + 10); // 15
  rdyPos[0].y = rdyPos[1].y -= 17;
  rdyPos[1].x = rdyPos[0].x + (pressPos.x < (BED_SIZE_X_MM / 2) ? +1 : -1) * BED_SIZE_X_MM / 5;
  rdyPos[2].y = rdyPos[0].y + (pressPos.y < (BED_SIZE_Y_MM / 2) ? +1 : -1) * BED_SIZE_Y_MM / 5;
  rdyPos[2].x = (rdyPos[0].x + (rdyPos[1].x - rdyPos[0].x) / 2);
  bool SoftEndstopEnable = soft_endstop._enabled;
  bool reenable = planner.leveling_active;
  // 0. Preparation before measurement

  SET_Z_OFFSET(0, false);                  // Clear the z offset to 0 first to prevent the z offset from affecting the measurement results.
  RUN_AND_WAIT_GCODE_CMD("G28", true);     // Get the home point first before measuring
  // RUN_AND_WAIT_GCODE_CMD("M211 S0", true); // Enable negative values

  // 1. Nozzle wiping for PLA consumables
  srand(millis());
  xyz_float_t startPos = {rdyPos[0].x + (rdyPos[1].x - rdyPos[0].x) * 1 / 5 - 10, rdyPos[0].y + (rdyPos[2].y - rdyPos[0].y) * 2 / 5 + random(0, 9) - 4, 6};
  xyz_float_t endPos = {rdyPos[0].x + (rdyPos[1].x - rdyPos[0].x) * 3 / 5, startPos.y, 6};
  startPos.x = CLEAR_NOZZL_START_X;
  startPos.y = CLEAR_NOZZL_START_Y;
  endPos.x = CLEAR_NOZZL_END_X;
  endPos.y = CLEAR_NOZZL_END_Y;
  startPos.z = 0;
  endPos.z = 0;
  CHECK_AND_RUN(isNozzleClr, clearByBed(startPos, endPos, 140, 175));
  Popup_Window_Height(Nozz_Hight); // Refresh the height page display_nozzle height measurement

  *outOffset = Multiple_Hight(isRunProByPress, isRunProByTouch); // Get the data after measuring twice
  SERIAL_ECHOLNPGM_P("=== Z Offset Measurement Completed ===");

  // PRINTF("\n***OUTPUT_ZOFFSET: zOffset=%s***\n", getStr(*outOffset));

  soft_endstop._enabled = SoftEndstopEnable;
  // RUN_AND_WAIT_GCODE_CMD("M211 S1", true); // Disable negative values

  planner.leveling_active = reenable;
  if ((*outOffset > ZOFFSET_VALUE_MAX) || (*outOffset < ZOFFSET_VALUE_MIN))
  {
    SERIAL_ECHOLNPGM_P("=== Z Offset Measurement Failed: Out of Range ===");
    SERIAL_ECHOLNPGM("OUTPUT_ZOFFSET: ", *outOffset);
    return false; // If the high value is too outrageous, just don’t do it.
  }

  SERIAL_ECHOLNPGM("OUTPUT_ZOFFSET: ", *outOffset);
  SET_Z_OFFSET(*outOffset, true);              // Set the measured Z-OFFSET to the system
  return (isRunProByPress && isRunProByTouch); // The data is valid only if both the nozzle and cr touch are measured.
}

#endif