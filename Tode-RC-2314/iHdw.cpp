/******************************************************************************************************************//**
 * @file    iHdw.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _IHDW_CPP
#define _IHDW_CPP

#include "iHdw.h"
//#####################################################################################################################
AddTode::AddTode():
  MenuValue(F("Add Tode"), VTRW+VTDIG+VTHEX) {
  ValueRange(0x0000, 0xFFFF);
  iValue = 0;
}
//------------------------------------------------------------------------------------------------------
int AddTode::Value() { return iValue; }
//------------------------------------------------------------------------------------------------------
void AddTode::Value(int _Value, byte _Status) {   DBINFOAL(("AddTode::Value[SET]: "),((unsigned int)_Value, HEX))
  iValue = _Value;
  if ( RF==0 ) { DBERRORL(("AddTode::Value(SET) RF==0")) return; }
  RF->Send(new TxPacket(EEPROM.read(EMC_SECNET),PKT_GETCONFIG,iValue));
}
//#####################################################################################################################
MemReset::MemReset(const __FlashStringHelper* _CName):
  MenuValue(_CName, VTRW) {
  ValueRange(0,1);
  SetNumberName(0, F("No"));
  SetNumberName(1, F("Yes"));
  iValue = 0;
}
//-----------------------------------------------------------------------------------------------------
int MemReset::Value() { return iValue; }
//-----------------------------------------------------------------------------------------------------
void MemReset::Value(int _Value, byte _Status) {                       DBENTERAL(("MemReset::Value(SET): "),(_Value))

  if (_Value==1) {
    // Clear all EEPROM and Reset Arduino
    for (int i=2; i<EEPROM.length();i++) {    //Skip resetting RFAddr at [0,1]
      if (i==EMC_SECNET) continue;            //Skip resetting SecNet
      EEPROM.update(i,0xFF); 
    }
    resetFunc();    
  }
}
//#####################################################################################################################
HdwSelect::HdwSelect(const __FlashStringHelper* _CName):
  MenuEEValue(_CName, EMC_HDWSELECT, VTRW+VTBYTE) {

  ValueRange(IOHDW_SIDEIO, IOHDW_NOTSET);
  SetNumberName(IOHDW_SIDEIO,   F("SideIO"));   //IOHDW_SIDEIO    0xFD
  SetNumberName(IOHDW_NOTSET,   F("NotSet"));   //IOHDW_NOTSET    0xFE
  SetNumberName(BNONE,          F("NotSet"));   //BNONE           0xFF    (RFC.h)
  
  
}
//-----------------------------------------------------------------------------------------------------

//#####################################################################################################################
PinSelect::PinSelect(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, HdwSelect* _Hardware):
MenuEEValue(_CName, EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset, VTRW+VTBYTE) {     
DBINITAAL(("PinSelect::PinSelect(CName,Hardware)"),(_CName),(int(_Hardware),HEX))
    if ( _Hardware==0 ) { DBERRORL(("PinSelect::PinSelect Hardware==0")) return; } else { Hdw = _Hardware; }
    if ( _RFID>=30 ) { DBERRORL(("PinSelect::PinSelect RFID>=10")) return; } else { RFID = _RFID; }
    
    SetNumberName(BNONE,        F("NotSet"));
    SetNumberName(IOHDW_NOTSET, F("SetHdw"));

    if ( Hdw == 0 ) { DBINFOL(("PinSelect::PinSelect Hdw* Not Set Yet.")) return; }
    if ( Hdw->Value() == IOHDW_SIDEIO ) {               // Available Analog Pins on Side-Plug IO
      SetNumberName(57, F("A3"));
      SetNumberName(59, F("A5"));
      SetNumberName(61, F("A7"));
      SetNumberName(63, F("A9"));
      SetNumberName(65, F("A11"));
      SetNumberName(67, F("A13"));
    }
}
//.....................................................................................................
PinSelect::~PinSelect() {
  Hdw = 0;
}
//-----------------------------------------------------------------------------------------------------
byte PinSelect::Navigate(byte _Key) {                         DBENTERAL(("PinSelect::Navigate(Key)"),(_Key,HEX))

  // SELECTOR
  if (      NavSelected == SEL_NONE ) {     _Key = NavNone(_Key); }
  else if ( NavSelected == SEL_NAME ) {     _Key = NavName(_Key); if (_Key==NAVKEYRIGHT) { NavSelected=SEL_VALUE; DispItem(); return NAVKEYNONE; } }
  else if ( NavSelected == SEL_SETNAME ) {  
    if (MenuNameSet==0) MenuNameSet=new SetMenuName(EENameAddress);
    _Key = MenuNameSet->NavNameSet(_Key); 
  }
  else if ( NavSelected == SEL_VALUE ) {
    _Key = NavValue(_Key);    
    if ( NavSelected==SEL_SETVALUE ) {
      if (PinValueSet==0) PinValueSet=new SetPinValue(XPos, YPos, FirstNumberName, Value(), VMin, VMax, VType, Hdw);
      DBINFOL(("MenuValue::Navigate new SetPinValue("))
    }
  }
  else if ( NavSelected == SEL_SETVALUE ) {
    if (PinValueSet==0) { DBERRORL(("MenuValue::Navigate PinValueSet==0")) }
    else { _Key = PinValueSet->NavValueSet(_Key); }
  }
  
  // Return Key Processing
  if ( _Key == NAVEXITNAMESET ) { 
    delete(MenuNameSet);
    MenuNameSet=0;
    DBINFOL(("MenuValue::Navigate Key == NAVEXITNAMESET"))
    NavSelected=SEL_NAME; 
    return NAVDSPNEWLIST;
  } 
  if ( _Key == NAVEXITVALUESET || _Key == NAVSAVEVALUESET ) { 
    if ( _Key == NAVSAVEVALUESET ) { 
      Value(PinValueSet->iSetTo, STSUSERSET);
      DBINFOAL(("PinSelect::Navigate Value(iSetTo, STSUSERSET)"),(PinValueSet->iSetTo))
    }
    delete(PinValueSet);
    PinValueSet=0;
    NavSelected=SEL_VALUE;
    DispItem();
  }
  return _Key;
}
//#####################################################################################################################
SetPinValue::SetPinValue( byte _XPos, byte _YPos, NumberName* _FirstNumberName, int _CurrValue, 
                          int _VMin, int _VMax, byte _VType, HdwSelect* _Hardware):
SetValue(_XPos, _YPos, _FirstNumberName, _CurrValue, _VMin, _VMax, _VType) {
  Hdw = _Hardware;
  //FirstNumberName is Pin Configuration
}
//-----------------------------------------------------------------------------------------------------
byte SetPinValue::NavValueSet(byte _Key) {               
DBENTERAL(("SetPinValue::NavValueSet(Key)"),(_Key,HEX))

  // LEFT
  if ( _Key==NAVKEYLEFT ) { return NAVEXITVALUESET; } // NO-SAVE and EXIT
  
  // Check Hardware is Set
  if ( Hdw==0 ) { DBERRORL(("SetPinValue::NavValueSet Hdw==0")) return _Key; }
  if ( Hdw->Value() == BVNOTSET || Hdw->Value() == IOHDW_NOTSET ) { 
    iSetTo=IOHDW_NOTSET;
    DisplayValue();
    return NAVSAVEVALUESET; 
  }
  
  // UP & DOWN
  bool PinOkay = false;
  if ( _Key==NAVKEYUP || _Key==NAVKEYDOWN ) { 
    while (!PinOkay) {
      if (_Key==NAVKEYUP) { if (iSetTo==BVNOTSET) { iSetTo=0; } else { iSetTo++; } }
      else { iSetTo--; }
      if ( iSetTo>=0 && iSetTo<=31 ) { PinOkay = bitRead(DSideIO_Pins,iSetTo); }
      else if ( iSetTo>=32 && iSetTo<=63 ) { PinOkay = bitRead(ASideIO_Pins,iSetTo-32); }
      else if ( iSetTo>=64 && iSetTo<=72 ) { PinOkay = bitRead(XSideIO_Pins,iSetTo-64); }
      if (iSetTo>70) { iSetTo = BVNOTSET; break; }
      if (iSetTo<0) { iSetTo = BVNOTSET; break; }
    }
    DisplayValue(); return NAVKEYNONE;
  }
      
  // SET & OKAY
  if ( _Key==NAVKEYSET ) { return NAVSAVEVALUESET; }  // SAVE and EXIT
  if ( _Key==NAVKEYOKAY ) { return NAVEXITVALUESET; }                // NO-SAVE and EXIT  
  
  return _Key;
}
//#####################################################################################################################
Device::Device(const __FlashStringHelper* _CName, byte _RFID, byte _ValueType, byte _TodeIndex):
MenuValue(_CName,_ValueType) {
DBENTERAAL(("Device::Device(_CName,_RFID,_ValuType,_TodeIndex)"),(_CName),(_RFID))  
  RFID = _RFID;
  TodeIndex = _TodeIndex;
  IsLocal = (TodeIndex == 0);
  SetNumberName(IVNOTSET,   F("?"));
  SetNumberName(IVEXPIRED,  F("EXP"));
}
//.....................................................................................................
Device::Device(int _EENameAddress, bool _NameSettable, byte _RFID, byte _ValueType, byte _TodeIndex):
MenuValue(_EENameAddress, _NameSettable,_ValueType) {
DBENTERAAL(("Device::Device(_EENameAddress,_NameSettable,_RFID,_ValueType,_TodeIndex)"),(_EENameAddress,HEX),(_NameSettable))
  RFID = _RFID;
  TodeIndex = _TodeIndex;
  IsLocal = (TodeIndex == 0);
  SetNumberName(IVNOTSET,   F("?"));
  SetNumberName(IVEXPIRED,  F("EXP"));
}
//.....................................................................................................
Device::Device(byte _TodeIndex, byte _RFID):
MenuValue((_TodeIndex*AEB_TODEALLOC) + AEB_TODEHEAD + (_RFID*AEB_DEVALLOC) +2, true , VTRW) {
DBENTERAAL(("Device::Device(_TodeIndex,_RFID)"),(_TodeIndex),(_RFID))    
  RFID = _RFID;
  TodeIndex = _TodeIndex;  
  IsLocal = (TodeIndex == 0);
  SetNumberName(IVNOTSET,   F("?"));
  SetNumberName(IVEXPIRED,  F("EXP"));
  
  // Calculate Addresses
  EETodeAddress = _TodeIndex*AEB_TODEALLOC;
  EEPROM.get(EETodeAddress, RFTodeAddress);
  EEDevAddress = EETodeAddress + AEB_TODEHEAD + (_RFID*AEB_DEVALLOC);  
}
//.....................................................................................................
void Device::EEClear() {
DBENTERL(("Device::EEClear()")) 
  if ( SubList != 0 ) SubList->DelAllItems(SubList->DestEEClear);
  
  if ( EEDevAddress != INONE ) EEPROM.update(EEDevAddress, BNONE);
  DBINFOAL(("Device::EEClear() EEPROM clear @: "),(EEDevAddress))
}
//---------------------------------------------------------------------------------------------------------------------
int Device::Value() {                                   
//DBENTERL(("Device::Value[GET]()")) 
  
  if ( IsLocal ) { 
    yStatus = STSNORMAL;
    if ( EPinMode != PM_OHIOFF ) { return IOValue(); } else {
      if ( IOValue() ) { return 0; } else { return 1; } // Reverse On/Off
    }
  }
  
  // Check if Value is Expired
  if ( millis() - RxValueTime > RXVALEXPIREMS ) yStatus = STSEXPIRED;
  return iValue;
}
//-----------------------------------------------------------------------------------------------------
void Device::Value(int _Value, byte _Status) {          
DBENTERAAL(("Device::Value[SET](_Value,_Status)"),(_Value),(_Status,HEX))

  if ( _Status == STSRFSET ) {
    if ( !IsLocal ) { DBERRORL(("Device::Value[SET] STSRFSET trying to set a non-local device.")) return; }
    DBINFO(("Device::Value[SET] STSRFSET IOValue(SET) by RF."))
    if ( EPinMode != PM_OHIOFF ) { IOValue(_Value); return; } else {
      if ( _Value ) { IOValue(0); } else { IOValue(1); return; } // Reverse On/Off
    }

  } else if ( _Status == STSUSERSET ) {
    
    if ( IsLocal ) {
      if ( EPinMode != PM_OHIOFF ) { IOValue(_Value); return; } else {
        if ( _Value ) { IOValue(0); } else { IOValue(1); return; } // Reverse On/Off
      }     
    
    } else {
      DBINFOL(("Device::Value[SET] STSUSERSET Remote Device Set TxPacket."))
      if ( RF==0 ) { DBERRORL(("Device::Value[SET] STSUSERSET RF==0")) return; }
      RF->Send(new TxPacket(
                  EEPROM.read(EMC_SECNET),
                  PKT_SETVAL,
                  RFTodeAddress,
                  EEPROM.read( TodeIndex*AEB_TODEALLOC+EMO_TODEVER ),
                  RFID,
                  _Value )); 
    }
  } else if ( _Status == STSRFGOT ) { 
    DBINFOL(("Device::Value[SET] STSRFGOT save GOT Value on expiring timer."))
    RxValueTime=millis(); 
    iValue = _Value;
    if ( OnDisplay() ) DisplayValue();

  } else { 
    DBINFO(("Device::Value[SET] Status == other ... Set iValue"))
    iValue=_Value; 
  }
}
//-----------------------------------------------------------------------------------------------------
int Device::IOValue() { return 0; }
//-----------------------------------------------------------------------------------------------------
void Device::IOValue(int _Value) { return; }
//-----------------------------------------------------------------------------------------------------
void Device::ApplySettings() { return; }
//-----------------------------------------------------------------------------------------------------
bool Device::ApplyPinSettings(byte _Pin, byte _pinMode) {
DBENTERAAL(("Device::ApplyPinSettings(Pin,pinMode)"),(_Pin),(_pinMode))

  if ( 0 > _pinMode || _pinMode > 3 ) { 
    DBINFOAL(("Device::ApplyPinSettings() 0>Value>3"),(_pinMode)) 
  } else {
    // Set pinMode
    switch(_pinMode) {
      case PM_OLOOFF:
        digitalWrite(_Pin,LOW);pinMode(_Pin,OUTPUT);break;
      case PM_OHIOFF:
        digitalWrite(_Pin,HIGH);pinMode(_Pin,OUTPUT);break;
      case PM_INHIGH:
        pinMode(_Pin,INPUT_PULLUP);break;
      case PM_INLOW:
        pinMode(_Pin,INPUT);break;
      default:
        break;
    }
  }
  if ( 70 < _Pin || _Pin < 0 ) { DBERRORAL(("Device::ApplyPinSettings 70 < Pin < 0"),(_Pin)) return false; }
  return true;  
}
//-----------------------------------------------------------------------------------------------------
bool Device::DoLoop() {
  if (!IsLocal) return false;
  if ((millis() - LocalRefreshMS) < 500 ) return false;                               // Only Loop 1/2-Seconds
  LocalRefreshMS = millis();
  
  if ( SubList != 0 ) { ChangingSettings = true; return false; }                      // Currently Changing Settings now
  if ( ChangingSettings && SubList==0 ) { ChangingSettings=false; ApplySettings(); }  // Changing Settings Complete

  if (!SettingsValid) return false;                                                   // Exit if Invalid
  return true;
}
//#####################################################################################################################
DevPinMode::DevPinMode(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset):
MenuEEValue(_CName, EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset, VTRW+VTBYTE) {
DBENTERAAL(("DevPinMode::DevPinMode(<CName>,RFID,EEOffset,Pin)@"),(_CName),(EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset,HEX))

  ValueRange(0,3);
  SetNumberName(IVNOTSET,   F("NotSet"));
  SetNumberName(BNONE,      F("NotSet"));
  SetNumberName(0,          F("OLoOff"));
  SetNumberName(1,          F("OHiOff"));
  SetNumberName(2,          F("InHigh"));
  SetNumberName(3,          F("InLow"));

}
//.....................................................................................................
DevPinMode::~DevPinMode() {
  //EEClear();
}
//#####################################################################################################################
DevModValue::DevModValue(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, bool _Denominator):
MenuEEValue(_CName, EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset, VTRW+VTDIG) {
DBENTERAAL(("DevModValue::DevModValue(<CName>,RFID,EEOffset)@"),(_CName),(EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset,HEX))

  ValueRange(-9999,9999);
  if (_Denominator) SetNumberName(0,   F("10K"));

}
//.....................................................................................................
DevModValue::~DevModValue() {
  //EEClear();
}
//#####################################################################################################################
ReadSamples::ReadSamples(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset):
MenuEEValue(_CName, EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset, VTRW+VTDIG+VTBYTE) {
DBENTERAAL(("ReadSamples::ReadSamples(<CName>,RFID,EEOffset)@"),(_CName),(EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset,HEX))

  ValueRange(0,20);
  SetNumberName(0,        F("Off"));
  SetNumberName(BVNOTSET, F("Off"));

}
//.....................................................................................................
bool ReadSamples::IsSampling() {
  if ( SamplesToGet != Value() ) {
    SamplesToGet = Value();
    if ( SamplesToGet > 20 ) { DBERRORAL(("ReadSamples::IsSampling() ?Value()?>20"),(SamplesToGet)) return false; }
    if ( SamplesToGet <= 0 ) { DBINFOAL(("ReadSamples::IsSampling() ?Value()?<=0 [OFF]"),(SamplesToGet)) return false; }    
    SamplesGotten = 0; 
    SampleIdx = 0;
  }
  return ( 0 < SamplesToGet && SamplesToGet <= 20 );
}
//.....................................................................................................
void ReadSamples::Sample(int _ReadValue) {
  if ( _ReadValue < 0 ) { DBERRORAL(("ReadSamples::Sample(?ReadValue?)<0"),(_ReadValue)) return; }
  if ( !IsSampling() ) { DBERRORL(("ReadSamples::Sample() !IsSampling")) return; }
  
  if ( SampleIdx < SamplesToGet ) { SampleIdx++; } else { SampleIdx = 1; }
  if ( SamplesGotten < SamplesToGet ) SamplesGotten++;
  //DBINFOAAL(("ReadSamples::Sample(?ReadValue?) SampleValues[?SampleIdx?]="),(_ReadValue),(SampleIdx))
  SampleValues[SampleIdx] = _ReadValue;
}
//.....................................................................................................
int ReadSamples::SampleAve() {
  if ( !IsSampling() ) { DBERRORL(("ReadSamples::SampleAve() !IsSampling")) return 0; }
  double AveSum=0; 

  for ( int i=1; i<=SamplesGotten; i++) { 
    //DBINFOAAL(("ReadSamples::SampleAve() SampleValues[?i?] = ?Value?"),(i),(SampleValues[i]))
    AveSum = AveSum + SampleValues[i]; 
  }
  return int(AveSum/double(SamplesGotten));
}
//.....................................................................................................
ReadSamples::~ReadSamples() {
  //EEClear();
}
//#####################################################################################################################
SampleBuffer::SampleBuffer(byte _SamplesToGet) {
DBENTERAL(("SampleBuffer::SampleBuffer(SamplesToGet)"),(_SamplesToGet))
  SamplesToGet = _SamplesToGet;
}
//.....................................................................................................
void SampleBuffer::Sample(int _ReadValue) {
  if ( _ReadValue < 0 ) { DBERRORAL(("ReadSamples::Sample(?ReadValue?)<0"),(_ReadValue)) return; }
  
  if ( SampleIdx < SamplesToGet ) { SampleIdx++; } else { SampleIdx = 1; }
  if ( SamplesGotten < SamplesToGet ) SamplesGotten++;
  //DBINFOAAL(("ReadSamples::Sample(?ReadValue?) SampleValues[?SampleIdx?]="),(_ReadValue),(SampleIdx))
  SampleValues[SampleIdx] = _ReadValue;
}
//.....................................................................................................
int SampleBuffer::SampleAve() {
  double AveSum=0; 

  for ( int i=1; i<=SamplesGotten; i++) { 
    //DBINFOAAL(("ReadSamples::SampleAve() SampleValues[?i?] = ?Value?"),(i),(SampleValues[i]))
    AveSum = AveSum + SampleValues[i]; 
  }
  return int(AveSum/double(SamplesGotten));
}
//_____________________________________________________________________________________________________________________
#endif
