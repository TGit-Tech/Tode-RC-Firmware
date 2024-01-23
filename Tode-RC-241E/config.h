/*******************************************************************************************************************//**
 * @file config.h
 * @brief Debug Pre-Compiler Constants
 **********************************************************************************************************************/
#ifndef _CONFIG_H
#define _CONFIG_H

// Backplane Options
#define SA212K 0
#define BD235M 1
#define SD23CF 2
// Radio Options
#define E32_433T30D 0
#define E220_400T30D 1
#define E22_400T33D 2
/*********************************************//**
 * @defgroup CONFIG Directives
 * @brief    
 * @{
 ************************************************/
#define FIRMWARE F("SRFWv-241E")
#define BACKPLANE SD23CF
#define RADIO E32_433T30D
#define DISPLIGHTACTIVE HIGH
///@}


//--------------------------------------------------------------------------------------------------------------------
//........................ BELOW SHOULD NOT CHANGE UNLESS NEW HARDWARE IS CREATED ....................................
// Board Design
// Arduino IO-Hardware Pins Masked by 'Long' bit positions -- to determine Available Pins (1=Available)
//  31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0   [DPINS]
//  63  62  61  60  59  58  57  56  55 [54] 53  52  51  50  49  48 |47  46  45  44  43  42  41  40  39  38  37  36  35  34  33  32  [APINS] A0=54
//                 |               |               |               |               |            72 |71  70 [69] 68 |67  66  65  64  [XPINSAA] A15=69 
//  A9  A8  A7  A6  A5  A4  A3  A2  A1 [A0]                                                     A   A   A   A15 A14 A13 A12 A11 A10  
#if BACKPLANE==SA212K
  #define KEYPADANAPIN A15          //Side-IO backplane Keypads Analog Pin
  #define RFSERIAL Serial1          //Side-IO backplane uses Arduino 18,19 Serial1 pins.
  #define RFPINS 22,20,18,19,21     //Side-IO backplane RF-Pins *M0*/22, /*M1*/20, /*TX*/18, /*RX*/19, /*AUX*/21
  #define DISPPINS 49,47,23         //Side-IO backplane Display Pins  TFT(cs, dc(rs)(cd), rst)
  #define DISPLIGHTPIN 48           //Side-IO backplane Display Backlight Pin
    #define DIG_DPINS 0x00000000  // P31[31] to P0[0]   None of D30 to D0 is available
    #define DIG_APINS 0xAA002002  // P63[31] to D32[0]  Pins A9, A7, A5, A3, D45, D33
    #define DIG_XPINS 0x0A        // P72[31] to P64[0]  Pins A13, A11
    #define ANA_DPINS 0x00000000  // P31[31] to P0[0]   None of D30 to D0 is available
    #define ANA_APINS 0xAA000000  // P63[31] to D32[0]  Pins A9, A7, A5, A3
    #define ANA_XPINS 0x0A        // P72[31] to P64[0]  Pins A13, A11
    #define PWM_DPINS 0x00000000  // P45 is the only PWM Pin available on Backplane 
    #define PWM_APINS 0x00002000  // P45 is the only PWM Pin available on Backplane
    #define PWM_XPINS 0x00        // P45 is the only PWM Pin available on Backplane
#elif BACKPLANE==BD235M
  #define KEYPADANAPIN A15          //Dummy value but not used - Side-IO backplane Keypads Analog Pin
  #define RFSERIAL Serial3          //IO-Access backplane uses Arduino 14,15 Serial3 pins.
  #define RFPINS 8,9,14,15,50       //IO-Access backplane RF-Pins *M0*/8, /*M1*/9, /*ATX*/14, /*ARX*/15, /*AUX*/50
  #define DISPPINS 7,6,5            //IO-Access backplane Display Pins TFT(cs, dc(rs)(cd), rst)
  #define DISPLIGHTPIN 4            //IO-Access backplane Display Backlight Pin
    #define DIG_DPINS 0xFFFC0000  // D31[31] to D00[0]  D31-D18 Available, D17-16 covered, D15 and below is used by display, radio & keypad
    #define DIG_APINS 0xFFC0FFFF  // P63[31] to D32[0]  A9-A0, SKIP 53-48, D47-D32
    #define DIG_XPINS 0x3F        // P71[07] to P64[0]  Pins A15-A10
    #define ANA_DPINS 0x00000000  // D31[31] to D00[0]  None of D31 to D0 is analog
    #define ANA_APINS 0xFFC00000  // P63[31] to D32[0]  Pins A9-A0
    #define ANA_XPINS 0x3F        // P71[07] to P64[0]  Pins A15-A10
    #define PWM_DPINS 0x00000000  // D31[31] to D00[0]  None OC pins D31-D18
    #define PWM_APINS 0x00007000  // P63[31] to D32[0]  Pins D46(OC5A),D45(OCR5B),D44(OC5C)
    #define PWM_XPINS 0x00        // P71[07] to P64[0]  None OC on any A-Pins
#elif BACKPLANE==SD23CF
  #define KEYPADANAPIN A15          //Side-IO backplane Keypads Analog Pin
  #define RFSERIAL Serial1          //Side-IO backplane uses Arduino 18,19 Serial1 pins.
  #define RFPINS 26,22,18,19,24     //Side-IO backplane RF-Pins *M0*/26, /*M1*/22, /*TX*/18, /*RX*/19, /*AUX*/24
  #define DISPPINS 29,28,-1         //Side-IO backplane Display Pins  TFT(cs, dc(rs)(cd), rst) -1=Not Used
  #define DISPLIGHTPIN 33           //Side-IO backplane Display Backlight Pin (definetly is 33)
    #define DIG_DPINS 0x00000000  // P31[31] to P0[0]   None of D30 to D0 is available
    #define DIG_APINS 0xAA800000  // P63[31] to D32[0]  Pins A9, A7, A5, A3, A1 (5)
    #define DIG_XPINS 0x0A        // P72[31] to P64[0]  Pins A13, A11          (^5+2=7)
    #define ANA_DPINS 0x00000000  // P31[31] to P0[0]   None of D30 to D0 is available
    #define ANA_APINS 0xAA800000  // P63[31] to D32[0]  Pins A9, A7, A5, A3, A1
    #define ANA_XPINS 0x0A        // P72[31] to P64[0]  Pins A13, A11
    #define PWM_DPINS 0x00000000  // P45[45] is the only PWM Pin available on Backplane (^7+1=8 (2)PWR done.) 
    #define PWM_APINS 0x00002000  // P45[45] is the only PWM Pin available on Backplane 
    #define PWM_XPINS 0x00        // P45[45] is the only PWM Pin available on Backplane
#else
  #error BACKPLANE configuration in config.h not found.
#endif
//_____________________________________________________________________________________________________________________
#endif