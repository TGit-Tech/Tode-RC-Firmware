/******************************************************************************************************************//**
 * @file    iHdw.h
 * @brief   **ITEM** Hardware focused MenuItem(s)
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
#define DT_MINBOUNDARY  0x78
///@}
/******************************************************************************//**
 * @defgroup IOHDW IO-Hardware Options
 * @{
 *********************************************************************************/
#define IOHDW_NOTSET    0xFE
#define IOHDW_SIDEIO    0xFD
///@}
/******************************************************************************//**
 * @defgroup IOHDW IO-Hardware Options
 * @{
 *********************************************************************************/
#define PM_OLOOFF       0
#define PM_OHIOFF       1
#define PM_INHIGH       2
#define PM_INLOW        3
#define PM_NOTSET       4
///@}
// Arduino Hardware Pins Masked by 'Long' bits -- to determine Used/Available Pins (IOHDW signature mask)
//  31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0   [DIO]
//  63  62  61  60  59  58  57  56  55  54  53  52  51  50  49  48  47  46  45  44  43  42  41  40  39  38  37  36  35  34  33  32  [AIO]
//                                                                                              72  71  70  69  68  67  66  65  64  [XIO]  
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
    virtual int           Value() override;                                        ///< GET Item Value
    virtual void          Value(int _Value, byte _Status = STSNORMAL) override;    ///< SET Item Value
};
/******************************************************************************************************************//**
 * @class   MemReset
 * @brief   **ITEM** for Clearing the EEPROM Memory
 *********************************************************************************************************************/
class MemReset : public MenuValue {
  public:
    MemReset(const __FlashStringHelper* _CName);
    virtual int           Value() override;
    virtual void          Value(int _Value, byte _Status = STSNORMAL) override;    ///< SET Item Value
  private:
    void(* resetFunc) (void) = 0;  // declare reset fuction at address 0
};
/******************************************************************************************************************//**
 * @class   HdwSelect
 * @brief   **ITEM** MenuEEValue: Menu Select attached \ref IOHDW (SIDEIO board, BP-BackPlane).
 *********************************************************************************************************************/
class HdwSelect : public MenuEEValue {
  public:
    HdwSelect(const __FlashStringHelper* _CName);    
};
/******************************************************************************************************************//**
 * @class   SetPinValue
 * @brief   **ITEM** MenuEEValue: Menu Pick-A-Pin of \ref IOHDW specific hardware.  
 *********************************************************************************************************************/
class SetPinValue : public SetValue {
  public:
    SetPinValue(byte _XPos, byte _YPos, NumberName* _FirstNumberName, int _CurrValue, int _VMin, int _VMax, byte _VType, HdwSelect* _Hardware);
    virtual byte    NavValueSet(byte _Key) override;              ///< Pin-Select Value-Set

  private: 
    HdwSelect*      Hdw = 0;
    byte            LastHdwValue = IOHDW_NOTSET;
    
    // SIDEIO Available Pins (bit-masked)
    unsigned long   DSideIO_Pins = 0x00000000;    // P31[31] to P0[0] None of D0 to D30 is available
    unsigned long   ASideIO_Pins = 0xAA002002;    // P63[31] to D32[0] need D45, A1, A3, A5, A7, A9
    byte            XSideIO_Pins = 0x0A;          // P72[31] to P64[0] need A11, A13
};
/******************************************************************************************************************//**
 * @class   PinSelect
 * @brief   **ITEM** MenuEEValue: Menu Pick-A-Pin of \ref IOHDW specific hardware.  
 *********************************************************************************************************************/
class PinSelect : public MenuEEValue {
  public:
    PinSelect(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, HdwSelect* _Hardware);
    virtual ~PinSelect();
    
    virtual byte    Navigate(byte _Key) override;                                   ///< Keypad Navigate by \ref KEY
    
  private:
    SetPinValue*    PinValueSet = 0;
    HdwSelect*      Hdw = 0;
    byte            RFID = 30;                    ///< The RFID of the Device for EEPROM Save
};
/******************************************************************************************************************//**
 * @class   DevPinMode
 * @brief   **ITEM** for Setting a Pinmode in Setup
 *********************************************************************************************************************/
class DevPinMode : public MenuEEValue {
  public:
    DevPinMode(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset);
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
 * @brief   **ITEM** for Sampling the Value for an Average
 *********************************************************************************************************************/
class ReadSamples : public MenuEEValue {
  public:
    ReadSamples(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset);
    virtual   ~ReadSamples();

    bool  IsSampling();
    int   SampleAve();
    void  Sample(int _ReadValue);
  protected:
    
  private:
    int   SampleValues[21] = {0};
    int   SamplesToGet = 0;
    int   SamplesGotten = 0;
    int   SampleValue = 0;
    int   SampleIdx = 0;
    int   SampleCnt = 0;                    // Starts at 0 and stops at Samples->Value() for start-up
    
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
/******************************************************************************************************************//**
 * @class   Device
 * @brief   **ITEM** for Remote and/or Local Values
 *********************************************************************************************************************/
class Device : public MenuValue {
  public:
    Device(const __FlashStringHelper* _CName, byte _RFID, byte _ValueType=0, byte _TodeIndex=0);
    Device(int _EENameAddress, bool _NameSettable, byte _RFID, byte _ValueType=0, byte _TodeIndex=0);
    Device(byte _TodeIndex, byte _RFID);
    
    virtual int     Value() override;                                     ///< GET Item Value
    virtual void    Value(int _Value, byte _Status = STSNORMAL) override; ///< SET Item Value
    virtual int     IOValue();
    virtual void    IOValue(int _Value);
    virtual void    EEClear() override;
            bool    IsLocal=false;
            byte    DevType = BNONE;
            
  protected:
    unsigned long   RxValueTime=0;                                    ///< Value Rx Time        
    unsigned int    RFTodeAddress = INONE;
    
    virtual void    ApplySettings();
    virtual bool    DoLoop();    
    unsigned long   LocalRefreshMS = 0;
    bool            ApplyPinSettings(byte _Pin, byte _pinMode);
    bool            SettingsValid=false;
    bool            ChangingSettings=false;
    
    int             LastGetIOValue = 0;                             // Display Update Changing Values
    int             EETodeAddress = INONE;
    int             EEDevAddress = INONE;
    byte            TodeIndex = BNONE;
    byte            EPin=BNONE, EPinMode=BNONE;
    
  private:
    
};
//_____________________________________________________________________________________________________________________
#endif
