/************************************************************************//**
   @file  ABC.cpp
   @brief Ref.h
   @authors
      TGit-Tech        9/2018       Original
 ******************************************************************************/
#ifndef _ABC_CPP
#define _ABC_CPP
#include "ABC.h"

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

// NAVKEYRIGHT = 11, 12, 13-14, 15       #18
// NAVKEYUP    = 20, 21, 22-23, 24       #27
// NAVKEYDOWN  =   31, 32-33, 34         #36
// NAVKEYLEFT  =     38, 39-40           #43
// NAVKEYOKAY  = 45, 46, |47| 48, 49     #52
// NAVKEYSET   =   54, 55-56, 57         #60
//#####################################################################################################
AnalogButtonCapture::AnalogButtonCapture(int _AnalogButtonsPin, unsigned long _ButtonDebounce_ms, 
                                         unsigned long _HoldRepeat, uint8_t _BuzzerPin) {
  DBINITL(("AnalogButtonCapture::AnalogButtonCapture"))
  ButtonISR::ButtonsPin = _AnalogButtonsPin;
  ButtonISR::Debounce = _ButtonDebounce_ms;
  Hold_Repeat = _HoldRepeat;
  BuzzerPin = _BuzzerPin;
}
//-----------------------------------------------------------------------------------------------------
void AnalogButtonCapture::StartCapture() {
  DBINFOL(("AnalogButtonCapture::StartCapture()"))
  ButtonISR::EnableInterrupt();
}
//-----------------------------------------------------------------------------------------------------
int AnalogButtonCapture::Button() {
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
//_____________________________________________________________________________________________________________________
#endif
