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
RadioAirSpeed::RadioAirSpeed():
  MenuValue(F("DRate(bps)"),VTRW) {
    #if RADIO==E32_433T30D
      ValueRange(0,5);
      SetNumberName(0,F("300"));
      SetNumberName(1,F("1200"));
      SetNumberName(2,F("2400"));
      SetNumberName(3,F("4800"));
      SetNumberName(4,F("9600"));
      SetNumberName(5,F("19200"));
      SetNumberName(6,F("19200"));
      SetNumberName(7,F("19200"));
    #elif RADIO==E220_400T30D
      ValueRange(2,7);
      SetNumberName(0,F("2400"));
      SetNumberName(1,F("2400"));
      SetNumberName(2,F("2400"));
      SetNumberName(3,F("4800"));
      SetNumberName(4,F("9600"));
      SetNumberName(5,F("19200"));  
      SetNumberName(6,F("38400"));
      SetNumberName(7,F("62500"));
    #elif RADIO==E22_400T33D
      ValueRange(2,7);
      SetNumberName(0,F("2400")); // Tested. It is actually 2400bps
      SetNumberName(1,F("2400")); // Tested. It is actually 2400bps
      SetNumberName(2,F("2400"));
      SetNumberName(3,F("4800"));
      SetNumberName(4,F("9600"));
      SetNumberName(5,F("19200"));  
      SetNumberName(6,F("38400"));
      SetNumberName(7,F("62500"));
    #else
      #error RadioAirSpeed::RadioAirSpeed RADIO in config.h not found.
    #endif
}
//-----------------------------------------------------------------------------------------------------
int RadioAirSpeed::Value() {                                  DBENTERL(("RadioAirSpeed::Value[GET]"))
  if (RF==0) {DBERRORL(("RadioAirSpeed::Value[GET] No RF Object")) return 0; } 
  else {return RF->AirSpeed();}
}
//-----------------------------------------------------------------------------------------------------
void RadioAirSpeed::Value(int _Value, byte _Status) { DBENTERAL(("RadioAddress::Value[SET]: "),((unsigned int)_Value))
  if ( RF==0 ) { DBERRORL(("RadioAirSpeed::Value[SET] No RF Object")) return; }
  RF->AirSpeed(_Value);
}
//#####################################################################################################################
RadioFrequency::RadioFrequency():
  MenuValue(F("Frequency"),VTRW) { 
#if RADIO==E32_433T30D
  ValueRange(410,441);
#elif RADIO==E220_400T30D
  ValueRange(410,493);
#elif RADIO==E22_400T33D
  ValueRange(410,493);
#else
  #error RadioFrequency::RadioFrequency RADIO in config.h not found.
#endif
}
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
    #if RADIO==E32_433T30D
      SetNumberName(0, F("30dBm"));
      SetNumberName(1, F("27dBm"));
      SetNumberName(2, F("24dBm"));
      SetNumberName(3, F("21dBm"));
    #elif RADIO==E220_400T30D
      SetNumberName(0, F("30dBm"));
      SetNumberName(1, F("27dBm"));
      SetNumberName(2, F("24dBm"));
      SetNumberName(3, F("21dBm"));
    #elif RADIO==E22_400T33D
      SetNumberName(0, F("33dBm"));
      SetNumberName(1, F("30dBm"));
      SetNumberName(2, F("27dBm"));
      SetNumberName(3, F("24dBm"));
    #else
      #error RadioTxPower::RadioTxPower() RADIO in config.h not found.
    #endif
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
#if RADIO==E22_400T33D
RadioNetID::RadioNetID():
  MenuValue(F("NetID"),VTRW+VTBYTE) {
    ValueRange(0,0xFF);
}
//-----------------------------------------------------------------------------------------------------    
int RadioNetID::Value() {
  if ( RF==0 ) { DBERRORL(("RadioNetID::Value[GET] No RF Object")) return 0; }
  return int(RF->NetID()); 
}
//-----------------------------------------------------------------------------------------------------    
void RadioNetID::Value(int _Value, byte _Status) {
  if ( RF==0 ) { DBERRORL(("RadioNetID::Value[SET] No RF Object")) return; }
  RF->NetID(_Value);
}
#endif
//_____________________________________________________________________________________________________________________
#endif
