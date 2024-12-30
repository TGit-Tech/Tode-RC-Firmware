/**********************************************************************************************//**
 * @file    DigKeyPadSD.h
 * @brief   Digital Keypad Button Capture using Arduino Interrupts for Tode 'SD' backplane.
 * @authors<pre>
 *  TGit-Tech           06/2023     Original
 *	TGit-Tech			11/2023		Display Backlight Setup and PinMode in Constructor
 *	TGit-Tech			12/2023		SD backplane additions to DigKeypad.h file.
 *  </pre>
 * @todo
 *************************************************************************************************/
#ifndef _DIGKEYPAD_H
#define _DIGKEYPAD_H

//#define BUZZER
#include "../config.h"
#include "arduino.h"
#include "DB.h"

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

  void OkayCapture() {
	if (ButtonBuffer != NAVKEYNONE || millis() - Button_Press_Time < Debounce) return;
	cli();
	Button_Press_Time = millis();                           // Record Press Time for Debounce
	Current_Button = NAVKEYOKAY;
	ButtonBuffer = Current_Button;
	sei();
  }
  void RightCapture() {
	if (ButtonBuffer != NAVKEYNONE || millis() - Button_Press_Time < Debounce) return;
	cli();
	Button_Press_Time = millis();                           // Record Press Time for Debounce
	Current_Button = NAVKEYRIGHT;
	ButtonBuffer = Current_Button;
	sei();
  }

/*
See AtMega 2560 datasheet 'External Interrupts'->'Pin Change Interrupt'
There are (3) Pin-Change Registers #0-PCINT[7-0], #1-PCINT[15-8], #2-PCINT[23-16]
Activate Group in PCICR (Pin Change Interrupt Control)
... Where (PCIE-PinChangeInterruptEnable) - Bit[7-3]N/A, Bit2(PCIE#2), Bit1(PCIE#1), Bit0(PCIE#0)
Use Mask Register to pick-out specific PCINT#
PCMSK2[PCINT23...16]
PCMSK1[PCINT15...8]
PCMSK0[PCINT7...0]
----------------------------------------
Digital Keypad							 
----------------------------------------
SET	= (1)ICSP, (PCINT3), MISO, D50, #22		in Bit0(PCIE0)
OK	= D20 (INT1), SDA, #44
RT	= D21 (INT0), SCL, #43
UP	= A12 (PCINT20)							in Bit2(PCIE2)
DW	= A14 (PCINT22)							in Bit2(PCIE2)
LT	= A15 (PCINT23)							in Bit2(PCIE2)
*/
  //-------------------------------------------------------------------------------------
  // Setup ISR
  //-------------------------------------------------------------------------------------
  void EnableInterrupt() {
    cli();                          // disable interrupt
    
	// There are (3) Pin Change Registers
	// Set PCMSK the Pin Change Interrupt Mask Per Interrupt

							//       7, 6, 5, 4, 3, 2, 1, 0 - bit
	PCMSK0=0b00001000;		//PCINT( 7, 6, 5, 4, 3, 2, 1, 0)							
							//       7, 6, 5, 4, 3, 2, 1, 0 - bit
	PCMSK2=0b11010000;		//PCINT(23,22,21,20,19,18,17,16)

	PCICR |= (1 << PCIE0);		// Enable Pin Change Interrupts PCINT[7-0]=PCIE0
	PCICR |= (1 << PCIE2);		// Enable Pin Change Interrupts PCINT[23-16]=PCIE2 
	
	attachInterrupt(digitalPinToInterrupt(20), ButtonISR::OkayCapture, FALLING);
	attachInterrupt(digitalPinToInterrupt(21), ButtonISR::RightCapture, FALLING);
    sei();  //allows interrupts
  }

  ISR(PCINT0_vect) {
	if (ButtonBuffer != NAVKEYNONE || millis() - Button_Press_Time < Debounce) return;
	cli();
    Button_Press_Time = millis();		// Record Press Time for Debounce
    Current_Button = NAVKEYNONE;		// Default is NAVKEYNONE
	if (     (PINB&0b00001000)==0b00000000) { Current_Button = NAVKEYSET; }
	//Serial.print("Current_Button=");Serial.println(Current_Button,HEX);
	//PORT PINB = 1111_0111 SET	(Active LOW on bit3)
	ButtonBuffer = Current_Button;
	sei();
  }
  ISR(PCINT2_vect) {
	if (ButtonBuffer != NAVKEYNONE || millis() - Button_Press_Time < Debounce) return;
	cli();
    Button_Press_Time = millis();		// Record Press Time for Debounce
    Current_Button = NAVKEYNONE;		// Default is NAVKEYNONE
	//PK7=A15|PK6=A14|PK5=A13|PK4=A12||PK3=A11|PK2=A10|PK1=A9|PK0=A8
	if (     (PINK&0b11010000)==0b11000000) { Current_Button = NAVKEYUP; }
	else if ((PINK&0b11010000)==0b10010000) { Current_Button = NAVKEYDOWN; }
	else if ((PINK&0b11010000)==0b01010000) { Current_Button = NAVKEYLEFT; }
	//Serial.print("Current_Button=");Serial.println(Current_Button,HEX);
	ButtonBuffer = Current_Button;
	sei();
  }  
 
}
//#####################################################################################################
/**********************************************************************************************//**
 * @class   KeyPad
 * @brief   DigKeyPad.h
 *************************************************************************************************/
class KeyPad {
  public:
    // Constructors for parameterized Base Class
    KeyPad(int _AnalogButtonsPin, unsigned long _ButtonDebounce_ms = 150, unsigned long _HoldRepeat = 2000, uint8_t _BuzzerPin = 0xFF) {
		DBINITL(("KeyPad::KeyPad DIGITAL"))
		
		pinMode(50, INPUT_PULLUP);	// SET
		pinMode(20, INPUT_PULLUP);	// OK
		pinMode(21, INPUT_PULLUP); 	// RT
		pinMode(66, INPUT_PULLUP);  // UP - A12
		pinMode(68, INPUT_PULLUP);  // DW - A14
		pinMode(69, INPUT_PULLUP);  // LT - A15
		
		ButtonISR::ButtonsPin = _AnalogButtonsPin;
		ButtonISR::Debounce = _ButtonDebounce_ms;
		Hold_Repeat = _HoldRepeat;
		BuzzerPin = _BuzzerPin;
	}

    // Public Functions
    void StartCapture() {
		DBINFOL(("KeyPad::StartCapture() DIGITAL"))
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