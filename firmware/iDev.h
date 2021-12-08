/******************************************************************************************************************//**
 * @file    iDev.h
 * @brief   **ITEM** Tode Device Drivers
 * @todo
 *  - Fix PinSelect in Devices
 *********************************************************************************************************************/
#ifndef _IDEV_H
#define _IDEV_H

#include "iHdw.h"
#include "lib/EEMap.h"
//#include <NewPing.h>
/******************************************************************************//**
 * @defgroup DT [DT] Device-Type enumeration
 * @{
 *********************************************************************************/
// 0x[7] Device Types
#define DT_RW_ONOFF   0x7E    ///< On/Off Switching Device
#define DT_RO_ONOFF   0x7D    ///< On/Off Monitoring Device
#define DT_RO_PRESS   0x7C    ///< Pressure Device
#define DT_RO_TEMP    0x7B    ///< Temperature Device
#define DT_RO_DIST    0x7A    ///< Distance Sensing Device
#define DT_RW_STSTP3W 0x79    ///< Start Stop 3-Wire
///@}
/******************************************************************************************************************//**
 * @class   OnOff
 * @brief   **DEVICE** Digital On/Off *Value* Control and Local Settings; inherits MenuValue.
 *********************************************************************************************************************/
class OnOff : public Device {
  public:
    OnOff(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);
    
    virtual int     IOValue() override;                                           // Stored on Tode
    virtual void    IOValue(int _Value) override;
      
};
/******************************************************************************************************************//**
 * @class   Dist
 * @brief   **DEVICE** Sonic Distance *Value* Control and Local Settings; inherits MenuValue.
 *********************************************************************************************************************/
class Dist : public Device {
  public:
    Dist(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);

    virtual int     IOValue() override;
    virtual void    IOValue(int _Value) override;
    
  protected:
    MenuValue*      TrigPin = 0;
    MenuValue*      EchoPin = 0;

  private:
    //NewPing*        Sonar = 0;
};
/******************************************************************************************************************//**
 * @class   STSTP3W
 * @brief   **DEVICE** Three Wire Start Stop Momentary Device
 *********************************************************************************************************************/
class STSTP3W : public Device {
  public:
    STSTP3W(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);

    virtual int     IOValue() override;
    virtual void    IOValue(int _Value) override;
    virtual byte    Loop() override;
    
  protected:
    MenuValue*      StartPin = 0;
    MenuValue*      StopPin = 0;
    MenuValue*      StatusPin = 0;

  private:
    unsigned long   LastStartMS = 0;
    unsigned long   LastStopMS = 0;
};
//_____________________________________________________________________________________________________________________
#endif
