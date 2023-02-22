/******************************************************************************************************************//**
 * @file    iLib.h
 * @brief   Menu-Items General Extension Library.
 *          Adds the *SetNameTool* class.
 *********************************************************************************************************************/
#ifndef _ILIB_H
#define _ILIB_H

#include "Menu.h"
#include <EEPROM.h>

#define RXVALEXPIREMS 5000    // 5-Seconds till remote value expires
#define EEMAXNAMESIZE 10
/******************************************************************************//**
 * @defgroup NS NS - Name Set Constants
 * @{
 *********************************************************************************/
#define NSTITLE       28
#define NSPOS         29
#define NSEXIT        30
#define NSSAVE        31
///@}
/******************************************************************************//**
 * @defgroup VT VT - Value Type
 * @{
 *********************************************************************************/
#define VTRW      0x01  ///< b0000 0001   - (0) Read-Only [default],  (1) Write
#define VTDIG     0x02  ///< b0000 001x   - (0) Increment [default],  (1) Digit Set
#define VTHEX     0x04  ///< b0000 01xx   - (0) Decimal [default],    (1) Hexidecimal
#define VTBYTE    0x08  ///< b0000 1xxx   - (0) Integer [default],    (1) Byte Size
///@}
/******************************************************************************//**
 * @defgroup IV IV - Integer Value() Constants
 * @{
 *********************************************************************************/
#define IVEENOTSET     0xFFFF  ///< EEPROM compatible Not Set
#define IVNOTSET       0x8000  ///< 2s Compliment -32,768
#define IVEXPIRED      0x8001  ///< 2s Compliment -32,767
///@}
/******************************************************************************//**
 * @defgroup BV BV - Byte Value() Constants
 * @{
 *********************************************************************************/
#define BVNOTSET       0xFF     ///< VTBYTE Value Not Set
///@}
/******************************************************************************************************************//**
 * @class   SetMenuName
 * @brief   Ability to set a *NAME* via display.
 *********************************************************************************************************************/
class SetMenuName : public GetDisplay {
  public:
    SetMenuName(int _EENameAddress);
    virtual byte          NavNameSet(byte _Key);                        ///< NAME SET Navigate
    
  private:
    char                  cSetTo = NSTITLE;
    char                  SetToName[EEMAXNAMESIZE];
    byte                  SetIdx = 0;                                   ///< Set-To Array[] current char-index
    virtual void          DspChar(bool _Selected = false);              ///< Per-Char Display
    virtual void          DspSetToName(bool _Selected = false);
    byte                  Type=0;
    byte                  Size=0;
    bool                  NameSetOnScreen = false;
    virtual void          DisplayNameSet();                    ///< Display the Full-Screen Name-Set
    char                  FName[EEMAXNAMESIZE+1];                       ///< C-Style Name Char[] for Name set/get.
    int                   EENameAddress = -1;                           ///< NAME is at EEPROM Addr...

};
/******************************************************************************************************************//**
 * @class   MenuName
 * @brief   **ITEM** adds *NAME* to MenuItem stored in either MEM or EEPROM determined by Constructor.
 *********************************************************************************************************************/
class MenuName : public MenuItem {
  public:
    MenuName(const __FlashStringHelper* _CName = 0, MenuList* _SubList=0);          ///< MEM F(Name) Constructor
    MenuName(const __FlashStringHelper* _CName, byte _ReturnKey);                   ///< MEM F(Name) assigned return KEY
    MenuName(int _EENameAddress, bool _NameSettable = false);                       ///< EEPROM Name Constructor
    MenuName(int _EENameAddress, byte _Offset, byte _ReturnKey);
    
    virtual const char*   Name();                                       ///< GET Item Name
    virtual void          Name(const __FlashStringHelper* _Name);       ///< SET Item Name via F()
    virtual void          Name(const char* _Name);                      ///< SET Item Name for EEPROM Save
    int                   EENameAddress = -1;                           ///< NAME is at EEPROM Addr...
    
    
  protected:
    virtual byte          Navigate(byte _Key) override;                   ///< Keypad Navigate by \ref KEY
    virtual void          DispItem(byte _XPos=0, byte _YPos=0, byte _DispMode = DMNORMAL) override;  ///< Display the Item
    virtual byte          NavNone(byte _Key);                             ///< NONE Navigate
    virtual byte          NavName(byte _Key);                             ///< NAME Navigate
    
    virtual void          DisplayName(byte _ID=BNONE);                    ///< Display the Row-Name
    virtual void          OnDisplay(bool _OnDisplay) override;
    virtual bool          OnDisplay() override;    
    SetMenuName*          MenuNameSet=0;
    virtual void          BuildSubList();
    char                  FName[EEMAXNAMESIZE+1];                         ///< C-Style Name Char[] for Name set/get.    
    
  private:
    const __FlashStringHelper*  CName = 0;                                    ///< Constant "Name()" pointer
    byte                        ReturnKey = NAVKEYNONE;                       ///<
    bool                        NameSettable = false;                         ///< NAME Settable?

};
struct NumberName     { int Number = -1; const __FlashStringHelper* Name = 0; NumberName* Next = 0; };   ///< Text-Name Numbers
/******************************************************************************************************************//**
 * @class   SetMenuName
 * @brief   Ability to set a *VALUE* via display.
 *********************************************************************************************************************/
class SetValue : public GetDisplay {
  public:
    SetValue(byte _XPos, byte _YPos, NumberName* _FirstNumberName, int _CurrValue, int _VMin, int _VMax, byte _VType);
    virtual byte          NavValueSet(byte _Key);             ///< Navigate a Value Set
    int                   iSetTo = 0;                       ///< Set-To Integer Variable    
    virtual void          DisplayValue();
  
  protected:   

  private:
    virtual const __FlashStringHelper*    GetNumberName(int _Value);        ///< Search if a Number has a Text-Name
    
    NumberName*           FirstNumberName = 0;                              ///< Link-List pointer for NumberName
    NumberName*           CurrNumberName = 0;                               ///< Link-List pointer for NumberName
    byte                  VRngDigits = 0, VDigitPos = 0, yVSignDig = 0;     ///< Selected Digit Position (Used only by VTDIG-Set)
    bool                  bVSigned = false;
    virtual byte          DigitCount(int _Value, byte _Base);
    int                   VMin = -9999;
    int                   VMax = 9999;                      ///< -32,766 to 32,767 (but full per digit set)
    byte                  VType = 0;                        ///< Value Type
    byte                  XPos=0, YPos=0;
};
/******************************************************************************************************************//**
 * @class   MenuValue
 * @brief   **ITEM** adds local *VALUE* to MenuName for 'Name->Value' Pairs 
 *          Offers Number-Naming ability 
 *          Offers Value Setting ability with optional Digit-Set or Incremental
 *********************************************************************************************************************/
class MenuValue : public MenuName {
  public:
    MenuValue(const __FlashStringHelper* _CName, byte _ValueType = 0);                    ///< CName + CValue
    MenuValue(const __FlashStringHelper* _CName, byte _ValueType, byte _ReturnKey);
    MenuValue(int _EENameAddress, bool _NameSettable=false, byte _ValueType = 0);         ///< EEName + CValue

    virtual byte          Navigate(byte _Key) override;                                   ///< Keypad Navigate by \ref KEY
    virtual void          DispItem(byte _XPos=0, byte _YPos=0, byte _DispMode = DMNORMAL) override;  ///< Display the Item
    virtual int           Value();                                                        ///< GET Item Value
    virtual void          Value(int _Value, byte _Status = STSNORMAL);                    ///< SET Item Value
    virtual void          ValueRange(int _VMin, int _VMax);                               ///< SET the Value Range
    virtual void          SetNumberName(int _Value, const __FlashStringHelper* _NName);   ///< SET a Text-Name for certain NUMBERS
    virtual void          DisplayValue();                                                 ///< Display the Row-Value  
    int                   VMin = -9999;
    int                   VMax = 9999;                      ///< -32,766 to 32,767 (but full per digit set)
        
  protected:
    virtual byte          NavValue(byte _Key);                ///< Navigate the Value
    NumberName*           FirstNumberName = 0;              ///< Link-List pointer for NumberName
    int                   iValue = IVNOTSET;                ///< 2s Compliment to -32,768 [ 0x8000 ]
    byte                  VType = 0;                        ///< Value Type
    
  private:
    SetValue*                             ValueSet = 0;
    NumberName*                           CurrNumberName = 0;               ///< Link-List pointer for NumberName
    virtual const __FlashStringHelper*    GetNumberName(int _Value);        ///< Search if a Number has a Text-Name
};
/******************************************************************************************************************//**
 * @class   MenuEEValue
 * @brief   **ITEM** adds On-Display *Per-Digit* SET feature to MenuValue.
 *********************************************************************************************************************/
class MenuEEValue : public MenuValue {
  public:
    MenuEEValue(const __FlashStringHelper* _CName, int _EEValAddress, byte _ValueType);       ///< CName + EE Value
    MenuEEValue(int _EENameAddress, bool _NameSettable, int _EEValAddress, byte _ValueType);  ///< EEName + EE Value
    
    virtual int           Value() override;                                                   ///< GET Item Value
    virtual void          Value(int _Value, byte _Status = STSNORMAL) override;               ///< SET Item Value
    virtual void          EEClear() override;
    
  protected:
    int                   EEValAddress = 0xFFFF;            ///< EEPROM Address of Value Location
  private: 
};
//_____________________________________________________________________________________________________________________
#endif
