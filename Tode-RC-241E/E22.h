/******************************************************************************************************************//**
 * @file E32.h
 * @brief Radio Communication for EBYTE E22 Radio Modules
 * @notes Communication Only works when Radio Settings are as follow.
 *        UartRate: 9600bps       FEC:        Enable      Address:  (Set via Menu)
 *        Parity:   8N1           Fixed Mode: Enable      Channel:  (Set via Menu - MHz)
 *        AirRate:  2.4kbps       WOR timing: 250ms
 *        Power:    (Set Menu)    IO mode:    PushPull
 *        ** Had a hard time figuring out that New Radio installs had to be in 'Fixed Mode'!
 *        ** Use Radio Menu PC Connect to verify/change be sure to turn off DEBUG and set both Serials to 9600.
 *********************************************************************************************************************/
#ifndef _E22_H
#define _E22_H

#include "RFC.h"
#define RFCLASS E22Radio

/*********************************************//**
 * @defgroup E22BYTES Ebyte-E22 Configuration
 * @{
 ************************************************/
#define E22HEAD0     0   ///< Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
#define E22HEAD1     1   ///< StartReg ( 0x00(00H) to 0x07(07H) )
#define E22HEAD2     2   ///< Length ( Number of Parameter-bytes (0-8) )
#define E22ADDH      3   ///< 00H ADDH
#define E22ADDL      4   ///< 01H ADDL
#define E22NETID     5   ///< 02H NETID (NOT on E220)
#define E22REG0      6   ///< 03H REG0 UART.765,PARITY.43,AIR.210 (same as E220)
#define E22REG1      7   ///< 04H REG1 SUB.76,RSSI.5,NA.432,TX.10 (same as E220)
#define E22REG2      8   ///< 05H REG2 FREQ.76543210 (same as E220)
#define E22REG3      9   ///< 06H REG3 RSSI.7,FIX.6,REPEATER.5,LBT.4,WOR.3,WOR.210 (Repeater bit)
#define E22CRYPTH    10  ///< 07H CRYPT_H Key-byte
#define E22CRYPTL    11  ///< 08H CRYPT_L Key-byte
                          // 80H-86H Product Information (Won't use this I don't think)
///@}
/*********************************************//**
 * @defgroup E32MODES Ebyte-E32 Modes
 * @{
 ************************************************/
                          //  M1, M0  - E22 - Operating Modes (SPECIFIC)
#define E22_NORMMODE  0   //  0   0   - TXTNORM OPERATION
#define E22_WAKEMODE  1   //  0   1   - WOR Mode (E32 & E220 is WAKE UP)
#define E22_PWRSAVE   2   //  1   1   - DEEP SLEEP ( NO TRANSmiT, AIR-AWAKE )
#define E22_SLEEPMODE 3   //  1   0   - CONFIGURATION (E32 & E220 is SLEEP/COMMAND MODE)
///@}

/******************************************************************************************************************//**
 * @class   E32Radio
 * @brief   On-Demand Connect RF-Radio Serial to Arduino USB Serial.
 *********************************************************************************************************************/
class E22Radio : public RadioI {
  public:
    E22Radio(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX);

    virtual void          Mode(byte _Mode);                 ///< SET \ref E32MODES[0]
    virtual byte          Mode();                           ///< GET \ref E32MODES[0]

    virtual void          Address(unsigned int _RFAddress); ///< SET Radio RF-Address[00H][01H]
    virtual unsigned int  Address();                        ///< GET Radio RF-Address[00H][01H]

    virtual void          NetID(byte _NetID);               ///< SET Radio NETID[02H]
    virtual byte          NetID();                          ///< GET Radio NETID[02H]

    virtual void          UARTSpeed(byte _eSpeed);          ///< SET Radio UART Speed REG0[03H.76543]
    virtual byte          UARTSpeed();                      ///< GET Radio UART Speed REG0[03H.76543]
    virtual void          AirSpeed(byte _Speed);            ///< SET Radio AirSpeed REG0[03H.210]
    virtual byte          AirSpeed();                       ///< GET Radio AirSpeed REG0[03H.210]
    
    virtual void          TxPower(byte _TxPower);           ///< SET Radio Tx Power Setting[04H.10]
    virtual byte          TxPower();                        ///< GET Radio Tx Power Setting[04H.10]
    
    virtual void          Frequency(int _RFFrequency);      ///< SET Radio Frequency Setting[05H.6543210]
    virtual int           Frequency();                      ///< GET Radio Frequency Setting[05H.6543210]
    virtual byte          Channel();                        ///< GET Channel and Return to Normal Mode[05H.6543210]

    virtual void          FECWakeIOFixed(byte _BitCombo);   ///< SET Radio Transmission Mode[06H.6]
    virtual byte          FECWakeIOFixed();                 ///< GET Radio Transmission Mode[06H.6]

    virtual bool          PacketAvailable();                ///< Loop() query to collect RxPacket
    virtual void          Send(TxPacket* Tx);               ///< Send a TxPacket
    RxPacket*             Packet = 0;                       ///< The RF-Received Packet data
    unsigned long         PacketTimer = 0;
    
  private:
    void                  GetParam();
    void                  SetParam();//    0,1,2,3,4,5,6,7,8,9,0,1
    byte                  E22Param[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
    byte                  PinM0=0, PinM1=0; 
    byte                  PinTX=0, PinRX=0, PinAUX=0;
};
//_____________________________________________________________________________________________________________________
#endif
