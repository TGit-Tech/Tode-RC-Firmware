/******************************************************************************************************************//**
 * @file    iHdw.h
 * @brief   **ITEM** Hardware focused MenuItem(s)
 * @details AddTode, SetPinValue, PinSelect, DevPinMode, ReadSamples, SampleBuffer, DLCDevPtrs, Device
 *********************************************************************************************************************/
#ifndef _IHDW_H
#define _IHDW_H

/*
 * SETTINGS
 *  Device Name
 *  OUT PIN (Drive)
 *  IN PIN (Feedback)
 *  LOCAL SERVICE ROUTINES
 *  LOCAL CALCULATORS?
 *  
 */
#include "iLib.h"
#include "lib/EEMap.h"

/******************************************************************************//**
 * @defgroup DT [DT] Device-Type enumeration
 * @{
 *********************************************************************************/
// 0x[7] Device Types
#define DT_RW_ONOFF     0x7E    ///< On/Off Switching Device
#define DT_RO_ONOFF     0x7D    ///< On/Off Monitoring Device
#define DT_RO_DIST      0x7A    ///< Distance Sensing Device
#define DT_RW_STSTP3W   0x79    ///< Start Stop 3-Wire
#define DT_RO_ANAINPUT  0x78    ///< Analog Device (Pressure, Temperature)
#define DT_RW_ANAOUTPUT 0x77
#define DT_DC_SETPOINT  0x76    ///< Dev Logic Control Boundary
#define DT_DC_MATH      0x75

#define DT_MINBOUNDARY  0x75    ///< MINIMUM BOUNDARY CHECK FOR ADD DEVICE KEYS
///@}
/******************************************************************************//**
 * @defgroup PM PinMode Options
 * @{
 *********************************************************************************/
#define PM_OLOOFF       0
#define PM_OHIOFF       1
#define PM_INHIGH       2
#define PM_INLOW        3
#define PM_NOTSET       4
///@}
/******************************************************************************//**
 * @defgroup PT Pin-Type
 * @{
 *********************************************************************************/
#define PT_DIGITAL      0
#define PT_ANALOG       1
#define PT_PWM          2
///@}
/******************************************************************************//**
 * @defgroup LOOP Command Options
 * @{
 *********************************************************************************/
#define LP_NONE         0xFF
#define LP_GETDEVICES   0xFE
///@}
/******************************************************************************//**
 * @defgroup AT AddTode Status
 * @{
 *********************************************************************************/
#define AT_NONE         0
#define AT_GETTING      1
#define AT_SUCCESS      2
#define AT_FAILED       3
///@}

static unsigned long DIO_UsedPins = 0x0;    ///< Bit-Marked assigned pins
static unsigned long AIO_UsedPins = 0x0;    ///< Bit-Marked assigned pins
static byte XIO_UsedPins = 0x0;             ///< Bit-Marked assigned pins

/******************************************************************************************************************//**
 * @class   AddTode
 * @brief   **ITEM** for Adding a Remote Tode by Radio-Address
 *********************************************************************************************************************/
class AddTode : public MenuValue {
  public:
    AddTode();
    virtual int           Value() override;                                       ///< GET Item Value
    virtual void          Value(int _Value, byte _Status = STSNORMAL) override;   ///< SET Item Value
    virtual byte          Loop() override;                                        ///< System loop()
    
  private:
    unsigned long         yStatusClockms = 0;
    bool                  bRanOnceLoopFlag = true;
};
/******************************************************************************************************************//**
 * @class   SetPinValue
 * @brief   **ITEM** MenuEEValue: Menu Pick-A-Pin of \ref IOHDW specific hardware.  
 *********************************************************************************************************************/
class SetPinValue : public SetValue {
  public:
    SetPinValue(byte _XPos, byte _YPos, NumberName* _FirstNumberName, int _CurrValue, int _VMin, int _VMax, byte _VType, byte _PinType=PT_DIGITAL);
    virtual byte    NavValueSet(byte _Key) override;              ///< Pin-Select Value-Set

  private: 
    byte            PinType=PT_DIGITAL;

};
/******************************************************************************************************************//**
 * @class   PinSelect
 * @brief   **ITEM** MenuEEValue: Menu Pick-A-Pin of \ref IOHDW specific hardware.  
 *********************************************************************************************************************/
class PinSelect : public MenuEEValue {
  public:
    PinSelect(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, byte _PinType=PT_DIGITAL);
    virtual ~PinSelect();
    
    virtual byte    Navigate(byte _Key) override;                                   ///< Keypad Navigate by \ref KEY
    
  private:
    SetPinValue*    PinValueSet = 0;
    byte            PinType=PT_DIGITAL;
    byte            RFID = 30;                    ///< The RFID of the Device for EEPROM Save
};
/******************************************************************************************************************//**
 * @class   DevPinMode
 * @brief   **ITEM** for Setting a Pinmode in Setup
 *********************************************************************************************************************/
class DevPinMode : public MenuEEValue {
  public:
    DevPinMode(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, bool _ReadOnly=false );
    virtual   ~DevPinMode();
};
/******************************************************************************************************************//**
 * @class   DevModValue
 * @brief   **ITEM** for Modifying Value stored in EEPROM
 *********************************************************************************************************************/
class DevModValue : public MenuEEValue {
  public:
    DevModValue(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, bool _Denominator=false);
    virtual   ~DevModValue();
  
  protected:
    
  private:
    
};
/******************************************************************************************************************//**
 * @class   ReadSamples
 * @brief   **ITEM** Menu item for selecting the number of samples to take.
 *********************************************************************************************************************/
class ReadSamples : public MenuEEValue {
  public:
    ReadSamples(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset);
    virtual   ~ReadSamples();
};
/******************************************************************************************************************//**
 * @class   SampleBuffer
 * @brief   Sample Buffer for sampling Values for an Average
 *********************************************************************************************************************/
class SampleBuffer {
  public:
    SampleBuffer(byte _SamplesToGet);
    int   SampleAve();
    void  Sample(int _ReadValue);
    byte  SamplesToGet = 0;
  protected:
    
  private:
    int   SampleValues[21] = {0};
    byte  SamplesGotten = 0;
    byte  SampleIdx = 0;
};
class DLCDevPtrs;                             // Forward Declare
/******************************************************************************************************************//**
 * @class   Device
 * @brief   **ITEM** for Remote and/or Local Values
 *********************************************************************************************************************/
class Device : public MenuValue {
  public:
    Device(const __FlashStringHelper* _CName, byte _RFID, byte _ValueType=0, byte _TodeIndex=0);
    Device(int _EENameAddress, bool _NameSettable, byte _RFID, byte _ValueType=0, byte _TodeIndex=0);
    Device(byte _TodeIndex, byte _RFID, byte _ValueType=VTRW);
    
    virtual int     Value() override;                                     ///< GET Item Value
    virtual void    Value(int _Value, byte _Status = STSNORMAL) override; ///< SET Item Value
    virtual void    DispItem(byte _XPos=0, byte _YPos=0, byte _DispMode = DMNORMAL) override;  ///< Display the Item
    virtual int     IOValue();
    virtual void    IOValue(int _Value);
    virtual void    EEClear() override;
            bool    IsLocal=false;
            byte    DevType = BNONE;
    DLCDevPtrs*     DLCDevs = 0;

  protected:
    virtual byte    NavValue(byte _Key) override;                     ///< Navigate the Value, implement ReadOnly
    unsigned long   RxValueTime=0;                                    ///< Value Rx Time        
    unsigned int    RFTodeAddress = INONE;

    virtual int     EESettingAddrBO(byte _ByteOffset);
    virtual void    ApplySettings();                                  
    virtual bool    DoLoop();    
    unsigned long   LocalRefreshMS = 0;

    // Don't Override
    bool    ApplyPinSettings(byte _Pin, byte _pinMode);             ///< Returns true if PinSetting Okay else false.
    bool    ChangingSettings=false;                                 ///< True when in Device Settings, calls ApplySettings() on exit.
    bool    SettingsValid=false;                                    ///< True when Device Settings are Valid.
        
    int             LastGetIOValue = 0;                             // Display Update Changing Values
    int             EETodeAddress = INONE;
    int             EEDevAddress = INONE;
    byte            TodeIndex = BNONE;
    byte            EPin=BNONE, EPinMode=BNONE;

  private:
};
/******************************************************************************************************************//**
 * @class   DLCDevPtrs
 * @brief   Class to Hold Input and Output Device pointers for DLC automation
 *********************************************************************************************************************/
class DLCDevPtrs { 
  public:
    byte InDevIdx   = BNONE; 
    byte In2DevIdx  = BNONE;
    byte OutTodeIdx = BNONE;
    byte OutDevIdx  = BNONE;
    Device*   InDev = 0;
    Device*   In2Dev = 0;
    MenuList* OutTode = 0;
    Device*   OutDev = 0;
};
//_____________________________________________________________________________________________________________________
#endif
