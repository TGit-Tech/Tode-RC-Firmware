/******************************************************************************************************************//**
 * @file E32.h
 * @brief Radio Communication for EBYTE E32 Radio Modules
 * @notes Communication Only works when Radio Settings are as follow.
 *        UartRate: 9600bps       FEC:        Enable      Address:  (Set via Menu)
 *        Parity:   8N1           Fixed Mode: Enable      Channel:  (Set via Menu - MHz)
 *        AirRate:  2.4kbps       WOR timing: 250ms
 *        Power:    (Set Menu)    IO mode:    PushPull
 *        ** Had a hard time figuring out that New Radio installs had to be in 'Fixed Mode'!
 *        ** Use Radio Menu PC Connect to verify/change be sure to turn off DEBUG and set both Serials to 9600.
 *********************************************************************************************************************/
#ifndef _E32_H
#define _E32_H

#include "RFC.h"
#define RFCLASS E32Radio

/*********************************************//**
 * @defgroup E32BYTES Ebyte-E32 Configuration
 * @{
 ************************************************/
#define E32HEAD       0
#define E32ADDH       1
#define E32ADDL       2
#define E32SPEED      3
#define E32CHAN       4
#define E32OPTION     5
///@}
/*********************************************//**
 * @defgroup E32MODES Ebyte-E32 Modes
 * @{
 ************************************************/
                          //  M0, M1  - E32 - Operating Modes
#define E32_NORMMODE  0   //  0   0   - TXTNORM OPERATION
#define E32_WAKEMODE  1   //  1   0   - WAKE UP
#define E32_PWRSAVE   2   //  0   1   - POWER SAVE ( NO TRANSmiT, AIR-AWAKE )
#define E32_SLEEPMODE 3   //  1   1   - SLEEP (COMMAND MODE)
///@}

/******************************************************************************************************************//**
 * @class   E32Radio
 * @brief   On-Demand Connect RF-Radio Serial to Arduino USB Serial.
 *********************************************************************************************************************/
class E32Radio : public RadioI {
  public:
    E32Radio(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX);

    virtual void          Mode(byte _Mode);                 ///< SET \ref E32MODES[0]
    virtual byte          Mode();                           ///< GET \ref E32MODES[0]

    virtual void          Address(unsigned int _RFAddress); ///< SET Radio RF-Address[1][2]
    virtual unsigned int  Address();                        ///< GET Radio RF-Address[1][2]

    virtual void          AirSpeed(byte _Speed);            ///< SET Radio AirSpeed[3.012]
    virtual byte          AirSpeed();                       ///< GET Radio AirSpeed[3.012]
    virtual void          UARTSpeed(byte _eSpeed);          ///< SET Radio UART Speed[3.34567]
    virtual byte          UARTSpeed();                      ///< GET Radio UART Speed[3.34567]

    virtual void          Frequency(int _RFFrequency);      ///< SET Radio Frequency Setting[4.01234]
    virtual int           Frequency();                      ///< GET Radio Frequency Setting[4.01234]
    virtual byte          Channel();                        ///< GET Channel and Return to Normal Mode[4.01234]

    virtual void          TxPower(byte _TxPower);           ///< SET Radio Tx Power Setting[5.01]
    virtual byte          TxPower();                        ///< GET Radio Tx Power Setting[5.01]

    virtual void          FECWakeIOFixed(byte _BitCombo);   ///< SET Radio Transmission Mode[5.2]
    virtual byte          FECWakeIOFixed();                 ///< GET Radio Transmission Mode[5.2]

    virtual bool          PacketAvailable();                ///< Loop() query to collect RxPacket
    virtual void          Send(TxPacket* Tx);               ///< Send a TxPacket
    RxPacket*             Packet = 0;                       ///< The RF-Received Packet data
    unsigned long         PacketTimer = 0;
    
  private:
    void                  GetParam();
    void                  SetParam();
    byte                  E32Param[6] = { 0,0,0,0,0,0 };
    byte                  PinM0=0, PinM1=0; 
    byte                  PinTX=0, PinRX=0, PinAUX=0;
};
//_____________________________________________________________________________________________________________________
#endif
