/******************************************************************************************************************//**
 * @file      Tode-RC-222M.ino
 * @brief     Arduino Sketch for the Tode-RC System Implementing Ebyte E32 Transceiver.
 * @details   Main Sketch
 * @version   222M [YYMD]
 * @bug       NA
 * @warning   Never call 'RF' object in Item Constructors
 * @copyright MIT Public License
 * @author    TGIT-TECH   Delete Device out-of-order Remote Config order by RFID corrected                  222M
 *            TGIT-TECH   Functional Del-Device in Device Setup (Okay on Tode Screen)                       222A
 *            TGIT-TECH   Erase Memory,Fix Per-Digit Hex,SecNet Boundaries                                  21BH
 *            TGIT-TECH   original creation                                                                 2116
 **********************************************************************************************************************/
#include "lib/ABC.h"
#include "lib/ABC.cpp"
#include "Sys.h"

#define DISPLIGHTACTIVE HIGH
//#define DISPLIGHTACTIVE LOW

AnalogButtonCapture* ButtonCapture = 0;
Sys* System=0;

void setup() {

  // vvv Initiate Serial Comm. vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  #if DEBUGLEVEL>0              // DEBUGLEVEL set in DB.h
    Serial.begin(115200);       // This has a funny way of blanking the display (Comment out to check)
    while (!Serial) ;           // If the Serial Monitor isn't open at the time
  #else
    Serial.begin(9600);         // Required for 'Radio'->'PC Conn'
    while (!Serial);
  #endif
    Serial1.begin(9600);        // Begin Radio Communication
    
  // vvv Setup Keypad Capture Display vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  pinMode(A15, INPUT_PULLUP);               // Pullup Analog Keypad-Pin          
  pinMode(48, OUTPUT);
  digitalWrite(48, DISPLIGHTACTIVE);       // Turn on Display Backlight
  ButtonCapture = new AnalogButtonCapture(/*Pin*/A15, /*Debounce*/300, /*HoldRepeat*/2000, /*BuzzPin*/46);
  ButtonCapture->StartCapture();

  System = new Sys();
  System->Navigate( NAVKEYNONE );       // Required to Start the Display on Power-Up

}

void loop() {
  int iButton = ButtonCapture->Button();
  if ( iButton == 0 ) { DBERRORL(("iButton == 0")) }
  else { System->Loop( System->Navigate( iButton )); }
}
