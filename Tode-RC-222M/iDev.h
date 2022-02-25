/******************************************************************************************************************//**
 * @file    iDev.h
 * @brief   **ITEM** Tode Device Drivers
 * @todo
 *********************************************************************************************************************/
#ifndef _IDEV_H
#define _IDEV_H

#include "iHdw.h"
#include "lib/EEMap.h"
/******************************************************************************************************************//**
 * @class   OnOff
 * @brief   **DEVICE** Digital On/Off *Value* Control and Local Settings; inherits MenuValue.
 *********************************************************************************************************************/
class OnOff : public Device {
  public:
    OnOff(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);
    
    virtual int     IOValue() override;                                           // Stored on Tode
    virtual void    IOValue(int _Value) override;
    virtual byte    Loop() override;
};
/******************************************************************************************************************//**
 * @class   OnOff
 * @brief   **DEVICE** Analog Read for Pressure Sensor and Local Settings; inherits MenuValue.
 *********************************************************************************************************************/
class Press : public Device {
  public:
    Press(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);
    
    virtual int     IOValue() override;                                           // Stored on Tode
    virtual void    IOValue(int _Value) override;
    virtual byte    Loop() override;

  private:
    int   AveValues[10] = {0};
    int   AveValue = 0;
    int   AveIter = 0;
  
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
    virtual byte    Loop() override;
    
  protected:
    PinSelect*      TrigPin = 0;
    PinSelect*      EchoPin = 0;
    DevPinMode*     TrigPinMode = 0;
    DevPinMode*     EchoPinMode = 0;
    
  private:
    int   AveValues[10] = {0};
    int   AveValue = 0;
    int   AveIter = 0;
    int   trigPin = 0;
    int   echoPin = 0;
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
    PinSelect*      StartPin = 0;
    PinSelect*      StopPin = 0;
    PinSelect*      StatusPin = 0;
    DevPinMode*     StartPinMode = 0;
    DevPinMode*     StopPinMode = 0;
    DevPinMode*     StatusPinMode = 0;
    
  private:
    unsigned long   LastStartMS = 0;
    unsigned long   LastStopMS = 0;
};
//_____________________________________________________________________________________________________________________
#endif
