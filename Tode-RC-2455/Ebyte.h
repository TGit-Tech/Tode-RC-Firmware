/******************************************************************************************************************//**
 * @file Ebyte.h
 * @brief Radio Communication for Ebyte Lora Radio Modules
 * @notes Communication Only works when Radio Settings are as follow.
 *        UartRate: 9600bps       FEC:        Enable      Address:  (Set via Menu)
 *        Parity:   8N1           Fixed Mode: Enable      Channel:  (Set via Menu - MHz)
 *        AirRate:  2.4kbps       WOR timing: 250ms
 *        Power:    (Set Menu)    IO mode:    PushPull
 *        ** Had a hard time figuring out that New Radio installs had to be in 'Fixed Mode'!
 *        ** Use Radio Menu PC Connect to verify/change be sure to turn off DEBUG and set both Serials to 9600.
 *********************************************************************************************************************/
#ifndef _EBYTE_H
#define _EBYTE_H

#include "RFC.h"
#define RFCLASS Ebyte
//#define FORCERADIOMODEL EM_E22_400T33D

/*********************************************//**
 * @defgroup EM Ebyte Radio Models Supported
 * @{
 ************************************************/
#define EM_NOTGOT        0
#define EM_UNKNOWN       1
#define EM_E22_400T30D   2
#define EM_E22_400T33D   3
#define EM_E22_900T30D   4
#define EM_E32_433T20D   5
#define EM_E32_433T30D   6
#define EM_E220_400T30D  7

/*********************************************//**
 * @defgroup EMS Ebyte Radio Model Series
 * @{
 ************************************************/
#define EMS_UNKNOWN     0
#define EMS_E220E22     1
#define EMS_E32         2

/*********************************************//**
 * @defgroup ERFMODE Ebyte Lora Radio Modes (M0,M1)
 * @{
 ************************************************/
#define ERFNORMMODE  0
#define ERFWAKEMODE  1
#define ERFPWRSAVE   2
#define ERFSETMODE   3
///@}

/******************************************************************************************************************//**
 * @class   Ebyte Radio Class
 * @brief   Ebyte brand Radio Settings and Send/Receive bytes Handling.
 *********************************************************************************************************************/
class Ebyte : public RadioI {
  public:
    Ebyte(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX);

    virtual byte          RadioID();
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

    virtual void          NetID(byte _NetID);
    virtual byte          NetID();

    virtual void          FixedTransmission(bool _Fixed);
    virtual bool          FixedTransmission();
    virtual void          E32IODrive(bool _Drive);
    virtual bool          E32IODrive();
    virtual void          RSSIOn(bool _On);
    virtual bool          RSSIOn();
    virtual void          LBTOn(bool _On);
    virtual bool          LBTOn();
    virtual void          WUORTime(byte _Time);
    virtual byte          WUORTime();
    virtual void          E32FECOn(bool _On);
    virtual bool          E32FECOn();
    virtual void          E22WOROn(bool _On);
    virtual bool          E22WOROn();

    virtual bool          PacketAvailable();                ///< Loop() query to collect RxPacket
    virtual void          Send(TxPacket* Tx);               ///< Send a TxPacket
    RxPacket*             Packet = 0;                       ///< The RF-Received Packet data
    unsigned long         PacketTimer = 0;
    
  private:
    void                  CheckRadio();
    void                  GetParam(bool _SkipReq=false);
    void                  SetParam();
    byte                  EM=EM_NOTGOT;     // Ebyte Module Full
    byte                  EMS=EMS_UNKNOWN;  // Ebyte Module Series
    byte                  paramADDH=0, paramADDL=0;
    byte                  paramNETID=0;
    byte                  paramREG0SPED=0, paramREG1OPT=0, paramREG2CHAN=0, paramREG3=0;
    byte                  paramCRYPTH=0, paramCRYPTL=0;
    byte                  EParamGotBytes = 0;
    byte                  PinM0=0, PinM1=0; 
    byte                  PinTX=0, PinRX=0, PinAUX=0;
};
//_____________________________________________________________________________________________________________________
#endif
