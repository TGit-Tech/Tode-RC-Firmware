/******************************************************************************************************************//**
 * @file E220.h
 * @brief Radio Communication for EBYTE E220 Radio Modules
 * @notes Communication Only works when Radio Settings are as follow.
 *        UartRate: 9600bps       FEC:        Enable      Address:  (Set via Menu)
 *        Parity:   8N1           Fixed Mode: Enable      Channel:  (Set via Menu - MHz)
 *        AirRate:  2.4kbps       WOR timing: 250ms
 *        Power:    (Set Menu)    IO mode:    PushPull
 *        ** Had a hard time figuring out that New Radio installs had to be in 'Fixed Mode'!
 *        ** Use Radio Menu PC Connect to verify/change be sure to turn off DEBUG and set both Serials to 9600.
 *********************************************************************************************************************/
#ifndef _E220_H
#define _E220_H

#include "RFC.h"
#define RFCLASS E220Radio

/*********************************************//**
 * @defgroup E220BYTES Ebyte-E220 Configuration
 * @{
 ************************************************/
#define E220HEAD0     0   ///< Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
#define E220HEAD1     1   ///< StartReg ( 0x00(00H) to 0x07(07H) )
#define E220HEAD2     2   ///< Length ( Number of Parameter-bytes (0-8) )
#define E220ADDH      3   ///< 00H ADDH
#define E220ADDL      4   ///< 01H ADDL
#define E220REG0      5   ///< 02H REG0 UART.765,PARITY.43,AIR.210
#define E220REG1      6   ///< 03H REG1 SUB.76,RSSI.5,NA.432,TX.01
#define E220REG2      7   ///< 04H REG2 FREQ.76543210
#define E220REG3      8   ///< 05H REG3 RSSI.7,FIX.6,NA.5,LBT.4,NA.3,WOR.210
#define E220CRYPTH    9   ///< 06H CRYPT_H Key-byte
#define E220CRYPTL    10  ///< 07H CRYPT_L Key-byte
///@}
// An all default read parameter = 0xC0 0x00 0x08 0x00 0x00 0x62 0x00 0x17 0x03 0x00 0x00
//                                 read strt lgth ADDH ADDL REG0 REG1 REG2 REG3 CPTH CPTL
/*********************************************//**
 * @defgroup E220MODES Ebyte-E220 Modes
 * @{
 ************************************************/
                          //  M0, M1  - E220 - Operating Modes
#define E220_NORMMODE  0   //  0   0   - TXTNORM OPERATION
#define E220_WAKEMODE  1   //  1   0   - WAKE UP
#define E220_PWRSAVE   2   //  0   1   - POWER SAVE ( NO TRANSmiT, AIR-AWAKE )
#define E220_SLEEPMODE 3   //  1   1   - SLEEP (COMMAND MODE)
///@}
/******************************************************************************************************************//**
 * @class   E220Radio
 * @brief   On-Demand Connect RF-Radio Serial to Arduino USB Serial.
 *********************************************************************************************************************/
class E220Radio : public RadioI {
  public:
    E220Radio(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX);

    virtual void          Mode(byte _Mode);                 ///< SET \ref E32MODES[0]
    virtual byte          Mode();                           ///< GET \ref E32MODES[0]

    virtual void          Address(unsigned int _RFAddress); ///< SET Radio RF-Address[0H][1H]
    virtual unsigned int  Address();                        ///< GET Radio RF-Address[0H][1H]

    virtual void          AirSpeed(byte _Speed);            ///< SET Radio AirSpeed[2H.012 old 3.012]
    virtual byte          AirSpeed();                       ///< GET Radio AirSpeed[2H.012 old 3.012]
    virtual void          UARTSpeed(byte _eSpeed);          ///< SET Radio UART Speed[2H.34567 old 3.34567]
    virtual byte          UARTSpeed();                      ///< GET Radio UART Speed[2H.34567 old 3.34567]

    virtual void          Frequency(int _RFFrequency);      ///< SET Radio Frequency Setting[4H.0123456] !!!
    virtual int           Frequency();                      ///< GET Radio Frequency Setting[4H.0123456] !!!
    virtual byte          Channel();                        ///< GET Channel and Return to Normal Mode[4H.0123456] !!!

    virtual void          TxPower(byte _TxPower);           ///< SET Radio Tx Power Setting[3H.01 old 5.01] !!!
    virtual byte          TxPower();                        ///< GET Radio Tx Power Setting[3H.01 old 5.01] !!!

    virtual void          FECWakeIOFixed(byte _BitCombo);   ///< SET Radio Transmission Mode[5H.2]
    virtual byte          FECWakeIOFixed();                 ///< GET Radio Transmission Mode[5H.2]

    virtual bool          PacketAvailable();                ///< Loop() query to collect RxPacket
    virtual void          Send(TxPacket* Tx);               ///< Send a TxPacket
    RxPacket*             Packet = 0;                       ///< The RF-Received Packet data
    unsigned long         PacketTimer = 0;
    
  private:
    void                  GetParam();
    void                  SetParam();
    byte                  E220Param[11] = { 0,0,0,0,0,0,0,0,0,0,0 };
    byte                  PinM0=0, PinM1=0; 
    byte                  PinTX=0, PinRX=0, PinAUX=0;
};
//_____________________________________________________________________________________________________________________
#endif