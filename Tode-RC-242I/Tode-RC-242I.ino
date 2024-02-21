/******************************************************************************************************************//**
 * @file      Tode-RC-242I.ino
 * @brief     Arduino Sketch for the Tode-RC System Implementing Ebyte E32 & E220 Transceivers.
 * @details   Main Sketch
 * @version   242I [YYMD] update in config.h FIRMWARE F("SRFWv-????"))
 * @bug       NA
 * @note      See config.h for Display and Keypad configuration.
 * @warning   Never call 'RF' object in Item Constructors
 * @copyright MIT Public License
 * @todo      Remove backplane BD235M
 * @author    TGIT-TECH   WIP 242I
 *            TGIT-TECH   Radio Auto-Discovery E32 Multi-Version and E220 Support added                       242E
 *            TGIT-TECH   Radio Auto-Discovery implemented                                                    241E
 *            TGIT-TECH   Added E22-400T33D Radio support (PWM pin 55 is showing but config marks 45)         2414
 *            TGIT-TECH   SA,SD,BD keypad                                                                     23CR
 *            TGIT-TECH                                                                                       23CD
 *            TGIT-TECH   RF Val-Status Colors DoLoop(), Default Settings, PinSelect for PinTypes             23CB
 **********************************************************************************************************************/
#include "Sys.h"
#include "serial2mqtt.h"
//#define FACTORYRESETEEPROM

// include Backplane specific Keypad driver
#if BACKPLANE==SA212K
  #include "lib/AnaKeyPadSA.h"      // Pin set in constructor but not redilly changeable due to interrupts
  #pragma message("BACKPLANE==SA212K")
#elif BACKPLANE==BD235M
  #include "lib/DigKeyPadBD.h"      // Pins are by port-bit assignment in file due to interrupts
  #pragma message("BACKPLANE==BD235M")
#elif BACKPLANE==SD23CF
  #include "lib/DigKeyPadSD.h"
  #pragma message("BACKPLANE==SD23CF")
#elif BACKPLANE==BD241S
  #include "lib/DigKeyPadBD241S.h"
  #pragma message("BACKPLANE==BD241S")
#endif

KeyPad* ButtonCapture = 0;
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
#if defined(FACTORYRESETEEPROM)
for(int addr=0;addr<1024;addr++) {EEPROM.put(addr,0xFF);}
#endif
  if ( SerialUse==USBSERUSE_RFPC ) { Serial.begin(9600); } else { Serial.begin(115200); }
  while (!Serial) ;                       // If the Serial Monitor isn't open at the time
  RFSERIAL.begin(9600);                   // Begin Ebytes Radio Communication
  while (!RFSERIAL);
  DBINFOAL(("setup() *** Free Memory *** = "),(frMemory()))

  pinMode(DISPLIGHTPIN, OUTPUT);digitalWrite(DISPLIGHTPIN, DISPLIGHTACTIVE);  // Turn on Display Backlight
  ButtonCapture = new KeyPad(/*Pin*/KEYPADANAPIN, /*Debounce*/300, /*HoldRepeat*/2000, /*BuzzPin*/0xFF);
  DBINFOAL(("setup() new KeyPad *** Free Memory *** = "),(frMemory()))
  ButtonCapture->StartCapture();
  DBINFOAL(("setup() ButtonCapture->StartCapture() *** Free Memory *** = "),(frMemory()))
  
  System = new Sys();
  DBINFOAL(("setup() new Sys() *** Free Memory *** = "),(frMemory()))
  System->Navigate( NAVKEYNONE );       // Required to Start the Display on Power-Up
  DBINFOAL(("setup() System->Navigate( NAVKEYNONE ) *** Free Memory *** = "),(frMemory()))
  System->mqttActive = (SerialUse==USBSERUSE_MQTT);
  if ( SerialUse==USBSERUSE_RFPC ) {
    if ( System->RF!=0 ) {System->RF->Mode(ERFSETMODE);}
    else { DBERRORL(("SerialUse==USBSERUSE_RFPC && System->RF==0")) SerialUse=USBSERUSE_NONE; }
  }
  // Flush the incoming buffer
  // This possibly was causing non-starts on E22-400T33D radios
  while (RFSERIAL.available()) { RFSERIAL.read();}
  //NAVSYSSIMPLRST - Would like to try a reset but would cause multiple reset condition
  //believe combining the RESET pin to the Display might be an issue here.
}

//vvv loop() vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
void loop() {
  int iButton = ButtonCapture->Button();

  if ( SerialUse==USBSERUSE_RFPC ) {
    if ( Serial.available() ) { RFSERIAL.write(Serial.read()); }
    if ( RFSERIAL.available() ) { Serial.write(RFSERIAL.read()); }
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
  //Used to correct a Device on pin #33 config that caused display to go dark after power-up.
  //pinMode(DISPLIGHTPIN, OUTPUT);digitalWrite(DISPLIGHTPIN, DISPLIGHTACTIVE);
}
