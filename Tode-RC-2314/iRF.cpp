/******************************************************************************************************************//**
 * @file    iRF.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _IRF_CPP
#define _IRF_CPP

#include "iRF.h"
//#####################################################################################################################
RadioAddress::RadioAddress():
  MenuValue(F("Address"),VTRW+VTDIG+VTHEX) {                     DBINITL(("RadioAddress::RadioAddress"))
    ValueRange(0x0000, 0xFFFF);
}
//-----------------------------------------------------------------------------------------------------
int RadioAddress::Value() {                                     DBENTERL(("RadioAddress::Value[GET]"))
  if (RF==0) {DBERRORL(("RadioAddress::Value[GET] No RF Object")) return 0; } 
  else {return RF->Address();} 
}
//-----------------------------------------------------------------------------------------------------
void RadioAddress::Value(int _Value, byte _Status) {    DBENTERAL(("RadioAddress::Value[SET]: "),((unsigned int)_Value))
  if ( RF==0 ) { DBERRORL(("RadioAddress::Value[SET] No RF Object")) return; }
  RF->Address(_Value);
  EEPROM.update(EMC_THISTODE+EMO_TODERF,highByte(_Value));
  EEPROM.update(EMC_THISTODE+EMO_TODERF,lowByte(_Value));;    
}
//#####################################################################################################################
RadioFrequency::RadioFrequency():
  MenuValue(F("Frequency"),VTRW) { ValueRange(410,441); }
//-----------------------------------------------------------------------------------------------------    
int RadioFrequency::Value() {
  if ( RF==0 ) { DBERRORL(("RadioFrequency::Value[GET] No RF Object")) return 0; } 
  return RF->Frequency(); 
}
//-----------------------------------------------------------------------------------------------------    
void RadioFrequency::Value(int _Value, byte _Status) {
  if ( RF==0 ) { DBERRORL(("RadioFrequency::Value[SET] No RF Object")) return; } 
  RF->Frequency(_Value); 
}
//#####################################################################################################################
RadioTxPower::RadioTxPower():
  MenuValue(F("Tx Power"),VTRW) { 
    ValueRange(0,3); 
    SetNumberName(0, F("30dBm"));
    SetNumberName(1, F("27dBm"));
    SetNumberName(2, F("24dBm"));
    SetNumberName(3, F("21dBm"));
}
//-----------------------------------------------------------------------------------------------------    
int RadioTxPower::Value() { 
  if ( RF==0 ) { DBERRORL(("RadioTxPower::Value[GET] No RF Object")) return 0; } 
  return int(RF->TxPower()); 
}
//-----------------------------------------------------------------------------------------------------    
void RadioTxPower::Value(int _Value, byte _Status) { 
  if ( RF==0 ) { DBERRORL(("RadioTxPower::Value[SET] No RF Object")) return; } 
  RF->TxPower(byte(_Value)); 
}
//#####################################################################################################################
RadioPCConn::RadioPCConn():
  MenuValue(F("PC Connex"),VTRW) {
      ValueRange(0,1);
      SetNumberName(0, F("Off"));
      SetNumberName(1, F("On"));
      iValue=0;                    // Set initial value; cannot use Value(set) because RF not set yet
}  
//-----------------------------------------------------------------------------------------------------
int RadioPCConn::Value() { return iValue; }
//-----------------------------------------------------------------------------------------------------
void RadioPCConn::Value(int _Value, byte _Status) { 
  iValue=_Value;
  if ( RF==0 ) { DBERRORL(("RadioPCConn::Value[SET] No RF Object")) return; } 
  if (_Value==1) { RF->Mode(E32_SLEEPMODE); } else { RF->Mode(E32_NORMMODE); }
}  
//-----------------------------------------------------------------------------------------------------
byte RadioPCConn::Loop() {
  if ( RF==0 ) { DBERRORL(("RadioPCConn::Loop No RF Object")) return 0; } 
  if ( iValue!= 1 ) { DBERRORL(("RadioPCConn::Loop but Not Set")) return 0; }
  if ( Serial.available() ) { Serial1.write(Serial.read()); }
  if ( Serial1.available() ) { Serial.write(Serial1.read()); }
}
//_____________________________________________________________________________________________________________________
#endif
