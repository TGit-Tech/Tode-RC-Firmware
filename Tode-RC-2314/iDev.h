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

  protected:
    virtual void    BuildSubList() override;
    virtual void    ApplySettings() override;
    
  private:
    HdwSelect*    Hardware = 0;
};
/******************************************************************************************************************//**
 * @class   AnaInput
 * @brief   **DEVICE** Analog Read for Pressure Sensor and Local Settings; inherits Device-MenuEEValue.
 * 
 * PSI = (Value - 97) * 0.2137 same as PreAdd=-97, Divider=4.679
 *********************************************************************************************************************/
class AnaInput : public Device {
  public:
    AnaInput(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);
    
    virtual int     IOValue() override;                                           // Stored on Tode
    virtual void    IOValue(int _Value) override;
    virtual byte    Loop() override;

  protected:
    virtual void    BuildSubList() override;
    virtual void    ApplySettings() override;
    
  private:
    HdwSelect*    Hardware = 0;
    int           IOReadAnaInput();
    int           PreAddValue = 0;      // (2)Bytes
    int           MultNumValue = 0;     // (2)Bytes
    int           MultDenValue = 0;     // (2)Bytes
    int           AddValue = 0;         // (2)Bytes
                                        // (1)Byte Pin, (1)Byte PinMode = 10-Bytes + Samples (OUT OF MEM)
    SampleBuffer* Samples=0;
    byte          ESamplesToGet = 0;
  
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
    virtual void    BuildSubList() override;    
    virtual void    ApplySettings() override;
    
  private:
    HdwSelect*      Hardware = 0;
    int             IOReadDistance();
    byte            trigPin = BNONE, trigPinmode = BNONE;
    byte            echoPin = BNONE, echoPinmode = BNONE;
    
    SampleBuffer*   Samples=0;
    byte            ESamplesToGet = 0;
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
    virtual void    BuildSubList() override;    
    virtual void    ApplySettings() override;
    
  private:
    HdwSelect*      Hardware = 0;
    unsigned long   LastStartMS = 0;
    unsigned long   LastStopMS = 0;
    byte            startPin = BNONE, startPinMode = BNONE;
    byte            stopPin = BNONE, stopPinMode = BNONE;
    byte            statusPin = BNONE, statusPinMode = BNONE;
};
//_____________________________________________________________________________________________________________________
#endif
