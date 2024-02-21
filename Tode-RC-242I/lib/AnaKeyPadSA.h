/**********************************************************************************************//**
 * @file    AnaKeyPad.h
 * @brief   Analog Keypad Button Capture using Arduino Interrupts.
 *          Arduino Interrupts on analog button capture requires correct resistor sizes.
 * @authors<pre>
 *  TGit-Tech           12/2019     Original for use on UNO pin A9
 *  TGit-Tech           11/2020     Updated for use with Mega-2560 Pin A15
 *	TGit-Tech			11/2023		Display Backlight Setup and PinMode in Constructor
 *  </pre>
 * @todo
 *  Add support for various Analog Pins and Boards
 *************************************************************************************************/
#ifndef _ANAKEYPAD_H
#define _ANAKEYPAD_H

//#define BUZZER
#include "../config.h"
#include "arduino.h"
#include "DB.h"

// YELLOW or ORNG Line above TGT-Star on Display = LOW else HIGH.
//#define DISPLIGHTACTIVE HIGH
//#define DISPLIGHTACTIVE LOW

/******************************************************************************//**
* @defgroup NAVKEY Navigate() Keypad Constants
* @{
*********************************************************************************/
// 0x[F]-Key Presses
#define NAVKEYNONE        0xFF
#define NAVKEYRIGHT       0xFE
#define NAVKEYLEFT        0xFD
#define NAVKEYUP          0xFC
#define NAVKEYDOWN        0xFB
#define NAVKEYSET         0xFA
#define NAVKEYOKAY        0xF9
#define KEYPRESSMIN       0xF0
///@}

//#####################################################################################################
namespace ButtonISR {
  int ButtonsPin = 0;
  unsigned long Debounce = 150;                   // Debounce button milliseconds
  volatile int ButtonBuffer = NAVKEYNONE;            // Store last button pressed for processing
  volatile int Current_Button = NAVKEYNONE;          // Stores last ISR Button
  volatile unsigned long Button_Press_Time = 0;   // Tracks if button is held-down

  //-------------------------------------------------------------------------------------
  // Setup ISR on A0
  //-------------------------------------------------------------------------------------
  void EnableInterrupt() {
    cli();                          // disable interrupt
    if (ButtonsPin == A9) {           // For Arduino UNO pin A9 (correct 19C3)
      PCMSK0 |= (1 << PCINT5);          // Use PCINT5 = PB5 pin 30.
      PCICR |= (1 << PCIE0);            // Enable (PCICR) Pin Change Interrupt Control Register (see page 91 of the Datasheet)
    } else if (ButtonsPin == A15) {   // For Mega pin A15
      //Serial.println("ButtonsPin == A15");
      PCMSK2 |= (1 << PCINT23);         // Use PCINT23 = PK7 pin A15.
      PCICR |= (1 << PCIE2);    
    }
    sei();  //allows interrupts
  }

  //-------------------------------------------------------------------------------------
  // ISR
  // ISR Activated ButtonCheck()
  // The Analog Interrupt will trigger when a button is pressed & when it is let-go
  // Keypad WITH a 4.3K-ohm resistor soldered from pin A0 to GND
  // Store_Button is only set to NONE by a call to retrieve the lcd.Button()
  // TODO; Find a way to programically change PCINT
  //-------------------------------------------------------------------------------------
  ISR(PCINT2_vect) {
    if (ButtonBuffer != NAVKEYNONE || millis() - Button_Press_Time < Debounce) return;    // Debounce Button presses

    // Capture Button Analog Value
    cli();
    //PCICR |= (0<<PCIE0);                                    // Disable (PCICR) Pin Change Interrupt
    Button_Press_Time = millis();                           // Record Press Time for Debounce
    int bpress = analogRead(ButtonsPin); int temp = 0;
    for ( int cnt = 0; cnt < 10; cnt++ ) {                  // Read button 10-Times
      temp = analogRead(ButtonsPin);
      if ( temp < bpress ) bpress = temp;                   // Keep smallest
    }

    // Assign Current Button
    Current_Button = NAVKEYNONE;
	//Serial.print("bpress=");Serial.println(bpress);
    if (bpress < 18) {      Current_Button = NAVKEYRIGHT; }
    else if (bpress < 27) { Current_Button = NAVKEYUP; }
    else if (bpress < 36) { Current_Button = NAVKEYDOWN; }
    else if (bpress < 43) { Current_Button = NAVKEYLEFT; }
    else if (bpress < 49) { Current_Button = NAVKEYOKAY; }
    else if (bpress < 60) { Current_Button = NAVKEYSET; }
    ButtonBuffer = Current_Button;
    //PCICR |= (1<<PCIE0);                                    // Enable (PCICR) Pin Change Interrupt
    sei();
  }
}
  
/**********************************************************************************************//**
 * @class   KeyPad
 * @brief   AnaKeyPad.h
 *************************************************************************************************/
class KeyPad {
  public:
    // Constructors for parameterized Base Class
	KeyPad(int _AnalogButtonsPin, unsigned long _ButtonDebounce_ms = 150, unsigned long _HoldRepeat = 2000, uint8_t _BuzzerPin = 0xFF) {
		DBINITL(("AnaKeyPad::AnaKeyPad"))
  
		pinMode(_AnalogButtonsPin, INPUT_PULLUP);           		// Pullup Analog Keypad-Pin
		
		ButtonISR::ButtonsPin = _AnalogButtonsPin;
		ButtonISR::Debounce = _ButtonDebounce_ms;
		Hold_Repeat = _HoldRepeat;
		BuzzerPin = _BuzzerPin;
		// NAVKEYRIGHT = 11, 12, 13-14, 15       #18
		// NAVKEYUP    = 20, 21, 22-23, 24       #27
		// NAVKEYDOWN  =   31, 32-33, 34         #36
		// NAVKEYLEFT  =     38, 39-40           #43
		// NAVKEYOKAY  = 45, 46, |47| 48, 49     #52
		// NAVKEYSET   =   54, 55-56, 57         #60
		//#####################################################################################################
	}



    // Public Functions
    void StartCapture() {
		DBINFOL(("AnaKeyPad::StartCapture()"))
		ButtonISR::EnableInterrupt();
	}
    int Button(){
		int Rtn = NAVKEYNONE;                                         // Default return NONE
		if ( ButtonISR::ButtonBuffer != NAVKEYNONE ) {                    // Check Button buffer for a button press
			Rtn = ButtonISR::ButtonBuffer;
			#if defined(BUZZER)
			if ( BuzzerPin != 0xFF ) {
				noTone(BuzzerPin);
				tone(BuzzerPin,1200,15);
			}
			#endif
			ButtonISR::ButtonBuffer = NAVKEYNONE;                           // Clear the Buffer
		}
		return Rtn;
	}

  private:
    unsigned long Last_Button_ms = 0;
    unsigned long Debounce = 150;
    unsigned long Hold_Repeat = 2000;
    int Last_Button_Retrieved = NAVKEYNONE;
    uint8_t BuzzerPin = 0xFF;
};
//_____________________________________________________________________________________________________________________
#endif
