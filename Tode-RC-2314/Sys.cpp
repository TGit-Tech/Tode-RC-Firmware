/******************************************************************************************************************//**
 * @file Sys.cpp
 * @brief Ref.h
 *********************************************************************************************************************/
#ifndef _MAIN_CPP
#define _MAIN_CPP
#include "Sys.h"

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
int freeMemory() {
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
Sys::Sys():
  Navigator() {                                                         DBINITL(("Sys::Sys"))

  DBINFOAL(("Sys::Sys() Begin *** Free Memory *** = "),(freeMemory()))
  //Serial.print("Sys::Sys() Begin Free Memory");Serial.println(freeMemory());
  
  // Preform EEPROM Clean for Debug
  //for (int i=2; i<EEPROM.length();i++) {if (i==EMC_SECNET) continue;EEPROM.update(i,0xFF);}
  DelTodesList = new MenuList(F("Del Tode"));
  DBINFOAL(("Sys::Sys() DelTodesList *** Free Memory *** = "),(freeMemory()))
  
  // Initialize RF Radio
  RF = new E32Radio(/*M0*/22, /*M1*/20, /*TX*/18, /*RX*/19, /*AUX*/21);
  RF->Mode(E32_SLEEPMODE);
  DBINFOAL(("Sys::Sys Radio Address: "), (RF->Address(),HEX))
  DBINFOAL(("Sys::Sys Radio Freq[mhz]: "), (RF->Frequency()))
  DBINFOAL(("Sys::Sys Radio Power[db]: "), (RF->TxPower()))
  RF->Mode(E32_NORMMODE);
  DBINFOAL(("Sys::Sys() RF *** Free Memory *** = "),(freeMemory()))
  
  // Check ThisTode EEPROM RF-Settings = Actual RF Radio
  ThisTode = this->NewTode(0);
  DBINFOAL(("EEPROM RF->Address() = "),((unsigned int)ThisTode->RFAddr(),HEX))
  if ( RF->Address() != ThisTode->RFAddr() ) { 
    DBINFOAL(("Sys::Sys UPDATING EEPROM - RF->RadioAddress() != ThisTode->RFAddr()"),(ThisTode->RFAddr(),HEX))
    ThisTode->RFAddr(RF->Address()); 
  } else { DBINFOL(("Sys::Sys EEPROM RF-Address Okay - RF->Address() == ThisTode->RFAddr()")) }
  DBINFOAL(("Sys::Sys() ThisTode *** Free Memory *** = "),(freeMemory()))
  
  // Build the Setup Menu (Must have Delete Tode List created before Adding)
  SetupMenu = this->Add(new MenuList(F("Setup")));
  BuildSetupMenu();
  DBINFOAL(("Sys::Sys() SetupMenu *** Free Memory *** = "),(freeMemory()))
  
  // Load Remote Todes from EEPROM
  for ( int i=1; i<AEB_MAXTODES; i++ ) {
    if ( EEPROM.read(i*AEB_TODEALLOC) != BNONE ) {  
      DBINFOAL(("Sys::Sys Tode@EEPROM: "),(i*AEB_TODEALLOC)) 
      this->NewTode(i);
      DBINFOAAL(("Sys::Sys() *** Free Memory *** @Tode<i>= "),(i),(freeMemory()))
      
    } else { DBINFOAL(("Sys::Sys NoTode@EEPROM: "),(i*AEB_TODEALLOC)) }
  }
  
  DBINFOAL(("Sys::Sys() End *** Free Memory *** = "),(freeMemory()))
  //Serial.print("Sys::Sys() End Free Memory");Serial.println(freeMemory());
}
//-----------------------------------------------------------------------------------------------------
MenuList* Sys::NewTode(byte _Index) {                     DBENTERAL(("Sys::NewTode(Index): "),(_Index))
  if ( _Index>AEB_MAXTODES ) { DBERRORAL(("Sys::NewTode(Index): Index>AEB_MAXTODES"),(_Index)) return 0; }
  if ( TodesPtr[_Index]!=0 ) { DBERRORAL(("Sys::NewTode(Index): TodesPtr[_Index]!=0"),(_Index)) return 0; }
  TodesPtr[_Index] = this->Add(new Tode(_Index));
  TodesPtr[_Index]->EELoadDevices();                                        // Have to call after Add for RF
  if(_Index!=0) { DelTodesList->Add(new MenuName(TodesPtr[_Index]->EEAddress(),+3,NAVDELTODE)); }
  return TodesPtr[_Index];
}
//-----------------------------------------------------------------------------------------------------
byte Sys::NewTode() {                                                     DBENTERL(("Sys::NewTode()"))
  // Find a Free Index
  byte i=1; while ( i<10 && TodesPtr[i] != 0 ) { i++; }
  if ( i==10 ) { DBERROR (("Sys::NewTode() OUT OF TODE MEM")) return BNONE; }
  NewTode(i);
  return i;
}
//---------------------------------------------------------------------------------------------------------------------
Tode* Sys::RFTode(unsigned int RFAddress, bool Create) {  DBENTERAAL(("Sys::RFTode(GET,CREATE): "),(RFAddress,HEX),(Create))
  
  int FreeIndex = 10;
  for ( int idx=0; idx<10; idx++ ) {                  // Search for Tode
    if ( TodesPtr[idx]!=0 ) { 
      if ( TodesPtr[idx]->RFAddr() == RFAddress ) return TodesPtr[idx]; 
    } else {
      if ( idx < FreeIndex ) FreeIndex = idx;
    }
  }
  // Nothing Found - then Create it at 'FreeIndex'
  if ( Create ) {
    if ( FreeIndex>=AEB_MAXTODES ) { DBERRORAL(("Sys::RFTode FreeIndex>=AEB_MAXTODES"),(FreeIndex)) return 0; }
    TodesPtr[FreeIndex] = this->Add(new Tode(FreeIndex));   // HERE Add doesn't work after a Delete!
    DBINFOAL(("Sys::RFTode *NEW* Tode(): "),(FreeIndex))
    TodesPtr[FreeIndex]->RFAddr(RFAddress);                 // Write the RFAddress
    return TodesPtr[FreeIndex];
  }
  return 0;
}
//-----------------------------------------------------------------------------------------------------
// WARNING! : In order to preserve internal object Items/List must be added in hierarcy order.
//-----------------------------------------------------------------------------------------------------
void Sys::BuildSetupMenu() {                                    DBENTERL(("Sys::BuildSetupMenu"))

  // 1. TodeName
  //MenuEEValue(const __FlashStringHelper* _CName, int _EEValAddress, byte _ValueType)
  DBINFOAL(("Sys::Sys() Sys::BuildSetupMenu() ENTRY *** Free Memory *** = "),(freeMemory()))
  ThisTode->TodeName = SetupMenu->Add(new MenuName(ThisTode->EEAddress()+EMO_TODENAME, true));  // 1. TodeName
  DBINFOAL(("Sys::Sys() Sys::BuildSetupMenu() SetupMenu->Add(new MenuName(ThisTode *** Free Memory *** = "),(freeMemory()))
  
  // 2. Radio . RadioSettings
  RadioSettings = new MenuList(F("Radio"));
  DBINFOAL(("Sys::Sys() new MenuList(F(Radio)) *** Free Memory *** = "),(freeMemory()))
  SetupMenu->Add(new MenuName(F("Radio"), RadioSettings));                                         // RADIO - Add supplies RF
  DBINFOAL(("Sys::Sys() SetupMenu->Add(new MenuName(F(Radio) *** Free Memory *** = "),(freeMemory()))
  SecNt = RadioSettings->Add(new MenuEEValue(F("SecNet"), EMC_SECNET, VTRW+VTBYTE+VTHEX+VTDIG));
  DBINFOAL(("Sys::Sys() RadioSettings->Add(new MenuEEValue(F(SecNet) *** Free Memory *** = "),(freeMemory()))
  RadioSettings->Add(new RadioAddress());
  DBINFOAL(("Sys::Sys() RadioSettings->Add(new RadioAddress()) *** Free Memory *** = "),(freeMemory()))
  RadioSettings->Add(new RadioFrequency());
  DBINFOAL(("Sys::Sys() RadioSettings->Add(new RadioFrequency()) *** Free Memory *** = "),(freeMemory()))
  RadioSettings->Add(new RadioTxPower());
  DBINFOAL(("Sys::Sys() RadioSettings->Add(new RadioTxPower()) *** Free Memory *** = "),(freeMemory()))
  RFPCConn = RadioSettings->Add(new RadioPCConn());
  DBINFOAL(("Sys::Sys() RadioSettings->Add(new RadioPCConn()) *** Free Memory *** = "),(freeMemory()))
  
  // 3. Add Device
  if (ThisTode==0) { DBERRORL(("Sys::BuildSetupMenu ThisTode==0")) }                            // 2.2 ThisTode Devices
  else {
    SetupMenu->Add(new MenuName(F("Add Device"), AddDeviceList = new MenuList(F("Add Device"))));
    DBINFOAL(("Sys::Sys() SetupMenu->Add(MenuName(Add Device) + MenuList(Add Device) *** Free Memory *** = "),(freeMemory())) 
    AddDeviceList->Add(new MenuName(F("OnOff"),     DT_RW_ONOFF));                                   //  2.1.1 OnOff
    DBINFOAL(("Sys::Sys() AddDeviceList->Add(new MenuName(F(OnOff) *** Free Memory *** = "),(freeMemory())) 
    AddDeviceList->Add(new MenuName(F("AnaInput"),  DT_RO_ANAINPUT));                                   //  2.1.2 Pressure
    AddDeviceList->Add(new MenuName(F("Distance"),  DT_RO_DIST));                                    //  2.1.4 Distance
    AddDeviceList->Add(new MenuName(F("STSTP3W"),   DT_RW_STSTP3W));                                 //  2.1.5 Distance
  }
  
  // 4. Todes MenuList
  SetupMenu->Add(new MenuName(F("Del Tode"), DelTodesList));
  
  // 5. AddATode Menu Item
  SetupMenu->Add(AddATode = new AddTode());

  // 6. IO-Hdw
  SetupMenu->Add(ThisTode->Hardware);                                                           // 2. IO-HDW (Created in ThisTode)

  // 7. Firmware
  SetupMenu->Add(new MemReset(F(FIRMWARE)));                                                       // FIRMWARE

  // Menu Item Settings
  SecNt->ValueRange(0x00, 0x7F);
}
//---------------------------------------------------------------------------------------------------------------------
byte Sys::Loop(byte _FinalKey) {

  RFLoop();

  // Loop local devices
  if ( ThisTode!=0 ) {
    for ( int i=0; i<AEB_MAXDEVICES; i++ ) {
      if ( ThisTode->Devices[i]!=0 ) ThisTode->Devices[i]->Loop();
    }
  }
  
  // [Specific] -----------------------------------------------
  if ( _FinalKey==0xFF ) return _FinalKey;
  DBINFOAL(("Sys::Loop SPECIFIC FinalKey = "),(_FinalKey,HEX))
  // Should we call Loop() in Every Tode? - Just local Tode
  
  
  // Add Device (KEYS in iDev.h)
  //#define DT_RW_ONOFF   0x7E    ///< On/Off Switching Device
  //#define DT_RO_ONOFF   0x7D    ///< On/Off Monitoring Device
  //#define DT_RO_PRESS   0x7C    ///< Pressure Device
  //#define DT_RO_TEMP    0x7B    ///< Temperature Device
  //#define DT_RO_DIST    0x7A    ///< Distance Sensing Device
  //#define DT_RW_STSTP3W 0x79    ///< Start Stop 3-Wire

  // Add a New Device
  if ( DT_MINBOUNDARY <= _FinalKey && _FinalKey <= 0x7E ) {
    int BeforeMEM = freeMemory();
    ThisTode->NewDevice(_FinalKey);

    int AfterMEM = freeMemory();
    DBINFOAL(("Object Size (kb)-> "), (BeforeMEM-AfterMEM))
    DBINFOAL(("Free Memory (kb)-> "), (AfterMEM))
  
    // return to Devices List
    this->Navigate( NAVKEYLEFT ); // Returns to Setup
    this->Navigate( NAVKEYLEFT ); // Returns to ThisTode (shows added item)
    return BNONE;
  }

  // Delete a Device
  if ( _FinalKey == NAVDELDEV ) { 
    //DBINFOAL(("NAVDELDEV"), (CurrList->PrevList->Title() ))
    if ( this->SublistEnterItem == 0 ) {DBINFOL(("Sys::Loop SublistEnterItem == 0")) return _FinalKey;}
    Device* ToDel = this->SublistEnterItem;
    this->Navigate( NAVKEYLEFT ); // Returns to Start Screen
    DBINFOAL(("Sys::Loop ToDel->Name"),(ToDel->Name()))
    ThisTode->DelDevice(ToDel);
    ThisTode->DispList(true);    
  }

  // Delete a Tode
  if ( _FinalKey == NAVDELTODE ) { 
    //DBINFOAL(("NAVDELTODE"), (CurrList->PrevList->Title() ))
    if ( DelTodesList->CurrItem == 0 ) {DBINFOL(("Sys::Loop DelTodesList->CurrItem == 0")) return _FinalKey;}
    MenuName* ToDel = DelTodesList->CurrItem;
    this->Navigate( NAVKEYLEFT ); // Returns to Start Screen
    DBINFOAL(("Sys::Loop ToDel->Name"),(ToDel->Name()))
    //ThisTode->DelDevice(ToDel);
    for ( int i=1; i<AEB_MAXTODES; i++ ) {
      if(TodesPtr[i]!=0) {
        if(TodesPtr[i]->EEAddress()==ToDel->EENameAddress-3) {        //Find the right Tode by it's EEAddress
          DBINFOAL(("Sys::Loop Tode to Delete @ TodePtr<i>"),(i))
          TodesPtr[i]->DelAllItems(true);                           //MenuList->DelAllItems(bool _EEClear=false) - Delete all Devices
          EEPROM.update(i*AEB_TODEALLOC,BNONE);                     // != BNONE Clear the EEPROM Entry for the Tode.  
          this->Del(TodesPtr[i]);                                   // Navigator->Del(Tode)
          TodesPtr[i] = 0;                                          // Set Pointer to 0
          DelTodesList->Del(ToDel);                                 // Remove Name from Delete Todes List
          return NAVKEYNONE;
        }
      }
    }
  }  
  
}
//---------------------------------------------------------------------------------------------------------------------
void Sys::RFLoop() {

  bool bNewTode = false;
  // Receive RF Data
  if ( RF==0 ) { DBERRORL(("Sys::RFLoop RF==0")) return 0; }
  if ( RFPCConn!=0 ) { if ( RFPCConn->Value()==1 ) { RFPCConn->Loop(); return;} }   // Loop for Radio->PC Connection 
  if ( !RF->PacketAvailable() ) return;                                             // Collect till Available
  
  // 1. Check Packet has an assigned Type
  if ( RF->Packet->Type() == PKT_NOTSET ) { 
    DBERRORL(("Sys::RFLoop PKT_NOTSET")) 
    delete(RF->Packet); RF->Packet = 0; return; 
  } else { DBINFOAL(("Sys::RFLoop RF->Packet->Type() = "),(RF->Packet->Type(),HEX)) }

  // 2. Obtain TargetTode
  Tode* TargetTode=0;                                                                   // Set Packets TargetTode
  if ( bitRead(RF->Packet->Type(),5) == 1 ) { TargetTode = ThisTode; }                  // GET/SET = ThisTode
  else { 
    TargetTode = RFTode(RF->Packet->FromRF(),false);                                    // Search for Tode in existance
    bNewTode = (TargetTode==0 && RF->Packet->Type()==PKT_GOTCONFIG);                    // Flag a NewTode Creation
    if (bNewTode) {TargetTode=RFTode(RF->Packet->FromRF(),true);}                       // Create a NewTode (,true)
  }                                                                                     // 
  if ( TargetTode==0 ) {                                                                // Check Target Tode
    DBERRORAL(("Sys::RFLoop TargetTode==0 PacketType:"),(RF->Packet->Type())) 
    delete(RF->Packet); RF->Packet = 0; return; 
  }
  DBINFOAL(("Sys::RFLoop TargetTode->TodeIndex:"),(TargetTode->TodeIndex))

  //------------------------- CONFIG ( No version control )----------------------------------------------------------
  if ( RF->Packet->Type() == PKT_GOTCONFIG ) {                                DBINFOL(("Sys::RFLoop PKT_GOTCONFIG"))
    RF->Packet->SaveTodeConfig( TargetTode->EEAddress() );                    // Save the Tode Configuration
    TargetTode->EELoadDevices();                                              // Reload Tode
    if (bNewTode) {DelTodesList->Add(new MenuName(TargetTode->EEAddress(),+3, NAVDELTODE));}       // Add new tode to delete list
    delete(RF->Packet); RF->Packet = 0; 
    if ( CurrList == TargetTode ) CurrList->DispList(true);                   // If Tode is On display refresh it.
    return;                                                                   // Exit
  }
  if ( RF->Packet->Type() == PKT_GETCONFIG ) {                                DBINFOL(("Sys::RFLoop PKT_GETCONFIG"))
    TxPacket Pkt(EEPROM.read(EMC_SECNET), PKT_GOTCONFIG, 
                 RF->Packet->FromRF(), TargetTode->Version() );               // Create TxPacket
    Pkt.AddTodeConfig( TargetTode->EEAddress() );                             // Load TxPacket with Configuration
    RF->Send(&Pkt);                                                           // Send Reply
    delete(RF->Packet); RF->Packet = 0; return;                               // Exit
  }

  //-------------------------------- VERSION MATCH ------------------------------------------------------------------
  if ( TargetTode->Version() != RF->Packet->Version() ) {                     // Check Version MATCH
    DBERRORAAL(("Sys::RFLoop Tode PACKET Version Mismatch(TodeVer,PktVer): "),
               (TargetTode->Version()), (RF->Packet->Version()))              // Show MISMATCH
    TxPacket Pkt(EEPROM.read(EMC_SECNET), PKT_GOTCONFIG, 
                 RF->Packet->FromRF(), TargetTode->Version() );               // MISMATCH Tx Update Config
    Pkt.AddTodeConfig( TargetTode->EEAddress() );                             // Tx Add Tode Config
    RF->Send(&Pkt);                                                           // Send Tode Config
    delete(RF->Packet); RF->Packet = 0; return;                               // Exit
  } else {
    DBINFOL(("Sys::RFLoop() TargetTode->Version() == RF->Packet->Version()"))
  }

  //-------------------------------- SINGLE DEVICE -------------------------------------------------------------------
  if ( RF->Packet->Type() == PKT_SETVAL || RF->Packet->Type() == PKT_GOTVAL ) {
    
    Device* TargetDev=0;
    int rfid = RF->Packet->RFID();
    if ( 0<=rfid && rfid<AEB_MAXDEVICES ) TargetDev = TargetTode->Devices[rfid];    // Get TargetDev
    if ( TargetDev==0 ) {                                                           // Check TargetDev
      DBERRORL(("Sys::RFLoop TargetDev==0"))
      delete(RF->Packet); RF->Packet = 0; return;                                   // ERROR Exit
    }

    if ( RF->Packet->Type() == PKT_SETVAL ) {                                       DBINFOL(("Sys::RFLoop PKT_SETVAL"))
      TargetDev->Value(RF->Packet->SetValue(), STSRFSET);                           // Set Device Value & Reply
      TxPacket Pkt(EEPROM.read(EMC_SECNET), PKT_GOTVAL, RF->Packet->FromRF(), 
                   TargetTode->Version(), TargetDev->RFID, TargetDev->Value() );    // GOTVAL the Set Value
      RF->Send(&Pkt);                                                               // Send the Reply
      
    } else if ( RF->Packet->Type() == PKT_GOTVAL ) {                                DBINFOL(("Sys::RFLoop PKT_GOTVAL"))
      TargetDev->Value( RF->Packet->Value( TargetDev->RFID ), STSRFGOT );           // Set GOT Value
    } 
    delete(RF->Packet); RF->Packet = 0; return;
  }
  
  //-------------------------------- MULTI DEVICE ---------------------------------------------------------------------
  if (RF->Packet->Type() == PKT_GETVALS ) {                                         DBINFOL(("Sys::RFLoop PKT_GETVALS"))
    TxPacket Pkt(EEPROM.read(EMC_SECNET), PKT_GOTVALS, RF->Packet->FromRF(), TargetTode->Version() );
    for ( int i=0; i<AEB_MAXDEVICES; i++ ) {                                        // Append every Device Value
      if ( TargetTode->Devices[i]!=0 ) {                                            // Iterate Devices[]
        if ( TargetTode->Devices[i]->RFID<AEB_MAXDEVICES ) {                        // Check Device RFID
          Pkt.AddValue(TargetTode->Devices[i]->RFID, TargetTode->Devices[i]->Value() ); }
      }
    }
    RF->Send(&Pkt);                                                                 // Send Packet
    
  } else if ( RF->Packet->Type() == PKT_GOTVALS ) {                                 DBINFOL(("Sys::RFLoop PKT_GOTVALS"))
    for ( int i=0; i<AEB_MAXDEVICES; i++ ) {                                        // Iterate Devices
      if ( TargetTode->Devices[i]!=0 ) {                                            // Assign Device Value
        TargetTode->Devices[i]->Value(RF->Packet->Value(TargetTode->Devices[i]->RFID),STSRFGOT);
        if ( CurrList==TargetTode ) TargetTode->Devices[i]->DisplayValue();         // Update Display
        DBINFOAL(("Sys::RFLoop PKT_GOTVALS RFID: "),(TargetTode->Devices[i]->RFID))
      }
    }
  }
  // Delete Packet after Processing
  delete(RF->Packet); RF->Packet = 0;
}
//_____________________________________________________________________________________________________________________
#endif
