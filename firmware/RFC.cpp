/******************************************************************************************************************//**
 * @file    RFC.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _RFC_CPP
#define _RFC_CPP

#include "RFC.h"
//#####################################################################################################
RxPacket::RxPacket() {  }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
byte    RxPacket::Type()      { return Bytes[PKB_TYPE]; }
int     RxPacket::FromRF()    { return word(Bytes[PKB_FROM_RFH],Bytes[PKB_FROM_RFL]); }
byte    RxPacket::Version()   { return Bytes[PKB_TODEVER]; }
byte    RxPacket::RFID()      { return Bytes[PKB_RFID]; }
int     RxPacket::SetValue()  { return word(Bytes[PKB_VALUEH],Bytes[PKB_VALUEL]); }
//-----------------------------------------------------------------------------------------------------
bool RxPacket::RxByte(byte _Byte) {
  DBINFOAAL(("RxPacket::RxByte(Byte)@Idx: "),(_Byte,HEX),(NextIdx))
  if ( NextIdx<58 ) { Bytes[NextIdx] = _Byte; }
  else { 
    if ( ExtraBytes == 0 ) ExtraBytes = new ArduinoQueue<byte>(512);
    if ( ExtraBytes->isFull() ) { DBERRORL(("RxPacket::RxByte ExtraBytes->isFull()")) return true; }
    ExtraBytes->enqueue(_Byte);
  }
  NextIdx++;
  if (NextIdx==5) { if (!IsSecure()) return true; }         //End on Insecure
  if (NextIdx>5 && NextIdx>=Size) return true;            //End
  return false;
}
//-----------------------------------------------------------------------------------------------------
bool RxPacket::IsSecure() {                                       DBENTERL(("RxPacket::IsSecure"))
  int SecNet = word(Bytes[PKB_SECH],Bytes[PKB_SECL]);   //0,1
  byte Sc = 0; int Sz = 0;int i = 0; int y = 0;

  while ( i<14 ) {bitWrite(Sc,y,bitRead(SecNet,i));i++;bitWrite(Sz,y,bitRead(SecNet,i));i++;y++;}
  bitWrite(Sc,y,bitRead(Size,i));y++;i++;
  bitWrite(Sz,y,bitRead(Size,i));
  
  DBINFOAL(("RxPacket::IsSecure Sc="),(Sc,HEX))
  DBINFOAL(("RxPacket::IsSecure Sz="),(Sz))
  if ( Sc != EEPROM.read(EMC_SECNET) ) { DBERRORL(("RxPacket::IsSecure FAILED SECURITY")) return false; }
  Size = Sz;
  return true;
}
//-----------------------------------------------------------------------------------------------------
bool RxPacket::IsValid() {
  if ( Size==0 ) return false;
  return true;
}
//-----------------------------------------------------------------------------------------------------
int RxPacket::Value(byte _RFID) {             DBENTERAL(("RxPacket::Value(GET) RFID: "),(_RFID))
  // Check bytes for RFID then return Value
  for ( int i=PKB_RFID; i<Size; i=i+3 ) {
    if ( Bytes[i]==_RFID ) {
      DBINFOL(("RxPacket::Value(GET) RFID Found."))
      return word(Bytes[i+1],Bytes[i+2]);
    }
  }
  DBERRORL(("RxPacket::Value(GET) RFID NOT Found."))
  return VALNOTSET;
}
//-----------------------------------------------------------------------------------------------------
void RxPacket::SaveTodeConfig(int _EEAddress) {      
  DBENTERAL(("RxPacket::SaveTodeConfig(EEAddress): "),(_EEAddress))
  //  TODEVER = 2
  EEPROM.update(_EEAddress+EMO_TODEVER, Version() );  DBINFOAL(("Save Version: "),(Version()))

  DBINFOAL(("Byte Size: "),(Size))
  int EEAddr = _EEAddress+EMO_TODENAME;             // Start save at Name
  
  for ( int i = PKB_TODECONFIG+1; i<Size; i++ ) {      // +1 to skip Version 
    if ( i<58 ) {
      EEPROM.update(EEAddr, Bytes[i]); EEAddr++;
      DBINFOAAL(("RxPacket::SaveTodeConfig Bytes[] EEPROM.update(Addr,Byte): "),(EEAddr),(Bytes[i],HEX))
    } else {
      // Use ExtraBytes
      if ( ExtraBytes == 0 ) { DBERRORL(("RxPacket::SaveTodeConfig ExtraBytes == 0")) return; }
      if ( ExtraBytes->isEmpty() ) { DBERRORL(("RxPacket::SaveTodeConfig ExtraBytes->isEmpty()")) return; }
      DBINFOAAL(("RxPacket::SaveTodeConfig ExtraBytes EEPROM.update(Addr,Byte): "),(EEAddr),(ExtraBytes->getHead(),HEX))
      EEPROM.update(EEAddr, ExtraBytes->dequeue()); EEAddr++;
    }
  }
  
}
//#####################################################################################################
TxPacket::TxPacket(byte _SecNet, byte _Type, int _ToRF, byte _Ver, byte _DevRFID, int _Value) {
  DBINITAAL(("TxPacket::TxPacket[.,ToRF,Ver,.,.]: "),((unsigned int)_ToRF,HEX),(_Ver))

  // Packet-Type to Byte-Size ( Size is Byte[i]+1 )
  if      ( _Type == PKT_GETCONFIG )  { DBINFOL(("TxPacket::TxPacket PKT_GETCONFIG")) Size = 8; }
  else if ( _Type == PKT_GETVALS )    { DBINFOL(("TxPacket::TxPacket PKT_GETVALS")) Size = 9; }
  else if ( _Type == PKT_GETVAL )     { DBINFOL(("TxPacket::TxPacket PKT_GETVAL")) Size = 10; }
  else if ( _Type == PKT_SETVAL )     { DBINFOL(("TxPacket::TxPacket PKT_SETVAL")) Size = 12; }
  else if ( _Type == PKT_GOTVAL )     { DBINFOL(("TxPacket::TxPacket PKT_GOTVAL")) Size = 12; }
  
  else if ( _Type == PKT_GOTCONFIG )  { DBINFOL(("TxPacket::TxPacket PKT_GOTCONFIG")) Size=PKB_TODECONFIG; }    // Start TodeConfig at Byte[9]
  else if ( _Type == PKT_GOTVALS ) { DBINFOL(("TxPacket::TxPacket PKT_GOTVALS")) Size=PKB_RFID; }               // Start Values at Byte[9]
  
  else { DBERRORAL(("UNIDENTIFIED Packet Type : "),(_Type)) return; }

  Bytes[PKB_TO_RFH] = highByte(_ToRF);    //0 
  Bytes[PKB_TO_RFL] = lowByte(_ToRF);     //1
  //Bytes[PKB_CHANNEL] set in Send()      //2   Set in E32::Send()
  //Bytes[PKB_SECH] = highByte(SecNet);   //3   Set in Secure()
  //Bytes[PKB_SECL] = lowByte(SecNet);    //4   Set in Secure()
  Bytes[PKB_TYPE] = _Type;                //5
  //Bytes[PKB_FROM_RFH] set in Send()     //6   Set in E32::Send()
  //Bytes[PKB_FROM_RFL] set in Send()     //7   Set in E32::Send()
  Bytes[PKB_TODEVER] = _Ver;              //8

  // ------ End of Static Sets ----------
  Bytes[PKB_RFID] = _DevRFID;             //9
  Bytes[PKB_VALUEH] = highByte(_Value);   //10
  Bytes[PKB_VALUEL] = lowByte(_Value);    //11
}
//-----------------------------------------------------------------------------------------------------
void TxPacket::TxByte(byte _Byte) {
  
  if ( Size<58 ) { Bytes[Size] = _Byte; } 
  else {
    if ( ExtraBytes == 0 ) ExtraBytes = new ArduinoQueue<byte>(512);
    if ( ExtraBytes->isFull() ) { DBERRORL(("TxPacket::TxByte ExtraBytes->isFull()")) return; }
    ExtraBytes->enqueue(_Byte);     
  }
  DBINFOAAL(("TxPacket::TxByte(Byte)(Idx/Size): "),(_Byte,HEX),(Size))
  Size++;
}
//-----------------------------------------------------------------------------------------------------
int TxPacket::Secure() {
  int i = 0; int y = 0; int Ret = 0xFFFF; byte EESec = EEPROM.read(EMC_SECNET);
  // i = 0 to 15 (Change: 4/7/ Secure Code must be smaller than 0x7F)
  // Use top bit for Size [ Size is an int ]
  // bit: 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
  //      s0  z0  s1  z1  s2  z2  s3  z3  s4  z4  s5  z5  s6  z6  s7  s8
  while ( i<14 ) {
    bitWrite(Ret,i,bitRead(EESec,y));i++;
    bitWrite(Ret,i,bitRead(Size,y));i++;
    y++;
  }
  bitWrite(Ret,i,bitRead(Size,y));y++;i++;    // i=14, y=7 Sets bit 14 to Size-bit 7[8]
  bitWrite(Ret,i,bitRead(Size,y));            // i=15, y=8 Sets bit 15 to Size-bit 8[9]
  
  Bytes[PKB_SECH] = highByte(Ret);     //3
  Bytes[PKB_SECL] = lowByte(Ret);      //4
  return Ret;
}
//-----------------------------------------------------------------------------------------------------
void TxPacket::AddTodeConfig(int _EEAddress) {
  DBENTERAL(("TxPacket::AddTodeConfig(EEAddress): "),(_EEAddress))
  
  int iDev = 0; int iTode = 0; byte DevByte = 255;
  for ( int EAdr = _EEAddress+2; EAdr<_EEAddress+AEB_TODEALLOC ; EAdr++ ) { // +3 skip RF Address

    // TODEHEAD ( Version + TodeName10 = 11-Bytes )
    if ( iTode<AEB_TODEHEAD-2 ) {
      iTode++; 
      DBINFOAA(("TxPacket::AddTodeConfig TODEHEAD EAdr,iTode: "),(EAdr),(iTode))
      TxByte( EEPROM.read(EAdr) );   // (1-11)
    
    // ADD DEVICES
    } else {
      DevByte = EEPROM.read(EAdr);
      if ( iDev>=AEB_DEVALLOC ) { DBINFOAL(("TxPacket::AddTodeConfig Device@EEPROM"),(EAdr)) iDev=0; }
      if ( iDev<=1 && DevByte==0xFF ) { DBINFOL(("TxPacket::AddTodeConfig End-of-devices")) break; }
      iDev++; DBINFOAA(("TxPacket::AddTodeConfig DEVICE EAdr,iDev: "),(EAdr),(iDev))
      TxByte( EEPROM.read(EAdr) );
    }
  }
  DBINFOAL(("TxPacket::AddTodeConfig Size = "),(Size))
}
//-----------------------------------------------------------------------------------------------------
void TxPacket::AddValue(byte _RFID, int _Value) {
  TxByte( _RFID ); TxByte( highByte(_Value) ); TxByte( lowByte(_Value) );
}
//_____________________________________________________________________________________________________________________
#endif
