/**********************************************************************************************//**
 * @file    ABC.h
 * @brief   Analog Keypad Button Capture using Arduino Interrupts.
 *          Arduino Interrupts on analog button capture requires correct resistor sizes.
 * @authors<pre>
 *  TGit-Tech           12/2019     Original for use on UNO pin A9
 *  TGit-Tech           11/2020     Updated for use with Mega-2560 Pin A15
 *  </pre>
 * @todo
 *  Add support for various Analog Pins and Boards
 *************************************************************************************************/
#ifndef _ABC_H
#define _ABC_H

//#define BUZZER
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
  
/**********************************************************************************************//**
 * @class   AnalogButtonCapture
 * @brief   ABC.h
 *************************************************************************************************/
class AnalogButtonCapture {
  public:
    // Constructors for parameterized Base Class
    AnalogButtonCapture(int _AnalogButtonsPin, unsigned long _ButtonDebounce_ms = 150, unsigned long _HoldRepeat = 2000, uint8_t _BuzzerPin = 0xFF);

    // Public Functions
    void StartCapture();     // Default Debounce is 150 milliseconds
    int Button();

  private:
    unsigned long Last_Button_ms = 0;
    unsigned long Debounce = 150;
    unsigned long Hold_Repeat = 2000;
    int Last_Button_Retrieved = NAVKEYNONE;
    uint8_t BuzzerPin = 0xFF;

};
//_____________________________________________________________________________________________________________________
#endif
