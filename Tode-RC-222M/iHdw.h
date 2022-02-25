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
#define DT_RW_ONOFF   0x7E    ///< On/Off Switching Device
#define DT_RO_ONOFF   0x7D    ///< On/Off Monitoring Device
#define DT_RO_PRESS   0x7C    ///< Pressure Device
#define DT_RO_TEMP    0x7B    ///< Temperature Device
#define DT_RO_DIST    0x7A    ///< Distance Sensing Device
#define DT_RW_STSTP3W 0x79    ///< Start Stop 3-Wire
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
    MemReset(const char* _CName);
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
    HdwSelect(const char* _CName);    
};
/******************************************************************************************************************//**
 * @class   PinSelect
 * @brief   **ITEM** MenuEEValue: Menu Pick-A-Pin of \ref IOHDW specific hardware.  
 *********************************************************************************************************************/
class PinSelect : public MenuEEValue {
  public:
    PinSelect(const char* _CName, byte _RFID, byte _EEOffset, HdwSelect* _Hardware);
    virtual ~PinSelect();
    
    virtual byte    NavValueSet(byte _Key) override;              ///< Pin-Select Value-Set
    
  private:
    HdwSelect*      Hdw = 0;
    byte            LastHdwValue = IOHDW_NOTSET;
    byte            RFID = 30;                    ///< The RFID of the Device for EEPROM Save
    int             ConfigurePins();

    // SIDEIO Available Pins (bit-masked)
    unsigned long   DSideIO_Pins = 0x00000000;    // P31[31] to P0[0] None of D0 to D30 is available
    unsigned long   ASideIO_Pins = 0xAA002002;    // P63[31] to D32[0] need D45, A1, A3, A5, A7, A9
    byte            XSideIO_Pins = 0x0A;          // P72[31] to P64[0] need A11, A13

    // Assign Selected Hardware to these
    unsigned long   DIO_Pins = 0;
    unsigned long   AIO_Pins = 0;
    byte            XIO_Pins = 0;
};
/******************************************************************************************************************//**
 * @class   DevPinMode
 * @brief   **ITEM** for Setting the Pinmode at startup
 *********************************************************************************************************************/
class DevPinMode : public MenuEEValue {
  public:
    DevPinMode(const char* _CName, byte _RFID, byte _EEOffset, PinSelect* _Pin);
    virtual   ~DevPinMode();
    
    void            SetPinMode();
  
  protected:
    
  private:
    PinSelect*      ThePin = 0;
    int             LastPinSet = IVNOTSET;
    
};
/******************************************************************************************************************//**
 * @class   Device
 * @brief   **ITEM** for Remote and/or Local Values
 *********************************************************************************************************************/
class Device : public MenuValue {
  public:
    Device(const char* _CName, byte _RFID, byte _ValueType=0, byte _TodeIndex=0);
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
    unsigned long   LocalRefreshMS = 0;
    int             LastGetIOValue = 0;
    int             EETodeAddress = INONE;
    int             EEDevAddress = INONE;
    byte            TodeIndex = BNONE;

    //Device Settings stored in 'SubList' object
    PinSelect*      IOPin = 0;
    DevPinMode*     IOPinMode = 0;
    
  private:
    
};
//_____________________________________________________________________________________________________________________
#endif
