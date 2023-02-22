/******************************************************************************************************************//**
 * @file      Tode-RC-232H.ino
 * @brief     Arduino Sketch for the Tode-RC System Implementing Ebyte E32 Transceiver.
 * @details   Main Sketch
 * @version   232H [YYMD]
 * @bug       NA
 * @warning   Never call 'RF' object in Item Constructors
 * @copyright MIT Public License
 * @author    TGIT-TECH   232H
 *            TGIT-TECH   Moved PCConn to SerialUse and added auto-reset on set                               232G
 *            TGIT-TECH   MQTT /get implemented                                                               232F
 *            TGIT-TECH   USBSerial for MQTT & KBC(Keyboard Control), src/lb and SRFWv-????                   232E
 *            TGIT-TECH   Implement mqtt(ts) and /set subscriptions in homieMQTT                              232D
 *            TGIT-TECH   Implemented homieMQTT, Setup 'system' menu                                          2327
 *            TGIT-TECH   Skipped changes for a marker.                                                       2323
 *            TGIT-TECH   USB-KB control, RadioPCConn 9600 in Set, 3W pinMode start/stop correction.          231V
 *            TGIT-TECH   No Changes                                                                          231H
 *            TGIT-TECH   Setpoint Incr/Decr adjusts by Input-Setpoint difference (Copy,ICopy in Math)        231D
 *            TGIT-TECH   Incr/Decr PWM Control fixed. DCL feature testing. (works pretty good)               231C
 *            TGIT-TECH   Added AnaOutput.  Testing 231B (Corrupted 2319). Delete PWM added                   231B
 *            TGIT-TECH   Fixed Remote DCLSetPoint value screen update                                        231A
 *            TGIT-TECH   Fixed DCL Del Device crash. Set-OK button to VMIN for 'OFF'. Reset Ptr on Set.      2319
 *            TGIT-TECH   Delete of DCL Device Crashes, DCL-Math (works but w/random issues)                  2318
 *            TGIT-TECH   Added DCL Library DCLimits working ( Delete Dev doesn't work! )                     2315
 *            TGIT-TECH   Put VRngDigits Count in SetValue Constructor, Allowed Named Denominator 0=10K       2314RC
 *            TGIT-TECH   Dynamic SetValue, Fixed Neg-Num Entry on AnaIn                                      2313
 *            TGIT-TECH   Set AEB_DEVSETTINGS 12 (was 10) fix AnaIn; this puts RFID25 out of EEPROM memory    2312
 *            TGIT-TECH   Moved all Devices to Dynamic Setup                                                  2311
 *            TGIT-TECH   Ana Setup Dynamic and Crashes Fixed                                                 22CV
 *            TGIT-TECH   Fixed Tode Naming & Crashes & Radio Settings                                        22CU
 *            TGIT-TECH   Class SetMenuName (Device Naming okay; Tode Naming Broken)                          22CS
 *            TGIT-TECH   MenuItem Names and MenuList Title to __FlashStringHelper                            22CR
 *            TGIT-TECH   Changed SetNumberName to const __FlashStringHelper                                  22CQ
 *            TGIT-TECH   Fixed Double Tode-Name adding to Tode Del List on Config Upd, Samples 20 save mem   22CP
 *            TGIT-TECH   Fixed 5-Digit Edit on HEX, Tode Delete works                                        22CM
 *            TGIT-TECH   AnaInput Device Pointers Fixed MultNum/MultDen where MultDen=0=10000, Removed Press 22CL
 *            TGIT-TECH   AnaInput Device Added but has missing pointer causing device crash                  228F
 *            TGIT-TECH   Distance, STSTP3W non-functioning, odd display on HEX edit figures                  223K
 *            TGIT-TECH   Fixed Add/Del leaving Tode Menu unmanueverable to top bar.                          222P
 *            TGIT-TECH   Delete Device out-of-order Remote Config order by RFID corrected                    222M
 *            TGIT-TECH   Functional Del-Device in Device Setup (Okay on Tode Screen)                         222A
 *            TGIT-TECH   Erase Memory,Fix Per-Digit Hex,SecNet Boundaries                                    21BH
 *            TGIT-TECH   original creation                                                                   2116
 **********************************************************************************************************************/
#include "lib/ABC.h"
#include "lib/ABC.cpp"
#include "Sys.h"
#include "serial2mqtt.h"

// YELLOW or ORNG Line above TGT-Star on Display = LOW else HIGH.
#define DISPLIGHTACTIVE HIGH
//#define DISPLIGHTACTIVE LOW

AnalogButtonCapture* ButtonCapture = 0;
Sys* System=0;
bool KB_ControlActive = false;

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
int frMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
byte SerialUse=EEPROM.read(EMC_USBSERUSE);

//vvv setup() vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
void setup() {
  if ( SerialUse==USBSERUSE_RFPC ) { Serial.begin(9600); } else { Serial.begin(115200); }
  while (!Serial) ;                     // If the Serial Monitor isn't open at the time
  Serial1.begin(9600);                  // Begin Ebytes Radio Communication
  pinMode(A15, INPUT_PULLUP);           // Pullup Analog Keypad-Pin          
  pinMode(48, OUTPUT);                  // Set buzzer to an OUTPUT
  digitalWrite(48, DISPLIGHTACTIVE);    // Turn on Display Backlight
  
  DBINFOAL(("setup() *** Free Memory *** = "),(frMemory()))
  ButtonCapture = new AnalogButtonCapture(/*Pin*/A15, /*Debounce*/300, /*HoldRepeat*/2000, /*BuzzPin*/46);
  DBINFOAL(("setup() new AnalogButtonCapture *** Free Memory *** = "),(frMemory()))
  ButtonCapture->StartCapture();
  DBINFOAL(("setup() ButtonCapture->StartCapture() *** Free Memory *** = "),(frMemory()))
  
  System = new Sys();
  DBINFOAL(("setup() new Sys() *** Free Memory *** = "),(frMemory()))
  System->Navigate( NAVKEYNONE );       // Required to Start the Display on Power-Up
  DBINFOAL(("setup() System->Navigate( NAVKEYNONE ) *** Free Memory *** = "),(frMemory()))
  System->mqttActive = (SerialUse==USBSERUSE_MQTT);
  if ( SerialUse==USBSERUSE_RFPC ) {
    if ( System->RF!=0 ) { System->RF->Mode(E32_SLEEPMODE); }
    else { DBERRORL(("SerialUse==USBSERUSE_RFPC && System->RF==0")) SerialUse=USBSERUSE_NONE; }
  }
}

//vvv loop() vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
void loop() {
  int iButton = ButtonCapture->Button();

  if ( SerialUse==USBSERUSE_RFPC ) {
    if ( Serial.available() ) { Serial1.write(Serial.read()); }
    if ( Serial1.available() ) { Serial.write(Serial1.read()); }
  } else if ( SerialUse==USBSERUSE_KBC ) {
    char SerChar = ' ';
    while (Serial.available()) {
      SerChar = Serial.read();
      if (SerChar=='8') { iButton=NAVKEYUP;Serial.println(SerChar); }
      else if (SerChar=='4') { iButton=NAVKEYLEFT;Serial.println(SerChar); }
      else if (SerChar=='2') { iButton=NAVKEYDOWN;Serial.println(SerChar); }
      else if (SerChar=='6') { iButton=NAVKEYRIGHT;Serial.println(SerChar); }
      else if (SerChar=='9') { iButton=NAVKEYSET;Serial.println(SerChar); }
      else if (SerChar=='3') { iButton=NAVKEYOKAY;Serial.println(SerChar); }
    }
  }

  if ( iButton == 0 ) { DBERRORL(("iButton == 0")) }
  else { System->Loop( System->Navigate( iButton )); }
}
