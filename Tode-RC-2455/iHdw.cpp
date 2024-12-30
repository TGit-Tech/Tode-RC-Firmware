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
  iValue = _Value; yStatus = _Status;
  if ( RF==0 ) { DBERRORL(("AddTode::Value(SET) RF==0")) return; }
  RF->Send(new TxPacket(EEPROM.read(EMC_SECNET),PKT_GETCONFIG,iValue));
  Status(STSRFGETTING);
  yStatusClockms = millis();  // Start Status Timer.
}
//------------------------------------------------------------------------------------------------------
byte AddTode::Loop() {
  if ( yStatus != STSRFGETTING ) return 0;
  if ( millis() - yStatusClockms > 10000 ) { Status(STSRFGETFAIL); }
}
//#####################################################################################################################
PinSelect::PinSelect(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, byte _PinType):
MenuEEValue(_CName, EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset, VTRW+VTBYTE) {     
DBINITAL(("PinSelect::PinSelect(CName,Hardware)"),(_CName))
    
    PinType=_PinType;
    if ( _RFID>=30 ) { DBERRORL(("PinSelect::PinSelect RFID>=10")) return; } else { RFID = _RFID; }
    SetNumberName(BNONE,        F("NotSet"));

    #if BACKPLANE==SA212K
      SetNumberName(57, F("A3"));
      SetNumberName(59, F("A5"));
      SetNumberName(61, F("A7"));
      SetNumberName(63, F("A9"));
      SetNumberName(65, F("A11"));
      SetNumberName(67, F("A13"));
    #elif BACKPLANE==BD235M || BACKPLANE==BD241S
      SetNumberName(54, F("A0"));
      SetNumberName(55, F("A1"));
      SetNumberName(56, F("A2"));
      SetNumberName(57, F("A3"));
      SetNumberName(58, F("A4"));
      SetNumberName(59, F("A5"));
      SetNumberName(60, F("A6"));
      SetNumberName(61, F("A7"));
      SetNumberName(62, F("A8"));
      SetNumberName(63, F("A9"));
      SetNumberName(64, F("A10"));
      SetNumberName(65, F("A11"));
      SetNumberName(66, F("A12"));
      SetNumberName(67, F("A13"));
      SetNumberName(68, F("A14"));
      SetNumberName(69, F("A15"));
    #elif BACKPLANE==SD23CF
      SetNumberName(57, F("A3"));
      SetNumberName(59, F("A5"));
      SetNumberName(61, F("A7"));
      SetNumberName(63, F("A9"));
      SetNumberName(65, F("A11"));
      SetNumberName(67, F("A13"));    
    #else
      #error PinSelect::PinSelect configuration in config.h not found.
    #endif
}
//.....................................................................................................
PinSelect::~PinSelect() {}
//-----------------------------------------------------------------------------------------------------
byte PinSelect::Navigate(byte _Key) {                         DBENTERAL(("PinSelect::Navigate(Key)"),(_Key,HEX))

  // SELECTOR
  if (      NavSelected == SEL_NONE ) {     _Key = NavNone(_Key); }
  else if ( NavSelected == SEL_NAME ) {     _Key = NavName(_Key); if (_Key==NAVKEYRIGHT) { NavSelected=SEL_VALUE; DispItem(); return NAVKEYNONE; } }
  else if ( NavSelected == SEL_VALUE ) {
    _Key = NavValue(_Key);    
    if ( NavSelected==SEL_SETVALUE ) {
      if (PinValueSet==0) PinValueSet=new SetPinValue(XPos, YPos, FirstNumberName, Value(), VMin, VMax, VType, PinType);
      DBINFOL(("MenuValue::Navigate new SetPinValue("))
    }
  }
  else if ( NavSelected == SEL_SETVALUE ) {
    if (PinValueSet==0) { DBERRORL(("MenuValue::Navigate PinValueSet==0")) }
    else { _Key = PinValueSet->NavValueSet(_Key); }
  }
  
  // Return Key Processing
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
                          int _VMin, int _VMax, byte _VType, byte _PinType):
SetValue(_XPos, _YPos, _FirstNumberName, _CurrValue, _VMin, _VMax, _VType) {
  //FirstNumberName is Pin Configuration
  PinType=_PinType;
}
//-----------------------------------------------------------------------------------------------------
byte SetPinValue::NavValueSet(byte _Key) {               
DBENTERAL(("SetPinValue::NavValueSet(Key)"),(_Key,HEX))
  DBINFOAL(("PinType: "),(PinType))
  // LEFT
  if ( _Key==NAVKEYLEFT ) { return NAVEXITVALUESET; } // NO-SAVE and EXIT
  
  // UP & DOWN
  bool PinOkay = false;
  if ( _Key==NAVKEYUP || _Key==NAVKEYDOWN ) { 
    while (!PinOkay) {
      // Increment(UP) or Decrement(DOWN) pin number.
      if (_Key==NAVKEYUP) { if (iSetTo==BVNOTSET) { iSetTo=0; } else { iSetTo++; } }
      else { iSetTo--; }

      if ( iSetTo>=0  && iSetTo<=31 )  { 
        if      (PinType==PT_DIGITAL) { PinOkay = bitRead(DIG_DPINS,iSetTo); }
        else if (PinType==PT_ANALOG)  { PinOkay = bitRead(ANA_DPINS,iSetTo); }
        else if (PinType==PT_PWM)     { PinOkay = bitRead(PWM_DPINS,iSetTo); } 
      } else if ( iSetTo>=32 && iSetTo<=63 ) { 
        if      (PinType==PT_DIGITAL) { PinOkay = bitRead(DIG_APINS,iSetTo-32); }
        else if (PinType==PT_ANALOG)  { PinOkay = bitRead(ANA_APINS,iSetTo-32); }
        else if (PinType==PT_PWM)     { PinOkay = bitRead(PWM_APINS,iSetTo-32); } 
      } else if ( iSetTo>=64 && iSetTo<=72 ) {
        if      (PinType==PT_DIGITAL) { PinOkay = bitRead(DIG_XPINS,iSetTo-64); }
        else if (PinType==PT_ANALOG)  { PinOkay = bitRead(ANA_XPINS,iSetTo-64); }
        else if (PinType==PT_PWM)     { PinOkay = bitRead(PWM_XPINS,iSetTo-64); } 
      }
      
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
Device::Device(byte _TodeIndex, byte _RFID, byte _ValueType):
MenuValue((_TodeIndex*AEB_TODEALLOC) + AEB_TODEHEAD + (_RFID*AEB_DEVALLOC) +2, true , _ValueType) {
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
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
int Device::EESettingAddrBO(byte _ByteOffset) {
  return EMC_DEVSETTINGS+(RFID*AEB_DEVSETTINGS)+_ByteOffset;
}
//---------------------------------------------------------------------------------------------------------------------
void Device::EEClear() {
DBENTERL(("Device::EEClear()")) 
  if ( SubList != 0 ) SubList->DelAllItems(SubList->DestEEClear);
  
  if ( EEDevAddress != INONE ) EEPROM.update(EEDevAddress, BNONE);
  DBINFOAL(("Device::EEClear() EEPROM clear @: "),(EEDevAddress))
}
//---------------------------------------------------------------------------------------------------------------------
int Device::Value() {                                   
DBENTERL(("Device::Value[GET]()")) 
  
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
    Status(_Status);
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
byte Device::NavValue(byte _Key) {                     
//DBENTERAL(("Device::NavValue"),(_Key,HEX))
  if ( _Key == NAVKEYSET && (VType&VTRW) ) { 
    
    // Prevent Set on Read-Only Devices.
    if ( DevType==DT_RO_ONOFF || DevType==DT_RO_DIST || DevType==DT_RO_ANAINPUT ) return NAVKEYNONE;
    NavSelected=SEL_SETVALUE;     
    return NAVKEYNONE;
    
  } else if ( _Key==NAVKEYLEFT ) {
    NavSelected=SEL_NAME;
    DispItem(); return NAVKEYNONE;
  }
  return _Key;
}
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
void Device::DispItem(byte _XPos, byte _YPos, byte _DispMode) {
//DBENTERAAL(("MenuValue::DispItem"),(_XPos),(_YPos))
  DispMode = _DispMode;
  if (_XPos!=0) XPos=_XPos; if (_YPos!=0) YPos=_YPos;
  DBINFOL(("OnDisplay(true)"))
  OnDisplay(true);
  if (DevType!=BNONE) { DisplayName(RFID); } else { DisplayName(); }
  if ( DispMode == DMNORMAL ) DisplayValue();
}
//-----------------------------------------------------------------------------------------------------
bool Device::DoLoop() {
  if ((millis() - LocalRefreshMS) < 500 ) return false;                               // Only Loop 1/2-Seconds
  LocalRefreshMS = millis();
  if (!IsLocal) {
    if (!OnDisplay()) return false;
    if(yStatus==STSRFGOT){if(millis()-RxValueTime>RXVALEXPIREMS){yStatus=STSEXPIRED;DisplayValue();}}
    return false;
  }
  
  if ( SubList != 0 ) { ChangingSettings = true; return false; }                      // Currently Changing Settings now
  if ( ChangingSettings && SubList==0 ) { ChangingSettings=false; ApplySettings(); }  // Changing Settings Complete
  
  if (!SettingsValid) return false;                                                   // Exit if Invalid
  return true;
}
//#####################################################################################################################
DevPinMode::DevPinMode(const __FlashStringHelper* _CName, byte _RFID, byte _EEOffset, bool _ReadOnly):
MenuEEValue(_CName, EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset, VTRW+VTBYTE) {
DBENTERAAL(("DevPinMode::DevPinMode(<CName>,RFID,EEOffset,Pin)@"),(_CName),(EMC_DEVSETTINGS+(_RFID*AEB_DEVSETTINGS)+_EEOffset,HEX))

  if (_ReadOnly) { ValueRange(2,3); } else { ValueRange(0,1); }
  SetNumberName(IVNOTSET,   F("NotSet"));
  SetNumberName(BNONE,      F("NotSet"));
  SetNumberName(0,          F("OLoOff"));
  SetNumberName(1,          F("OHiOff"));
  SetNumberName(2,          F("InHigh"));
  SetNumberName(3,          F("InLow"));

}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
//-----------------------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------------------
ReadSamples::~ReadSamples() {
  //EEClear();
}
//#####################################################################################################################
SampleBuffer::SampleBuffer(byte _SamplesToGet) {
DBENTERAL(("SampleBuffer::SampleBuffer(SamplesToGet)"),(_SamplesToGet))
  SamplesToGet = _SamplesToGet;
}
//-----------------------------------------------------------------------------------------------------
void SampleBuffer::Sample(int _ReadValue) {
  if ( _ReadValue < 0 ) { DBERRORAL(("ReadSamples::Sample(?ReadValue?)<0"),(_ReadValue)) return; }
  
  if ( SampleIdx < SamplesToGet ) { SampleIdx++; } else { SampleIdx = 1; }
  if ( SamplesGotten < SamplesToGet ) SamplesGotten++;
  //DBINFOAAL(("ReadSamples::Sample(?ReadValue?) SampleValues[?SampleIdx?]="),(_ReadValue),(SampleIdx))
  SampleValues[SampleIdx] = _ReadValue;
}
//-----------------------------------------------------------------------------------------------------
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
