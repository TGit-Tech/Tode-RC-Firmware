/******************************************************************************************************************//**
 * @file    iDCL.h
 * @brief   **ITEM** Device Control Logic (DCL)
 * @todo
 *********************************************************************************************************************/
#ifndef _IDCL_H
#define _IDCL_H

#include "iHdw.h"
#include "lib/EEMap.h"
/******************************************************************************//**
 * @defgroup DCSP [DT] Device Control Set Point enumeration
 * @{
 *********************************************************************************/
#define DCSPO_MIN   0
#define DCSPO_OFF   0    ///< Less Than or Equal To
#define DCSPO_ON    1    ///< Greater Than or Equal To
#define DCSPO_INCR  2    ///< Equal To
#define DCSPO_DECR  3
#define DCSPO_MAX   3
///@}
/******************************************************************************//**
 * @defgroup DCLM [DT] Device Control Logic Mathematics
 * @{
 *********************************************************************************/
#define DCLM_ADD        0x00    ///< Less Than or Equal To
#define DCLM_SUBTRACT   0x01    ///< Greater Than or Equal To
#define DCLM_MULTIPLY   0x02    ///< Equal To
#define DCLM_DIVIDE     0x03    ///< Not Equal To
#define DCLM_AVERAGE    0x04
#define DCLM_COPY       0x05
#define DCLM_ICOPY      0x06
///@}
/******************************************************************************************************************//**
 * @class   Control
 * @brief   **ITEM** for Controol Devices
 *********************************************************************************************************************/
class Control : public Device {
  public:
    Control(byte _TodeIndex, byte _RFID);

    virtual int     IOValue() override;
    virtual void    IOValue(int _Value) override;

  protected:
    virtual bool    DoLoop();
    byte            LoopReturn = BNONE;    
    int             LastSetOutDevValue = IVNOTSET;
    int             EEValAddress = 0xFFFF;            ///< EEPROM Address of Value Location
};
/******************************************************************************************************************//**
 * @class   DCMath
 * @brief   **ITEM** for Input Math like differences across multiple input devices
 *********************************************************************************************************************/
class DCMath : public Control {
  public:
    DCMath(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);  

    virtual int     IOValue() override;
    virtual void    IOValue(int _Value) override;
    virtual byte    Loop() override;
      
  protected:
    virtual void    BuildSubList() override;
    virtual void    ApplySettings() override;

  private:
      HdwSelect*    Hardware = 0;
      byte          Operator=BNONE;
      int           Tolerenc = 0;
      int           LastValue = IVNOTSET;
      unsigned long LastRFSetMS=0;
};
/******************************************************************************************************************//**
 * @class   DCSetPoint
 * @brief   **DEVICE** Device Control(CL) Limit Switch Control
 *********************************************************************************************************************/
class DCSetPoint : public Control {
  public:
    DCSetPoint(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0);
    
    virtual byte    Loop() override;

  protected:
    virtual void    BuildSubList() override;
    virtual void    ApplySettings() override;
    
  private:
    HdwSelect*    Hardware = 0;
    byte          OCntrlTode = 0, OCntrlDev = 0, InputDev = 0;
    int           Tolerance=0;
    byte          LOSetV=BNONE;
    byte          HOSetV=BNONE;
    int           LastInDevValue=IVNOTSET;
    int           LastSetPoint=IVNOTSET;
    unsigned long LastRFSetMS=0;

};
//_____________________________________________________________________________________________________________________
#endif
