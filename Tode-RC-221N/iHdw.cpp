/******************************************************************************************************************//**
 * @file    iHdw.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _IHDW_CPP
#define _IHDW_CPP

#include "iHdw.h"
//#####################################################################################################################
AddTode::AddTode():
  MenuValue("Add Tode", VTRW+VTDIG+VTHEX) {
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
MemReset::MemReset(const char* _CName):
  MenuValue(_CName, VTRW) {
  ValueRange(0,1);
  SetNumberName(0, "No");
  SetNumberName(1, "Yes");
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
HdwSelect::HdwSelect(const char* _CName):
  MenuEEValue(_CName, EMC_HDWSELECT, VTRW+VTBYTE) {
  
  ValueRange(IOHDW_SIDEIO, IOHDW_NOTSET);
  SetNumberName(IOHDW_NOTSET,   "NotSet");
  SetNumberName(IOHDW_SIDEIO,   "SideIO");
}
//-----------------------------------------------------------------------------------------------------

//#####################################################################################################################
PinSelect::PinSelect(const char* _CName, byte _RFID, byte _EEOffset, HdwSelect* _Hardware):
  MenuEEValue(_CName, EMC_DEVSETTINGS+(_RFID*10)+_EEOffset, VTRW+VTBYTE) {     
    DBINITAAL(("PinSelect::PinSelect(CName,Hardware)"),(_CName),(int(_Hardware),HEX))
    if ( _Hardware==0 ) { DBERRORL(("PinSelect::PinSelect Hardware==0")) return; } else { Hdw = _Hardware; }
    if ( _RFID>=30 ) { DBERRORL(("PinSelect::PinSelect RFID>=10")) return; } else { RFID = _RFID; }
    SetNumberName(BVNOTSET, "NotSet");
    SetNumberName(IOHDW_NOTSET, "SetHdw");
}
//-----------------------------------------------------------------------------------------------------
byte PinSelect::NavValueSet(byte _Key) {               DBENTERAL(("PinSelect::NavValueSet"),(_Key,HEX))

  // LEFT
  if ( _Key==NAVKEYLEFT ) { NavSelected=SEL_NAME; DispItem(); return BNONE; } // NO-SAVE and EXIT
  
  // Check
  if ( Hdw==0 ) { DBERRORL(("PinSelect::NavValueSet Hdw==0")) return _Key; }
  if ( ConfigurePins() == IOHDW_NOTSET ) { Value(IOHDW_NOTSET); DisplayValue(); return _Key; }
  
  // UP & DOWN
  bool PinOkay = false;
  if ( _Key==NAVKEYUP || _Key==NAVKEYDOWN ) { 
    while (!PinOkay) {
      if (_Key==NAVKEYUP) { if (iSetTo==BVNOTSET) { iSetTo=0; } else { iSetTo++; } }
      else { iSetTo--; }
      if ( iSetTo>=0 && iSetTo<=31 ) { PinOkay = bitRead(DIO_Pins,iSetTo); }
      else if ( iSetTo>=32 && iSetTo<=63 ) { PinOkay = bitRead(AIO_Pins,iSetTo-32); }
      else if ( iSetTo>=64 && iSetTo<=72 ) { PinOkay = bitRead(XIO_Pins,iSetTo-64); }
      if (iSetTo>70) { iSetTo = BVNOTSET; break; }
      if (iSetTo<0) { iSetTo = BVNOTSET; break; }
    }
    DisplayValue(); return NAVKEYNONE;
  }
      
  // SET & OKAY
  if ( _Key==NAVKEYSET ) { NavSelected=SEL_VALUE; Value(iSetTo); DispItem(); return NAVKEYNONE; }  // SAVE and EXIT
  if ( _Key==NAVKEYOKAY ) { NavSelected=SEL_VALUE; DispItem(); return NAVKEYNONE; }                // NO-SAVE and EXIT  

}
//-----------------------------------------------------------------------------------------------------
int PinSelect::ConfigurePins() {
  
  int HdwVal = Hdw->Value();
  if ( LastHdwValue==HdwVal ) return HdwVal;
  if ( HdwVal == BVNOTSET ) { DBINFOL(("PinSelect::ConfigurePins HdwVal==IOHDW_NOTSET")) return IOHDW_NOTSET; }
  LastHdwValue = HdwVal;
  
  if ( Hdw->Value() == IOHDW_SIDEIO ) {               // Available Analog Pins on Side-Plug IO
    SetNumberName(57, "A3");
    SetNumberName(59, "A5");
    SetNumberName(61, "A7");
    SetNumberName(63, "A9");
    SetNumberName(65, "A11");
    SetNumberName(67, "A13");
    DIO_Pins = DSideIO_Pins;
    AIO_Pins = ASideIO_Pins;
    XIO_Pins = XSideIO_Pins;
  } 
  return HdwVal;
}
//#####################################################################################################################
Device::Device(const char* _CName, byte _RFID, byte _ValueType, byte _TodeIndex):
  MenuValue(_CName,_ValueType) {
  
  RFID = _RFID;
  TodeIndex = _TodeIndex;
  IsLocal = (TodeIndex == 0);
  SetNumberName(IVNOTSET,   "?");
  SetNumberName(IVEXPIRED,  "EXP");
}
//.....................................................................................................
Device::Device(int _EENameAddress, bool _NameSettable, byte _RFID, byte _ValueType, byte _TodeIndex):
  MenuValue(_EENameAddress, _NameSettable,_ValueType) {
    
  RFID = _RFID;
  TodeIndex = _TodeIndex;
  IsLocal = (TodeIndex == 0);
  SetNumberName(IVNOTSET,   "?");
  SetNumberName(IVEXPIRED,  "EXP");
}
//.....................................................................................................
Device::Device(byte _TodeIndex, byte _RFID):
  MenuValue((_TodeIndex*AEB_TODEALLOC) + AEB_TODEHEAD + (_RFID*AEB_DEVALLOC) +2, true , VTRW) {
  
  RFID = _RFID;
  TodeIndex = _TodeIndex;  
  IsLocal = (TodeIndex == 0);
  SetNumberName(IVNOTSET,   "?");
  SetNumberName(IVEXPIRED,  "EXP");
  
  // Calculate Addresses
  EETodeAddress = _TodeIndex*AEB_TODEALLOC;
  EEPROM.get(EETodeAddress, RFTodeAddress);
  EEDevAddress = EETodeAddress + AEB_TODEHEAD + (_RFID*AEB_DEVALLOC);  
}
//---------------------------------------------------------------------------------------------------------------------
int Device::Value() {                                   DBENTERL(("Device::Value(GET)")) 
  
  if ( IsLocal ) { bStatus = STSNORMAL; return IOValue(); }
  
  // Check if Value is Expired
  if ( millis() - RxValueTime > RXVALEXPIREMS ) bStatus = STSEXPIRED;
  return iValue;
}
//-----------------------------------------------------------------------------------------------------
void Device::Value(int _Value, byte _Status) {          DBENTERAAL(("Device::Value(SET,STS): "),(_Value),(_Status,HEX))

  if ( _Status == STSRFSET ) {
    if ( !IsLocal ) { DBERRORL(("Device::Value(SET) STSRFSET trying to set a non-local device.")) return; }
    DBINFO(("Device::Value(SET) STSRFSET IOValue(SET) by RF."))
    IOValue(_Value);
    
  } else if ( _Status == STSUSERSET ) {
    
    if ( IsLocal ) {
      DBINFO(("Device::Value(SET) STSUSERSET IOValue(SET) by User."))
      IOValue(_Value);     
    
    } else {
      DBINFOL(("Device::Value(SET) STSUSERSET Remote Device Set TxPacket."))
      if ( RF==0 ) { DBERRORL(("Device::Value(SET) STSUSERSET RF==0")) return; }
      RF->Send(new TxPacket(
                  EEPROM.read(EMC_SECNET),
                  PKT_SETVAL,
                  RFTodeAddress,
                  EEPROM.read( TodeIndex*AEB_TODEALLOC+EMO_TODEVER ),
                  RFID,
                  _Value )); 
    }
  } else if ( _Status == STSRFGOT ) { 
    DBINFOL(("Device::Value(SET) STSRFGOT save GOT Value on expiring timer."))
    RxValueTime=millis(); 
    iValue = _Value;
    if ( OnDisplay() ) DisplayValue();

  } else { 
    DBINFO(("Device::Value(SET) Status == other ... Set iValue"))
    iValue=_Value; 
  }
}
//-----------------------------------------------------------------------------------------------------
int Device::IOValue() { return 0; }
//-----------------------------------------------------------------------------------------------------
void Device::IOValue(int _Value) { return; }

//_____________________________________________________________________________________________________________________
#endif
