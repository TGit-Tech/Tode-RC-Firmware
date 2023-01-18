/******************************************************************************************************************//**
 * @file    LHdw.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _LHDW_CPP
#define _LHDW_CPP

#include "LHdw.h"
#include "lib/EEMap.h"

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
int freeMem() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
//#####################################################################################################################
MenuTodeName::MenuTodeName(int _EENameAddress, byte _ID):
MenuName(_EENameAddress, true) { 
DBINITAAL(("MenuTodeName::MenuTodeName(EENameAddress,ID)"),(_EENameAddress,HEX),(_ID)) 
  ID=_ID; 
}
// ABCDEFG    
// 0123456
//  ABCDEFG
// 6=5
//-----------------------------------------------------------------------------------------------------
const char* MenuTodeName::Name() {                                        
DBENTERL(("MenuTodeName::Name[GET]"))
  if ( EENameAddress >= 0 ) { 
    DBINFOAL(("MenuTodeName::Name[GET] EEPROM.get(?EENameAddress?, FName)"),(EENameAddress,HEX))
    for (int i=0;i<EEMAXNAMESIZE;i++) { FName[i] = ' '; }       // Clean get array 'EEFName'
    EEPROM.get(EENameAddress, FName);                           // Get characters
    for (int i=EEMAXNAMESIZE;i>0;i--) { FName[i]=FName[i-1]; }  // Shift characters right 1
    if (ID==0) FName[0]='0';
    if (ID==1) FName[0]='1';
    if (ID==2) FName[0]='2';
    if (ID==3) FName[0]='3';
    if (ID==4) FName[0]='4';
    if (ID==5) FName[0]='5';
    if (ID==6) FName[0]='6';
    if (ID==7) FName[0]='7';
    if (ID==8) FName[0]='8';
    if (ID==9) FName[0]='9';
    FName[EEMAXNAMESIZE]='\0';                               // Add Terminator in +1 FName
    return FName;
  }
  DBERRORAL(("CName==0 and ?EENameAddress?<0 at : "),(EENameAddress)) 
  return "?Name?";
}
//#####################################################################################################################
Tode::Tode(byte _TodeIndex): MenuList() {                       DBINITAL(("Tode::Tode"), (_TodeIndex))
  
  if ( _TodeIndex>9 ) { DBERRORAL(("Tode::Tode INVALID INDEX must be (0-9)!"),(_TodeIndex)) }
  else {
    TodeIndex = _TodeIndex;
    bIsLocal = (_TodeIndex == 0);
    if ( bIsLocal ) Hardware = new HdwSelect(F("IO HDW"));           // Create a hardware select

    //MenuName(int _EENameAddress, bool _NameSettable = false);         
    TodeName = new MenuTodeName( EEAddress()+EMO_TODENAME, _TodeIndex);     // EEPROM Name Constructor    
    
  }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char* Tode::Title() { return TodeName->Name(); } 
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
    int devEEStart = this->EEAddress()+AEB_TODEHEAD+(devIdx*AEB_DEVALLOC);
    devType = EEPROM.read(devEEStart);
    if ( devType==BNONE ) { 
      DBINFOA(("Tode::EELoadDevices() - NO Device(devIdx)"),(devIdx))
      DBINFOAAL((" - EEPROM.read(EAddr)=(devType)"),(this->EEAddress()+AEB_TODEHEAD+(devIdx*AEB_DEVALLOC)),(devType,HEX))
    }
    else {
      byte devRFID = EEPROM.read(devEEStart+1);
      DBINFOAAL(("Tode::EELoadDevices() *** Free Memory *** Pre-(devRFID) = "),(devRFID),(freeMem())) 
      if ( devRFID > 29 ) { DBERRORL(("Tode::EELoadDevices() devRFID > 29")) continue; }
      this->AddDevice(devType, devRFID);
      DBINFOAAL(("Tode::EELoadDevices() *** Free Memory *** Post-(devRFID) = "),(devRFID),(freeMem())) 
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
MenuItem* Tode::NewDevice(byte _DTKey) {            
DBENTERAL(("Tode::NewDevice(_DTKey)"),(_DTKey,HEX))

  if ( 0x7F < _DTKey || _DTKey < DT_MINBOUNDARY ) { DBERRORL(("Tode::NewDevice. DTKey OUT OF BOUNDS")) return 0; }
  byte _RFID = 0; 
  while (_RFID<30) { if (Devices[_RFID]==0) {break;} else {_RFID++;} }      // Find Next Avaialbe RFID
   
  if (_RFID>29) { DBERRORL(("Tode::NewDevice OUT OF MEMORY")) return 0; }
  int EA = EEAddress() + AEB_TODEHEAD + (_RFID*AEB_DEVALLOC);
  
  DBINFOAAL(("Tode::NewDevice at _RFID,EEAddress : "),(_RFID),(EA))
  EEPROM.update(EA,_DTKey);EEPROM.update(EA+1,_RFID);EEPROM.put(EA+2,"?NAME?");
  Version(Version()+1);
  
  return AddDevice(_DTKey, _RFID);
  
}
//-----------------------------------------------------------------------------------------------------
MenuItem* Tode::AddDevice(byte _DTKey, byte _RFID) { 
DBENTERAAL(("Tode::AddDevice(DTKey,RFID) = "),(_DTKey,HEX),(_RFID))

  // Add Device (KEYS in iDev.h)
  //#define DT_RW_ONOFF     0x7E    ///< On/Off Switching Device
  //#define DT_RO_ONOFF     0x7D    ///< On/Off Monitoring Device
  //#define DT_RO_DIST      0x7A    ///< Distance Sensing Device
  //#define DT_RW_STSTP3W   0x79    ///< Start Stop 3-Wire
  //#define DT_RO_ANAINPUT  0x78    ///< Analog Device (Pressure, Temperature)
  //#define DT_DC_SETPOINT   0x77    ///< Dev Logic Control Boundary
  //#define DT_DC_MATH     0x76
  
  if ( 0x7F < _DTKey || _DTKey < DT_MINBOUNDARY ) { DBERRORL(("Tode::AddDevice. DTKey OUT OF BOUNDS")) return 0; }
  if ( 0>_RFID || _RFID>AEB_MAXDEVICES-1 ) return 0;          // Check RFID
  
  if ( _DTKey == DT_RW_ONOFF ) {
    //OnOff(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0)
    Devices[_RFID] = this->Add(new OnOff(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  }
  else if ( _DTKey == DT_RO_ONOFF ) { }
  else if ( _DTKey == DT_RO_DIST ) { 
    Devices[_RFID] = this->Add(new Dist(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];    
  } else if ( _DTKey == DT_RW_STSTP3W ) {
    Devices[_RFID] = this->Add(new STSTP3W(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  } else if ( _DTKey == DT_RO_ANAINPUT ) {
    Devices[_RFID] = this->Add(new AnaInput(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  } else if ( _DTKey == DT_DC_SETPOINT ) {
    //DCLimits::DCLimits(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware=0)
    Devices[_RFID] = this->Add(new DCSetPoint(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  } else if ( _DTKey == DT_DC_MATH ) {
    Devices[_RFID] = this->Add(new DCMath(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  } else if ( _DTKey == DT_RW_ANAOUTPUT ) {
    Devices[_RFID] = this->Add(new AnaOutput(TodeIndex,_RFID,Hardware));
    return Devices[_RFID];
  }
  return 0;
  
}
//-----------------------------------------------------------------------------------------------------
void Tode::DelDevice(MenuItem* _Item) {
DBENTERL(("Tode::DelDevice(MenuItem* _Item)"))
  _Item->EEClear();                                 // Erases EEPROM for Device
  Del(_Item);                                       // Menu.cpp Deletes from Link-List
  for ( int i = 0; i<AEB_MAXDEVICES; i++ ) {
    if ( Devices[i] != 0 ) { 
      if ( Devices[i] == _Item ) { 
        DBINFOAL(("Tode::DelDevice Devices[i] == _Item"),(i))
        Devices[i] = 0; 
      } 
    } 
  }
  Version(Version()+1);
}
//#####################################################################################################################
//-----------------------------------------------------------------------------------------------------
//_____________________________________________________________________________________________________________________
#endif
