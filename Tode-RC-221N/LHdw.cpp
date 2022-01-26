/******************************************************************************************************************//**
 * @file    LHdw.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _LHDW_CPP
#define _LHDW_CPP

#include "LHdw.h"
#include "lib/EEMap.h"
//#####################################################################################################################
Tode::Tode(byte _TodeIndex): MenuList() {                       DBINITAL(("Tode::Tode"), (_TodeIndex))
  
  if ( _TodeIndex>9 ) { DBERRORAL(("Tode::Tode INVALID INDEX must be (0-9)!"),(_TodeIndex)) }
  else {
    TodeIndex = _TodeIndex;
    bIsLocal = (_TodeIndex == 0);
    if ( bIsLocal ) Hardware = new HdwSelect("IO HDW");           // Create a hardware select

    //MenuName(int _EENameAddress, bool _NameSettable = false);         
    TodeName = new MenuName( EEAddress()+EMO_TODENAME, true);     // EEPROM Name Constructor    
    
  }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char* Tode::Title() {               return TodeName->Name(); }
bool Tode::IsLocal() {                    return bIsLocal; }
unsigned int Tode::RFAddr() {             unsigned int Ret=0; EEPROM.get(EEAddress(), Ret); return Ret; }
void Tode::RFAddr(unsigned int _RFAddr) { EEPROM.put(EEAddress(),_RFAddr); }
byte Tode::Version() {                    return EEPROM.read(EEAddress() + EMO_TODEVER); }
void Tode::Version(byte _Version) {       if(_Version==255)_Version=0;EEPROM.update(EEAddress() + EMO_TODEVER,_Version); }
int Tode::EEAddress() {                   return TodeIndex*AEB_TODEALLOC; }
//-----------------------------------------------------------------------------------------------------
void Tode::EELoadDevices() {
    // LOAD DEVICES from EEPROM
    // Each Devices is (12)bytes
    // Byte Order is [ TYPE-BYTE . RFID-Byte . (10)Bytes NAME ]
    // With 30-Devices per Tode = 360Bytes from EEPROM per Tode + Devices

  // Clean Old Devices
  DelAllItems();
  for ( int i=0; i<AEB_MAXDEVICES; i++) { Devices[i]=0; }

  // Load Devices from EEPROM
  byte devType = BNONE;
  for ( int devIdx=0; devIdx<30; devIdx++ ) {
    devType = EEPROM.read(this->EEAddress()+AEB_TODEHEAD+(devIdx*AEB_DEVALLOC));
    if ( devType==BNONE ) { 
      DBINFOA(("Tode::Tode NO Device at"),(devIdx))
      DBINFOAL(("EEPROM"),(this->EEAddress()+AEB_TODEHEAD+(devIdx*AEB_DEVALLOC)))
    }
    else { 
      //EEPROM.update(this->EEAddress()+AEB_TODEHEAD+(devIdx*AEB_DEVALLOC),0xFF); // Removes all Devices
      this->AddDevice(devType, devIdx); 
    }
  }  
}
//-----------------------------------------------------------------------------------------------------
void Tode::Update() {                                                     DBENTERL(("Tode::Update()"))

  if ( IsLocal() ) {
    DBINFOL(("Tode::Update IS LOCAL"))
    // Loop local devices
    
    for ( int i=0; i<AEB_MAXDEVICES; i++ ) {
      if ( Devices[i]!=0 ) {
        Devices[i]->DisplayValue();
      }
    }
    
  
  } else {
    if ( RF==0 ) { DBERRORL(("Tode::RFGetVals RF==0")) return; }
    RF->Send(new TxPacket( EEPROM.read(EMC_SECNET), PKT_GETVALS, RFAddr(), Version() ));
    //TxPacket(byte _SecNet, byte _Type, int _ToRF, byte _Ver=BNONE, byte _DevRFID=BNONE, int _Value = INONE) 
  }
}
//-----------------------------------------------------------------------------------------------------
MenuItem* Tode::NewDevice(byte _DTKey) {            DBENTERAL(("Tode::NewDevice. DTKey= "),(_DTKey,HEX))

  if ( 0x7F < _DTKey || _DTKey < 0x79 ) { DBERRORL(("Tode::NewDevice. DTKey OUT OF BOUNDS")) return 0; }
  byte _RFID = 0; 
  while (_RFID<30) { if (Devices[_RFID]==0) {break;} else {_RFID++;} }      // Find Next Avaialbe RFID
   
  if (_RFID>29) { DBERRORL(("Tode::NewDevice OUT OF MEMORY")) return 0; }
  int EA = EEAddress() + AEB_TODEHEAD + (_RFID*AEB_DEVALLOC);
  
  DBINFOAAL(("Tode::NewDevice at Index/EEAddress : "),(_RFID),(EA))
  EEPROM.update(EA,_DTKey); EEPROM.put(EA+1,"?NAME?");
  Version(Version()+1);
  
  return AddDevice(_DTKey, _RFID);
  
}
//-----------------------------------------------------------------------------------------------------
MenuItem* Tode::AddDevice(byte _DTKey, byte _RFID) { 
DBENTERAAL(("Tode::AddDevice(DTKey,RFID) = "),(_DTKey,HEX),(_RFID))

  // Add Device (KEYS in iDev.h)
  //#define DT_RW_ONOFF   0x7E    ///< On/Off Switching Device
  //#define DT_RO_ONOFF   0x7D    ///< On/Off Monitoring Device
  //#define DT_RO_PRESS   0x7C    ///< Pressure Device
  //#define DT_RO_TEMP    0x7B    ///< Temperature Device
  //#define DT_RO_DIST    0x7A    ///< Distance Sensing Device
  //#define DT_RW_STSTP3W 0x79    ///< Start Stop 3-Wire
  
  if ( 0x7F < _DTKey || _DTKey < 0x79 ) { DBERRORL(("Tode::AddDevice. DTKey OUT OF BOUNDS")) return 0; }
  if ( 0>_RFID || _RFID>AEB_MAXDEVICES-1 ) return 0;          // Check RFID
  
  if ( _DTKey == DT_RW_ONOFF ) {
    //OnOff(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0)
    Devices[_RFID] = this->Add(new OnOff(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  }
  else if ( _DTKey == DT_RO_ONOFF ) { }
  else if ( _DTKey == DT_RO_PRESS ) {
    Devices[_RFID] = this->Add(new Press(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  }
  else if ( _DTKey == DT_RO_TEMP ) { }
  else if ( _DTKey == DT_RO_DIST ) { 
    Devices[_RFID] = this->Add(new Dist(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];    
  } else if ( _DTKey == DT_RW_STSTP3W ) {
    Devices[_RFID] = this->Add(new STSTP3W(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  }
  return 0;
  
}
//#####################################################################################################################
TodeList::TodeList() { }
//-----------------------------------------------------------------------------------------------------
//_____________________________________________________________________________________________________________________
#endif
