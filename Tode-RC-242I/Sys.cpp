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
PublishValues::PublishValues(byte _PubTimes, unsigned long _OnIntervalMS) { 
  PubTimes = _PubTimes;           // 0=done.
  OnIntervalMS = _OnIntervalMS;
}
//-----------------------------------------------------------------------------------------------------
Tode* PublishValues::OnTode() { return pTode[index]; }
//-----------------------------------------------------------------------------------------------------
void PublishValues::PubTode(Tode* _pTode) {
  int Idx = -1; if (_pTode==0) return;
  for ( int i=0; i<5; i++ ) { 
    if ( pTode[i]==_pTode ) { PubCount[i]=PubTimes; return; }   // Reset PubCount if Already exists.
  } 
  for ( int i=0; i<5; i++ ) {
    if ( PubCount[i]==0 ) { Idx=i; break; }       // Free index found.
    if ( PubCount[i]<PubCount[Idx] ) { Idx=i; }   // else Use index of least PubCount.
  }
  LastResultMS[Idx]=millis();
  PubCount[Idx]=PubTimes;
  pTode[Idx]=_pTode;
}
//-----------------------------------------------------------------------------------------------------
bool PublishValues::ForLoopCount(int _index) {  // returns true when Pub Needed.
  if ( PubCount[index]<1 ) { LastResultMS[index]=0; PubCount[index]=0; pTode[index]=0; }
  if ( _index > 4 || _index < 0 ) { return false; } else { index = _index; }
  if ( pTode[index]==0 ) { LastResultMS[index]=0; PubCount[index]=0; }
  if ( PubCount[index]<1 ) { return false; }
  if ( millis() - LastResultMS[index] < OnIntervalMS ) return false;
  LastResultMS[index]=millis();
  PubCount[index] = PubCount[index] -1;                   // Mark as a Publish time.
  return true;
}
//#####################################################################################################################
USBSerialUse::USBSerialUse():
  MenuEEValue(F("USBSerial"), EMC_USBSERUSE, VTRW+VTBYTE) {
    ValueRange(0,3);
    SetNumberName(BNONE,          F("NONE"));
    SetNumberName(USBSERUSE_NONE, F("NONE"));
    SetNumberName(USBSERUSE_MQTT, F("MQTT"));
    SetNumberName(USBSERUSE_KBC,  F("KBC"));
    SetNumberName(USBSERUSE_RFPC, F("RFPC"));
}
//-----------------------------------------------------------------------------------------------------
int USBSerialUse::Value() {                                       
//DBENTERAL(("USBSerialUse::Value[GET] @EE"),(EEValAddress,HEX)) 
  if ( VType&VTBYTE ) { iValue = (int)EEPROM.read(EEValAddress); } 
  else { EEPROM.get(EEValAddress, iValue); }
  return iValue;
}
//-----------------------------------------------------------------------------------------------------
void USBSerialUse::Value(int _Value, byte _Status) {                   
DBENTERAAL(("USBSerialUse::Value[SET](?Value?,Status)@?EE?"),(_Value),(EEValAddress,HEX))
  if ( !(VType&VTRW)) { DBINFOL(("MenuEEValue::Value[SET] BLOCKED Read-Only")) return; }  // Block-Set on Read-Only 
  if ( VType&VTBYTE ) { EEPROM.update(EEValAddress, byte(_Value)); }                      // Write Byte-Value
  else { EEPROM.put(EEValAddress,_Value); }                                               // Write Integer-Value
  if ( _Value!=iValue ) resetFunc();                                                      // Value Change causes Reset
  iValue = _Value;                                                                        // Use iValue for Integers
}
//#####################################################################################################################
Sys::Sys():
  Navigator() {                                                         DBINITL(("Sys::Sys"))

  // UN-Comment below to Preform EEPROM Clean for disfunctional firmware
  //for (int i=2; i<EEPROM.length();i++) {if (i==EMC_SECNET) continue;EEPROM.update(i,0xFF);}
  
  DBINFOAL(("Sys::Sys() Begin *** Free Memory *** = "),(freeMemory()))

  DelTodesList = new MenuList(F("Del Tode"));
  DBINFOAL(("Sys::Sys() DelTodesList *** Free Memory *** = "),(freeMemory()))
  
  // Initialize RF Radio
  RF = new Ebyte(RFPINS);      // Checks Settings in Constructor
  DBINFOAL(("Sys::Sys() RF *** Free Memory *** = "),(freeMemory()))
  
  ThisTode = this->NewTode(0);
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
  USBSerialeqRFPC=(EEPROM.read(EMC_USBSERUSE)==USBSERUSE_RFPC); // Used to exit RFLoop()
}
//#####################################################################################################################
String Sys::mqttForm(String _string) {
  int bufsize = _string.length();
  byte Tbuf[bufsize]; _string.getBytes(Tbuf,bufsize);    // Load the string into a char buffer
  for ( int c=0; c<bufsize; c++ ) {                      // Clean up characters
    if ( Tbuf[c]<48 || (Tbuf[c]>57&&Tbuf[c]<65) || (Tbuf[c]>90&&Tbuf[c]<97) || Tbuf[c]>122 ) { _string.setCharAt(c,' '); }
    if ( Tbuf[c]>64&&Tbuf[c]<91 ) { _string.setCharAt(c,Tbuf[c]+32); }    // Lowercase all letters
  };
  _string.replace(" ","");                              // Remove whitespaces  
  return _string;
}
//-----------------------------------------------------------------------------------------------------
void Sys::PubHomieMQTT() {
  if ( mqtt==0 ) return;
  
  unsigned long pubdelay = 0;   // Delay between Publish Lines
  String SubLines = "";         // Create Subscription topics.
  String PubLines = "";         // Publish lines backwards so $nodes=CSV is 1st. |LineSeperated=message.
  String nodesCSV = "";         // Variable to store the $nodesCSV in. (Add custom-nodesCSV in initalizer)
  
  for ( int t=0; t<AEB_MAXTODES; t++ ) {
    if ( TodesPtr[t]==0 ) continue;            
    bool bHasDevices = false;                   // Only add Todes with devices.
    for( int d=0; d<AEB_MAXDEVICES; d++ ) {     // homieMQTT complains if 'node' has no 'properties'.
      if( TodesPtr[t]->Devices[d] != 0 ) { bHasDevices=true; break; }
    }
    if ( bHasDevices ) {
      String TName = TodesPtr[t]->TodeName->Name(); TName = mqttForm(TName);
      if ( nodesCSV.length()!=0 ) { nodesCSV.concat(","); }
      nodesCSV.concat(TName);                   // Add TodeName to nodesCSV.
      nodesCSV.concat(",ts"+TName);             // Add tsTodeName to nodesCSV./
      if ( SubLines.length()!=0 ) { SubLines.concat(","); }
      SubLines.concat(TName +"/"+ "get");         // Add TodeName/get to subscribes
      
        String propCSV = "";                      // Variable to store $propertiesCSV in.
        String tspropCSV = "";                    // Variable to store $propertiesCSV for ts-TName.
        for ( int d=0; d<AEB_MAXDEVICES; d++ ) { 
          if( TodesPtr[t]->Devices[d]==0 ) continue;
          String DName = TodesPtr[t]->Devices[d]->Name(); DName = mqttForm(DName);
          if ( propCSV.length()!=0 ) { propCSV.concat(","); }       
          if ( tspropCSV.length()!=0 ) { tspropCSV.concat(","); }
          propCSV.concat(DName);                                    // Add DevName to propCSV.
          tspropCSV.concat("ts"+DName);                             // Add tsDevName to propCSV.
          byte DType = TodesPtr[t]->Devices[d]->DevType;
          String sSettable="false"; String sDatatype="string"; String sFormat="";
          if (      DType == DT_RW_ONOFF ) {      sSettable=F("true");  sDatatype=F("enum");    sFormat=F("ON,OFF");}
          else if ( DType == DT_RO_ONOFF ) {      sSettable=F("false"); sDatatype=F("enum");    sFormat=F("ON,OFF");}
          else if ( DType == DT_RO_DIST ) {       sSettable=F("false"); sDatatype=F("integer"); sFormat=F("#");     }
          else if ( DType == DT_RW_STSTP3W ) {    sSettable=F("true");  sDatatype=F("enum");    sFormat=F("ON,OFF");}
          else if ( DType == DT_RO_ANAINPUT ) {   sSettable=F("false"); sDatatype=F("integer"); sFormat=F("#");     }
          else if ( DType == DT_RW_ANAOUTPUT ) {  sSettable=F("true");  sDatatype=F("integer"); sFormat=F("#");     }
          else if ( DType == DT_DC_SETPOINT ) {   sSettable=F("true");  sDatatype=F("integer"); sFormat=F("#");     }
          else if ( DType == DT_DC_MATH ) {       sSettable=F("false"); sDatatype=F("integer"); sFormat=F("#");     }          
          String Topic =        TName +"/"+ DName      + "/";       // Topic = [homie/tode/]TName/DName/
          String tsTopic = "ts"+TName +"/"+ "ts"+DName +"/";        // tsTopic = [homie/tode/]tsTName/tsDName/
          if ( PubLines.length()!=0 ) { PubLines.concat("|"); }
          PubLines.concat(tsTopic+F("$settable") +"="+ F("false") +"|"); // 7.ts-
          PubLines.concat(tsTopic+F("$datatype") +"="+ F("string")+"|"); // 6.ts-
          PubLines.concat(tsTopic+F("$name")     +"="+ "ts"+DName +"|"); // 5.ts-       
          PubLines.concat(  Topic+F("$settable") +"="+ sSettable  +"|"); // 4.
          PubLines.concat(  Topic+F("$format")   +"="+ sFormat    +"|"); // 3.
          PubLines.concat(  Topic+F("$datatype") +"="+ sDatatype  +"|"); // 2.
          PubLines.concat(  Topic+F("$name")     +"="+ DName);           // 1.
          if ( SubLines.length()!=0 ) { SubLines.concat(","); }
          SubLines.concat(Topic+"set,");
          SubLines.concat(Topic+"get,");
          SubLines.concat("ts"+TName +"/"+ "ts"+DName);
        } // End Device Iterate
      String NTopic= TName+"/"; String tsNTopic = "ts"+TName+"/"; // Node Topics
      if ( PubLines.length()!=0 ) { PubLines.concat("|"); }
      PubLines.concat(  NTopic+F("$properties="));PubLines.concat(propCSV + "|"); // 6. 'TName/$properties'
      PubLines.concat(  NTopic+F("$name="));PubLines.concat(TName + "|");         // 5. 'TName/$name'
      PubLines.concat(  NTopic+F("$type=string|"));                               // 4. 'TName/$type'
      PubLines.concat(tsNTopic+F("$properties="));PubLines.concat(tspropCSV +"|");// 3. 'tsTName/$properties'
      PubLines.concat(tsNTopic+F("$name="));PubLines.concat("ts"+TName +"|");     // 2. 'tsTName/$name'
      PubLines.concat(tsNTopic+F("$type=string"));                                // 1. 'tsTName/$type'

    } // Has Devices
  }   // End Tode Iterate
  if ( PubLines.length()!=0 ) { PubLines.concat("|"); }
  PubLines.concat(F("$nodes="));PubLines.concat(nodesCSV);
      
  // Publish data
  mqtt->publish(F("homie/tode/$state"),           F("lost"),          1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$state"),           F("disconnected"),  1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$state"),           F("sleeping"),      1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$state"),           F("init"),          1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$homie"),           F("4.0"),           1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$name"),            F("mqtt-tode"),     1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$extensions"),      F(""),              1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$fw/name"),         F("tode-rc"),       1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$fw/version"),      F("2327"),          1,true);delay(pubdelay);
  mqtt->publish(F("homie/tode/$implementation"),  F("mega2560"),      1,true);delay(pubdelay);
  
  // Publish /homie/tode/TName/DName data
  while ( PubLines.length()!= 0 ) {
    String Topic=F("homie/tode/");int lineAt = PubLines.lastIndexOf('|')+1; int eqAt = PubLines.lastIndexOf('=');
    if (eqAt<1) { mqtt->println(F("Bad PubLine no =")); break; }
    Topic.concat(PubLines.substring(lineAt,eqAt));              // lastline -1+1=0
    mqtt->publish(Topic, PubLines.substring(eqAt+1),1,true);delay(pubdelay);
    if (lineAt==0) break;PubLines.remove(lineAt-1);
  }

  // Send 'ready' signal
  mqtt->publish(F("homie/tode/$state"), F("ready"), 1,true);delay(pubdelay);
  mqtt->_lastReceivedLoopback = millis();     //Prevent loopback timeout to 'offline' status runaway.

  // Subscribe to Topics
  while ( SubLines.length() != 0 ) {
    int commaAt = SubLines.lastIndexOf(',');
    String subtopic = F("homie/tode/");
    if ( commaAt!=-1 ) { 
      subtopic.concat(SubLines.substring(commaAt+1));
      mqtt->subscribe(subtopic);
      SubLines.remove(commaAt);
    } else { /*lastline*/ 
      subtopic.concat(SubLines);
      mqtt->subscribe(subtopic); 
      SubLines = ""; 
    }
  }
}
//-----------------------------------------------------------------------------------------------------
void Sys::MQTTLoop() {
  if ( mqtt==0 ) return;
  mqtt->housekeeping();

  if(mqttOnline != mqtt->online()) {       //if Online state has changed
    if(mqtt->online()) {
      mqttOnline = true;
      PubHomieMQTT();
    } else { 
      mqttOnline = false; 
      mqtt->println(F("MQTT just went offline"));    
    }
  }

  //Check for incoming messages
  if(mqtt->messageWaiting()) {
    String mtopic = mqtt->topic();
    int backslashAt = mtopic.lastIndexOf('/');
    String operation = mtopic.substring(backslashAt+1);
    
    if ( operation.equals("set") || operation.equals("get") ) {
      mtopic.remove(backslashAt);                     // (del)  homie/tode/Tode/Device(/set)
      backslashAt = mtopic.lastIndexOf('/');          // (find) homie/tode/Tode(/)Device
      String sDev = mtopic.substring(backslashAt+1);  // (get)  homie/tode/Tode/(Device)
      mtopic.remove(backslashAt);                     // (del)  homie/tode/Tode(/Device)
      backslashAt = mtopic.lastIndexOf('/');          // (find) homie/tode(/)Tode
      String sTode = mtopic.substring(backslashAt+1); // (get)  homie/tode/(Tode)
      mtopic.remove(backslashAt);                     // (del)  homie/tode(/Tode)
      backslashAt = mtopic.lastIndexOf('/');          // (find) homie(/)tode
      if ( backslashAt < 0 ) sTode = sDev;            // Is a homie/tode/Tode/get message.

      // Find the Tode pointer
      Tode* pTode = 0;
      for ( int t=0; t<AEB_MAXTODES; t++ ) {
        if ( TodesPtr[t] == 0 ) continue;
        String TName = TodesPtr[t]->TodeName->Name(); TName = mqttForm(TName);
        if ( sTode==TName ) { pTode = TodesPtr[t]; break; }
      }
      if ( pTode == 0 ) { mqtt->println(F("Tode not found.")); mqtt->markMessageRead(); return; }
      if ( operation.equals("get") ) { 
        PubValues->PubTode(pTode); 
        mqtt->markMessageRead(); 
        return; 
      }
      
      // SET a Device.  
      Device* pDevice = 0;
      for ( int d=0; d<AEB_MAXDEVICES; d++ ) {
        if ( pTode->Devices[d] == 0 ) continue;
        String DName = pTode->Devices[d]->Name(); DName = mqttForm(DName);
        if ( sDev==DName ) { pDevice = pTode->Devices[d]; break; }
      }
      if ( pDevice == 0 ) { mqtt->markMessageRead(); mqtt->println(F("Device not found.")); return; }
      if ( mqtt->message().equalsIgnoreCase("ON") ) {  pDevice->Value(1,STSUSERSET); }
      else if ( mqtt->message().equalsIgnoreCase("OFF") ) { pDevice->Value(0,STSUSERSET); }
      else { pDevice->Value(mqtt->message().toInt(),STSUSERSET); }
      PubValues->PubTode(pTode);
    } // close set or get
    mqtt->markMessageRead();
  }

  //..... Send PubValues ...................................................
  for ( int i=0; i<5; i++) {
    if ( PubValues->ForLoopCount(i) ) {
      PubValues->OnTode()->Update();
      String TName = PubValues->OnTode()->Title(); TName = mqttForm(TName);
      for ( int d=0; d<AEB_MAXDEVICES; d++ ) {
        if ( PubValues->OnTode()->Devices[d] == 0 ) continue;
        String DName = PubValues->OnTode()->Devices[d]->Name(); DName = mqttForm(DName); 
        int iVal = PubValues->OnTode()->Devices[d]->Value(); 
        String sVal = String(iVal);
        byte DType = PubValues->OnTode()->Devices[d]->DevType;
        if ( DType == DT_RW_ONOFF || DType == DT_RO_ONOFF || DType == DT_RW_STSTP3W ) {
          if ( iVal==0 ) { sVal="OFF"; } else if ( iVal == 1 ) { sVal="ON"; } 
        }
        String Topic = F("homie/tode/"); Topic.concat(TName+"/"+DName);
        String tsTopic = F("homie/tode/"); tsTopic.concat("ts"+TName+"/"+"ts"+DName);
        mqtt->publish( Topic,   sVal);
        String Valtimestamped = sVal + "-" + String(millis(), HEX);
        mqtt->publish( tsTopic, Valtimestamped);
      }
    }
  }

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
void Sys::EnterSetupMenu() {                                    DBENTERL(("Sys::EnterSetupMenu"))
  DBINFOAL(("Sys::Sys() Sys::EnterSetupMenu() ENTRY *** Free Memory *** = "),(freeMemory()))

  SetupMenu = new MenuList(F("Setup"));
  SetupMenu->RF = this->RF;

  // 1. TodeName
  SetupMenu->Add(new MenuNameSet(ThisTode->TodeName->EENameAddress));

  // 2. Radio . RadioSettings
  if (this->RF->RadioID()==EM_E22_400T30D) {RadioSettings = new MenuList(F("E22400T30D"));}
  else if (this->RF->RadioID()==EM_E22_400T33D) {RadioSettings = new MenuList(F("E22400T33D"));}
  else if (this->RF->RadioID()==EM_E22_900T30D) {RadioSettings = new MenuList(F("E22900T30D"));}
  else if (this->RF->RadioID()==EM_E32_433T20D) {RadioSettings = new MenuList(F("E32433T20D"));}
  else if (this->RF->RadioID()==EM_E32_433T30D) {RadioSettings = new MenuList(F("E32433T30D"));}
  else if (this->RF->RadioID()==EM_E220_400T30D) {RadioSettings = new MenuList(F("E220400T30D"));}
  else {RadioSettings = new MenuList(F("Unknown"));}

  SetupMenu->Add(new MenuName(F("Radio"), RadioSettings));                                         // RADIO - Add supplies RF
  SecNt = RadioSettings->Add(new MenuEEValue(F("SecNet"), EMC_SECNET, VTRW+VTBYTE+VTHEX+VTDIG));
  RadioSettings->Add(new RadioAddress());
  if (this->RF->RadioID()==EM_E22_400T33D || this->RF->RadioID()==EM_E22_900T30D ) {
    RadioSettings->Add(new RadioNetID());
  }
  RadioSettings->Add(new RadioFrequency(this->RF->RadioID()));
  RadioSettings->Add(new RadioAirSpeed(this->RF->RadioID()));
  RadioSettings->Add(new RadioTxPower(this->RF->RadioID()));
  
  
  // 3. Add Device
  if (ThisTode==0) { DBERRORL(("Sys::BuildSetupMenu ThisTode==0")) }                            // 2.2 ThisTode Devices
  else {
    SetupMenu->Add(new MenuName(F("Add Device"), AddDeviceList = new MenuList(F("Add Device"))));
    AddDeviceList->Add(new MenuName(F("OnOff-Out"),   DT_RW_ONOFF));                                   //  2.1.1 OnOff 
    AddDeviceList->Add(new MenuName(F("OnOff-In"),    DT_RO_ONOFF));
    AddDeviceList->Add(new MenuName(F("AnaInput"),    DT_RO_ANAINPUT));                                   //  2.1.2 Pressure
    AddDeviceList->Add(new MenuName(F("AnaOutput"),   DT_RW_ANAOUTPUT));
    AddDeviceList->Add(new MenuName(F("Distance"),    DT_RO_DIST));                                    //  2.1.4 Distance
    AddDeviceList->Add(new MenuName(F("STSTP3W"),     DT_RW_STSTP3W));                                 //  2.1.5 Distance
    AddDeviceList->Add(new MenuName(F("SetPoint"),    DT_DC_SETPOINT));
    AddDeviceList->Add(new MenuName(F("Math"),        DT_DC_MATH));
    
  }
  
  // 4. Todes MenuList
  MenuName* DeleteTodesList = new MenuName(F("Del Tode"), DelTodesList);
  DeleteTodesList->bSaveSubList = 1;                                          // DelTodesList cannot be deleted or crashes
  SetupMenu->Add(DeleteTodesList);
  
  // 5. AddATode Menu Item
  SetupMenu->Add(AddATode = new AddTode());

  // 6. System
  SystemCommands = new MenuList(F("System"));
  SetupMenu->Add(new MenuName(F("System"), SystemCommands));
  SystemCommands->Add(new USBSerialUse());
  SystemCommands->Add(new MenuName(F("Factry RST"), NAVSYSFCTRYRST));
  SystemCommands->Add(new MenuName(F("Pub ADiscv"), NAVSYSPUBADISC));
  SystemCommands->Add(new MenuName(FIRMWARE,        NAVSYSSIMPLRST));

  // Menu Item Settings
  SecNt->ValueRange(0x00, 0x7F);

  MenuList* pList = CurrList;
  SetupMenu->PrevList = CurrList;
  CurrList = SetupMenu;
  if ( pList != 0 ) pList->OffDisplay();
  CurrList->DispList(true);
  bInSetupMenu = 1;
}
//---------------------------------------------------------------------------------------------------------------------
byte Sys::Loop(byte _FinalKey) {
  
  RFLoop();
  if ( mqttActive ) {
    if ( mqtt==0 ) { 
      mqtt = new serial2mqtt;
      mqtt->begin(Serial);
      mqtt->useJSONobjects();
      mqtt->debug(false);
      mqtt->loopbackTopic(F("src/tode/lb")); 
    }
    MQTTLoop();
  }

  // Loop SetupMenu Items
  if ( SetupMenu != 0 && AddATode != 0 ) AddATode->Loop();

  // Loop local devices and get device pointers for DCL drivers
  if ( ThisTode!=0 ) {
    for ( int i=0; i<AEB_MAXDEVICES; i++ ) {
      if ( ThisTode->Devices[i]!=0 ) {
        if ( ThisTode->Devices[i]->Loop() == LP_GETDEVICES ) {  // LOOP gets CALLED
          if ( ThisTode->Devices[i]->DLCDevs != 0 ) {
            DBINFOAL(("Sys::Loop() Getting DCL Device Pointers for Device[i]"),(i))
            byte InIdx = ThisTode->Devices[i]->DLCDevs->InDevIdx;
            byte In2Idx = ThisTode->Devices[i]->DLCDevs->In2DevIdx;
            byte OTodeIdx = ThisTode->Devices[i]->DLCDevs->OutTodeIdx;
            byte OutIdx = ThisTode->Devices[i]->DLCDevs->OutDevIdx;
            Tode* TheTode = 0;
            if ( InIdx<16 ) { ThisTode->Devices[i]->DLCDevs->InDev = ThisTode->Devices[InIdx]; }
            if ( In2Idx<16 ) { ThisTode->Devices[i]->DLCDevs->In2Dev = ThisTode->Devices[In2Idx]; }
            if ( OTodeIdx<10 && OutIdx<16 ) { 
              Tode* TheTode=TodesPtr[OTodeIdx];
              if ( TheTode!=0 ) { 
                ThisTode->Devices[i]->DLCDevs->OutTode = TheTode;
                ThisTode->Devices[i]->DLCDevs->OutDev = TheTode->Devices[OutIdx]; 
              }
            }
          }
        } // END ThisTode->Devices[i]->Loop() == LP_GETDEVICES
      }
    }
  }

  // Loop remote devices
  for ( int t=1; t<AEB_MAXTODES; t++ ) {
    if ( TodesPtr[t]!=0 ) { 
      for ( int d=0; d<AEB_MAXDEVICES; d++ ) {
        if ( TodesPtr[t]->Devices[d]!=0 ) { TodesPtr[t]->Devices[d]->Loop(); }
      }
    }
  }
  
  // [Specific] -----------------------------------------------
  if ( _FinalKey==0xFF ) return _FinalKey;
  DBINFOAL(("Sys::Loop SPECIFIC FinalKey = "),(_FinalKey,HEX))
  // Should we call Loop() in Every Tode? - Just local Tode

  // Device Types defined in iHdw.h  
  //#define DT_RW_ONOFF     0x7E    ///< On/Off Switching Device
  //#define DT_RO_ONOFF     0x7D    ///< On/Off Monitoring Device
  //#define DT_RO_DIST      0x7A    ///< Distance Sensing Device
  //#define DT_RW_STSTP3W   0x79    ///< Start Stop 3-Wire
  //#define DT_RO_ANAINPUT  0x78    ///< Analog Device (Pressure, Temperature)
  //#define DT_RW_ANAOUTPUT 0x77
  //#define DT_DC_SETPOINT  0x76    ///< Dev Logic Control Boundary
  //#define DT_DC_MATH      0x75
  //#define DT_MINBOUNDARY  0x75    ///< MINIMUM BOUNDARY CHECK FOR ADD DEVICE KEYS

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
 
    // Set any DCL Device Pointers to 0
    Tode* ATode = 0; Device* ADev = 0;
    for (byte t=0;t<AEB_MAXTODES;t++) {
      ATode=this->TodesPtr[t];
      if ( ATode!=0 ) {
        for (byte d=0;d<AEB_MAXDEVICES;d++) {
          ADev=ATode->Devices[d];
          if ( ADev!= 0 ) {
            if ( ADev->DLCDevs != 0 ) {
              if ( ADev->DLCDevs->InDev == ToDel ) ADev->DLCDevs->InDev=0;
              if ( ADev->DLCDevs->In2Dev == ToDel ) ADev->DLCDevs->In2Dev=0; 
              if ( ADev->DLCDevs->OutDev == ToDel ) ADev->DLCDevs->OutDev=0;
            }
          }
        }  
      }
    }
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

          // Set any DCL Device Pointers to 0
          Tode* ATode = 0; Device* ADev = 0;
          for (byte t=0;t<AEB_MAXTODES;t++) {
            ATode=this->TodesPtr[t];
            if ( ATode!=0 ) {
              for (byte d=0;d<AEB_MAXDEVICES;d++) {
                ADev=ATode->Devices[d];
                if ( ADev!= 0 ) {
                  if ( ADev->DLCDevs != 0 ) { if ( ADev->DLCDevs->OutTodeIdx == i ) ADev->DLCDevs->OutDev=0; }
                }
              }  
            }
          }

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

  // System Commands
  if ( _FinalKey == NAVSYSSIMPLRST ) { resetFunc(); }
  if ( _FinalKey == NAVSYSFCTRYRST ) {
      // Clear all EEPROM and Reset Arduino
    for (int i=2; i<EEPROM.length();i++) {    //Skip resetting RFAddr at [0,1]
      if (i==EMC_SECNET) continue;            //Skip resetting SecNet
      if (i==EMC_USBSERUSE) continue;         //Skip USB-use for testing
      EEPROM.update(i,0xFF); 
    }
    resetFunc();
  }
  if ( _FinalKey == NAVSYSPUBADISC ) { PubHomieMQTT(); }
}
//---------------------------------------------------------------------------------------------------------------------
void Sys::RFLoop() {

  bool bNewTode = false;
  // Receive RF Data
  if ( RF==0 ) { DBERRORL(("Sys::RFLoop RF==0")) return 0; }
  if (USBSerialeqRFPC) return;
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
    if ( SetupMenu != 0 && AddATode != 0 ) AddATode->Status(STSRFGOT);
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
