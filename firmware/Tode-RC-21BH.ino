/******************************************************************************************************************//**
 * @file      Tode-RC-21BH.ino
 * @brief     Arduino Sketch for the Tode-RC System Implementing Ebyte E32 Transceiver.
 * @details   Main Sketch
 * @version   21BH [YYMD]
 * @bug       None Yet
 * @warning   Never call 'RF' object in Item Constructors
 * @copyright MIT Public License
 * @author    TGIT-TECH   Erase Memory,Fix Per-Digit Hex,SecNet Boundaries                                  21BH
 *            TGIT-TECH   original creation                                                                 2116
 **********************************************************************************************************************/
#include "lib/ABC.h"
#include "lib/ABC.cpp"
#include "Sys.h"

AnalogButtonCapture* ButtonCapture = 0;
Sys* System=0;

void setup() {
  
  pinMode(A15, INPUT_PULLUP);   // Pullup Analog Keypad-Pin
  pinMode(33, OUTPUT);          // RELAY-1
  pinMode(A11, OUTPUT);         // RELAY-2
  /*
  pinMode(A13, INPUT_PULLUP);   // 3-Wire Status ( Grounds on Run )
  digitalWrite(33, HIGH);
  digitalWrite(A11, HIGH);
  */
  pinMode(45, OUTPUT);          // ** For Pump VFD Speed Set **
  pinMode(48, OUTPUT);          
  digitalWrite(48, HIGH);       // Turn on Display Backlight
    
#if DEBUGLEVEL>0
  Serial.begin(115200);           // This has a funny way of blanking the display (Comment out to check)
  while (!Serial) ;             // If the Serial Monitor isn't open at the time
#else
  // Required for 'Radio'->'PC Conn'
  Serial.begin(9600);
  while (!Serial);
#endif

  Serial1.begin(9600);          // Begin Radio Communication

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
