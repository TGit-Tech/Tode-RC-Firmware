/******************************************************************************************************************//**
 * @file    iDev.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _IDEV_CPP
#define _IDEV_CPP

#include "iDev.h"

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
int Mem() {
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
OnOff::OnOff(byte _TodeIndex, byte _RFID, bool _NewDevice, bool _ReadOnly):
Device(_TodeIndex, _RFID)  {                
DBINITAAL(("OnOff::OnOff(TodeIndex,RFID,Hardware)"),(_TodeIndex),(_RFID))

    if (_ReadOnly) { DevType = DT_RO_ONOFF; } else { DevType = DT_RW_ONOFF; }
    ValueRange(0,1);
    SetNumberName(0, F("Off"));
    SetNumberName(1, F("On"));
    
    if (_NewDevice) {         // EE write default settings on _NewDevice
      int EA = (_TodeIndex*AEB_TODEALLOC)+AEB_TODEHEAD+(_RFID*AEB_DEVALLOC);
      if(_ReadOnly) {
        EEPROM.put(EA+2,"?ONOFFI?");                  // default Name
        EEPROM.update(EESettingAddrBO(1),PM_INLOW);   //defualt PinMode
      } else {
        EEPROM.put(EA+2,"?ONOFFO?");                  // default Name
        EEPROM.update(EESettingAddrBO(1),PM_OLOOFF);  //defualt PinMode
      }
      EEPROM.update(EESettingAddrBO(0),BVNOTSET);   //default PIN
    }

    if (IsLocal) ApplySettings();
}
//-----------------------------------------------------------------------------------------------------
void OnOff::BuildSubList() {
DBENTERL(("OnOff::BuildSubList()"))
  if (IsLocal) {
      bool bReadOnly = (DevType==DT_RO_ONOFF);
      if (bReadOnly) { SubList = new MenuList(F("OnOff-In"));  }
      else {           SubList = new MenuList(F("OnOff-Out")); }
      
      SubList->RF = this->RF;
      
      // Menu Settable Device Settings
      SubList->Add(new MenuNameSet(EENameAddress));
      SubList->Add(new PinSelect  (F("Pin")         ,RFID,0,PT_DIGITAL));
      SubList->Add(new DevPinMode (F("pinMode")     ,RFID,1,bReadOnly ));
      SubList->Add(new MenuName   (F("Del Device")  ,NAVDELDEV        ));
      SubList->DestEEClear=false;
  }
}
//-----------------------------------------------------------------------------------------------------
void OnOff::ApplySettings() {
DBENTERL(("OnOff::ApplySettings()"))

  // Get Device Settings
  EPin =      EEPROM.read(EESettingAddrBO(0));
  EPinMode =  EEPROM.read(EESettingAddrBO(1));
  
  // Apply & Validate - Pin & PinMode Settings
  SettingsValid=true;
  if (!ApplyPinSettings(EPin,EPinMode)) SettingsValid = false;
}
//-----------------------------------------------------------------------------------------------------
int OnOff::IOValue() {                                                 
DBENTERL(("OnOff::IOValue[GET]"))
  if ( !SettingsValid ) { DBERRORL(("OnOff::IOValue[GET] !SettingsValid")) return -1; }
  DBINFOAL(("OnOff::IOValue[GET] digitalRead(EPin)"),(EPin))
  return digitalRead(EPin);
}
//-----------------------------------------------------------------------------------------------------
void OnOff::IOValue(int _Value) {             
DBENTERAL(("OnOff::IOValue[SET]"),(_Value))
  if ( !SettingsValid ) { DBERRORL(("OnOff::IOValue[SET] !SettingsValid")) return -1; }
  DBINFOAAL(("OnOff::IOValue[SET] digitalWrite(EPin,Value)"),(EPin),(_Value))
  //PM_OLOOFF,PM_OHIOFF,PM_INHIGH,PM_INLOW
  if ( EPinMode==PM_INHIGH || EPinMode==PM_INLOW ) { 
    DBERRORL(("OnOff::IOValue[SET] EPinMode==PM_INHIGH || EPinMode==PM_INLOW"))
  } else {
    digitalWrite(EPin,_Value);  
  }
}
//-----------------------------------------------------------------------------------------------------
byte OnOff::Loop() { 
  if (!DoLoop()) return 0;
  if (!OnDisplay()) return 0;
  
  int GetIOValue = digitalRead(EPin);
  if ( GetIOValue == LastGetIOValue ) return 0;
  LastGetIOValue = GetIOValue;
  DisplayValue();
}
//#####################################################################################################################
Dist::Dist(byte _TodeIndex, byte _RFID, bool _NewDevice):
Device(_TodeIndex, _RFID) {                 
  DBINITAAL(("Dist::Dist(TodeIndex,RFID,Hardware)"),(_TodeIndex),(_RFID))
  DevType = DT_RO_DIST;

  if (_NewDevice) {         // EE write default settings on _NewDevice
    int EA = (_TodeIndex*AEB_TODEALLOC)+AEB_TODEHEAD+(_RFID*AEB_DEVALLOC);
    EEPROM.put(EA+2,"?DIST?");                    //default Name
    EEPROM.update(EESettingAddrBO(0),BVNOTSET);   //default Trig-PIN
    EEPROM.update(EESettingAddrBO(1),PM_OLOOFF);  //defualt TrigPMode
    EEPROM.update(EESettingAddrBO(2),BVNOTSET);   //default Echo-PIN
    EEPROM.update(EESettingAddrBO(3),PM_INLOW);   //default EchoPMode
    EEPROM.update(EESettingAddrBO(4),0);          //default Samples
  }

  if (IsLocal) { ApplySettings(); }
}
//-----------------------------------------------------------------------------------------------------
void Dist::BuildSubList() {
DBENTERL(("Dist::BuildSubList()"))
  if (IsLocal) {
      //if ( Hardware==0 ) { DBERRORL(("Dist::Dist IsLocal but Hardware==0")) return; } 
      SubList = new MenuList(F("Distance"));
      SubList->RF = this->RF;

      // Menu Settable Device Settings
      SubList->Add(new MenuNameSet(EENameAddress));
      SubList->Add(new PinSelect  (F("Trig"),      RFID,0,PT_DIGITAL));
      SubList->Add(new DevPinMode (F("TrigPMode"), RFID,1));
      SubList->Add(new PinSelect  (F("Echo"),      RFID,2,PT_DIGITAL));
      SubList->Add(new DevPinMode (F("EchoPMode"), RFID,3,true));
      SubList->Add(new ReadSamples(F("Samples"),   RFID,4));
      SubList->Add(new MenuName   (F("Del Device"), NAVDELDEV));
      SubList->DestEEClear=false;
  }
}
//-----------------------------------------------------------------------------------------------------
void Dist::ApplySettings() {
DBENTERL(("Dist::ApplySettings()"))

  // Get Device Settings
  trigPin =       EEPROM.read(EESettingAddrBO(0));
  trigPinmode =   EEPROM.read(EESettingAddrBO(1));
  echoPin =       EEPROM.read(EESettingAddrBO(2));
  echoPinmode =   EEPROM.read(EESettingAddrBO(3));
  ESamplesToGet = EEPROM.read(EESettingAddrBO(4));
  
  // Apply & Validate - Pin & PinMode Settings
  SettingsValid=true;
  if (!ApplyPinSettings(trigPin,trigPinmode)) SettingsValid = false;
  if (!ApplyPinSettings(echoPin,echoPinmode)) SettingsValid = false;
  
  // Setup Samples Buffer.
  if(Samples!=0){if(Samples->SamplesToGet!=ESamplesToGet){delete Samples;Samples=0;}}
  if(Samples==0&&(21>ESamplesToGet&&ESamplesToGet>0)) { 
    Samples=new SampleBuffer(ESamplesToGet);
    DBINFOAL(("AnaInput::ApplySettings() SampleBuffer(ESamplesToGet)"),(ESamplesToGet))
  }

}
//-----------------------------------------------------------------------------------------------------
void Dist::IOValue(int _Value) { DBENTERAL(("Dist::IOValue[SET]: "),(_Value)) /*Read-Only Device*/ }
//-----------------------------------------------------------------------------------------------------
int Dist::IOValue() {                                               
DBENTERL(("Dist::IOValue[GET]"))

  if ( !IsLocal || !SettingsValid ) return 0;
  int AveVal = 0;
  if ( Samples!=0 ) { AveVal = Samples->SampleAve(); }
  else              { AveVal = IOReadDistance(); }  
  
  return AveVal;
}
//-----------------------------------------------------------------------------------------------------
int Dist::IOReadDistance() {
  if ( !IsLocal || !SettingsValid ) return 0;
  
  // generate 10-microsecond pulse to TRIG pin
  digitalWrite(trigPin, LOW);delayMicroseconds(2);digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); digitalWrite(trigPin, LOW);
  
  float duration_us = pulseIn(echoPin, HIGH);     // measure duration of pulse from ECHO pin
  float distance_in = duration_us / 74 / 2;       // calculate the distance

  return (int)distance_in;
}
//-----------------------------------------------------------------------------------------------------
byte Dist::Loop() {
  if (!DoLoop()) return 0;
  if ( Samples==0 ) return 0;  // Need to loop expecially when on display.

  Samples->Sample( IOReadDistance() );

  // If on display check if AveValue has changed and Display if it has.
  if (!OnDisplay()) return 0;
  int Ave = IOValue();
  if ( Ave == LastGetIOValue ) return 0;
  LastGetIOValue = Ave;
  DisplayValue();
}
//#####################################################################################################################
AnaInput::AnaInput(byte _TodeIndex, byte _RFID, bool _NewDevice):
Device(_TodeIndex, _RFID)  {                
  DBINITAAL(("AnaInput::AnaInput(TodeIndex,RFID,Hardware)"),(_TodeIndex),(_RFID))
  DevType = DT_RO_ANAINPUT;

  if (_NewDevice) {         // EE write default settings on _NewDevice
    int i = 0; int EA = (_TodeIndex*AEB_TODEALLOC)+AEB_TODEHEAD+(_RFID*AEB_DEVALLOC);
    EEPROM.put(EA+2,"?ANAIN?");                   //default Name
    EEPROM.update(EESettingAddrBO(0),BVNOTSET);   //default PIN
    EEPROM.update(EESettingAddrBO(1),PM_INLOW);   //defualt PinMode
    i=0;EEPROM.put(EESettingAddrBO(2),i);         //default PreAdd
    i=1;EEPROM.put(EESettingAddrBO(4),i);         //default MultNum
    i=1;EEPROM.put(EESettingAddrBO(6),i);         //default MultDen
    i=0;EEPROM.put(EESettingAddrBO(8),i);         //default MultDen
    EEPROM.update(EESettingAddrBO(10),0);         //default Samples
  }

  if (IsLocal) { ApplySettings(); }
}
//-----------------------------------------------------------------------------------------------------
void AnaInput::BuildSubList() {
DBENTERL(("AnaInput::BuildSubList()"))
  if (IsLocal) { 
    SubList = new MenuList(F("AnaInput"));
    SubList->RF = this->RF;
    
    // Menu Settable Device Settings
    SubList->Add(new MenuNameSet(EENameAddress));
    SubList->Add(new PinSelect(   F("Pin"),         RFID,0,PT_ANALOG));
    SubList->Add(new DevPinMode(  F("pinMode"),     RFID,1, true    ));
    SubList->Add(new DevModValue( F("PreAdd"),      RFID,2          ));
    SubList->Add(new DevModValue( F("MultNum"),     RFID,4          ));
    SubList->Add(new DevModValue( F("MultDen"),     RFID,6, true    ));
    SubList->Add(new DevModValue( F("Add"),         RFID,8          ));
    SubList->Add(new ReadSamples( F("Samples"),     RFID,10         ));
    SubList->Add(new MenuName(    F("Del Device"),  NAVDELDEV       ));
    SubList->DestEEClear=false;
  }
}
//-----------------------------------------------------------------------------------------------------
void AnaInput::ApplySettings() {
DBENTERL(("AnaInput::ApplySettings()"))
  
  // Get Device Settings
  EPin =      EEPROM.read(EESettingAddrBO(0));
  EPinMode =  EEPROM.read(EESettingAddrBO(1));
              EEPROM.get( EESettingAddrBO(2),   PreAddValue);
              EEPROM.get( EESettingAddrBO(4),   MultNumValue);
              EEPROM.get( EESettingAddrBO(6),   MultDenValue);
              EEPROM.get( EESettingAddrBO(8),   AddValue);
              EEPROM.get( EESettingAddrBO(10),  ESamplesToGet);

  // Apply & Validate - Pin & PinMode Settings
  SettingsValid=true;
  if (!ApplyPinSettings(EPin,EPinMode)) SettingsValid = false;

  // Setup Samples Buffer.
  if(Samples!=0){if(Samples->SamplesToGet!=ESamplesToGet){delete Samples;Samples=0;}}
  if(Samples==0&&(21>ESamplesToGet&&ESamplesToGet>0)) { 
    Samples=new SampleBuffer(ESamplesToGet);
    DBINFOAL(("AnaInput::ApplySettings() SampleBuffer(ESamplesToGet)"),(ESamplesToGet))
  }

}
//-----------------------------------------------------------------------------------------------------
int AnaInput::IOReadAnaInput() {
  if ( !IsLocal || !SettingsValid ) return 0;
  return analogRead( EPin );
}
//-----------------------------------------------------------------------------------------------------
int AnaInput::IOValue() {                                                 
DBENTERL(("AnaInput::IOValue[GET]"))
  // PSI = (Value - 97) * 0.2137 is PreAdd = -97, MultNum/MultDen = (2137/10000)
  
  if ( !IsLocal || !SettingsValid ) return 0;
  int AveVal = 0;
  if ( Samples!=0 ) { AveVal = Samples->SampleAve(); }
  else              { AveVal = IOReadAnaInput(); }  
  
  // Calculate ModValue
  float PreAdd =  float(PreAddValue);
  float MultNum = float(MultNumValue);
  float MultDen = float(MultDenValue);
  float AddVal =  float(AddValue);
  
  if ( MultDen == 0 ) MultDen=10000;
  //DBINFOAAL(("AnaInput::IOValue[GET] (AveVal)+(PreAdd)"),(AveVal),(PreAdd))
  //DBINFOAAL(("AnaInput::IOValue[GET] (MultNum)/(MultDen)+(AddVal)"),(MultNum),(MultDen))
  //DBINFOAL(("AnaInput::IOValue[GET] (AddVal)"),(AddVal))
  //DBINFOAL(("AnaInput::IOValue[GET] = (Equals)"),( (AveVal + PreAdd) * (MultNum / MultDen) + AddVal ))
  
  return int( ((AveVal+PreAdd) * (MultNum/MultDen) ) + AddVal);
}
//-----------------------------------------------------------------------------------------------------
void AnaInput::IOValue(int _Value) { DBENTERAL(("AnaInput::IOValue[SET]: "),(_Value)) /*Read-Only Device*/ }
//-----------------------------------------------------------------------------------------------------
byte AnaInput::Loop() {
  if (!DoLoop()) return 0;
  if ( Samples==0 ) return 0;

  // Add Read Value to Values to Average
  Samples->Sample(IOReadAnaInput());
    
  // If on display check if AveValue has changed and Display if it has.
  if (NavSelected == SEL_SETNAME){ DBINFOL(("AnaInput::Loop() - NavSelected == SEL_SETNAME")) return 0; }
  
  if (!OnDisplay()) return 0;
  int Ave = IOValue();
  if ( Ave == LastGetIOValue ) return 0;
  LastGetIOValue = Ave;
  DisplayValue();

}
//#####################################################################################################################
STSTP3W::STSTP3W(byte _TodeIndex, byte _RFID, bool _NewDevice):
Device(_TodeIndex, _RFID) {               
DBINITAAL(("STSTP3W::STSTP3W(TodeIndex,RFID)"),(_TodeIndex),(_RFID)) 
  DevType = DT_RW_STSTP3W;
  ValueRange(0,1);
  SetNumberName(0, F("Off"));
  SetNumberName(1, F("On"));

  if (_NewDevice) {         // EE write default settings on _NewDevice
    int EA = (_TodeIndex*AEB_TODEALLOC)+AEB_TODEHEAD+(_RFID*AEB_DEVALLOC);
    EEPROM.put(EA+2,"?STSTP?");                   //default Name
    EEPROM.update(EESettingAddrBO(0),BVNOTSET);   //default StartPin
    EEPROM.update(EESettingAddrBO(1),PM_OLOOFF);  //defualt StartPMode
    EEPROM.update(EESettingAddrBO(2),BVNOTSET);   //default StopPin
    EEPROM.update(EESettingAddrBO(3),PM_OLOOFF);  //default StopPMode
    EEPROM.update(EESettingAddrBO(4),BVNOTSET);   //default StatusPin
    EEPROM.update(EESettingAddrBO(5),PM_INHIGH);  //default StatPMode
  }

  if (IsLocal) { ApplySettings(); }
}
//-----------------------------------------------------------------------------------------------------
void STSTP3W::BuildSubList() {
DBENTERL(("STSTP3W::BuildSubList()"))
  if (IsLocal) {
    SubList = new MenuList(F("STSTP3W"));
    SubList->RF = this->RF;
    
    // Menu Settable Device Settings
    SubList->Add(new MenuNameSet(EENameAddress));
    SubList->Add(new PinSelect  (F("StartPin"),   RFID,0,PT_DIGITAL ));
    SubList->Add(new DevPinMode (F("StartPMode"), RFID,1            ));
    SubList->Add(new PinSelect  (F("StopPin"),    RFID,2,PT_DIGITAL ));
    SubList->Add(new DevPinMode (F("StopPMode"),  RFID,3            ));
    SubList->Add(new PinSelect  (F("Status"),     RFID,4  ));
    SubList->Add(new DevPinMode (F("StatPMode"),  RFID,5,PT_DIGITAL ));
    SubList->Add(new MenuName   (F("Del Device"), NAVDELDEV         ));
    SubList->DestEEClear=false;
  }
}
//-----------------------------------------------------------------------------------------------------
void STSTP3W::ApplySettings() {
DBENTERL(("STSTP3W::ApplySettings()"))
  
  // Get Device Settings
  startPin =      EEPROM.read(EESettingAddrBO(0));
  startPinMode =  EEPROM.read(EESettingAddrBO(1));
  stopPin =       EEPROM.read(EESettingAddrBO(2));
  stopPinMode =   EEPROM.read(EESettingAddrBO(3));
  statusPin =     EEPROM.read(EESettingAddrBO(4));
  statusPinMode = EEPROM.read(EESettingAddrBO(5));

  // Apply & Validate - Pin & PinMode Settings
  SettingsValid=true;
  if (!ApplyPinSettings(startPin,startPinMode)) SettingsValid = false;
  if (!ApplyPinSettings(stopPin,stopPinMode)) SettingsValid = false;
  ApplyPinSettings(statusPin,statusPinMode);

}
//-----------------------------------------------------------------------------------------------------
void STSTP3W::IOValue(int _Value) {                        
DBENTERAL(("STSTP3W::IOValue[SET]: "),(_Value))

  if ( !SettingsValid ) { DBERRORL(("STSTP3W::IOValue[SET] !SettingsValid")) return -1; }

  if (_Value==0) {
    // Activate the STOP (OFF) Relay for 3-Seconds
    if ( stopPinMode == PM_OHIOFF ) { 
      DBINFOAL(("STSTP3W::IOValue[SET]STOP. PM_OHIOFF. digitalWrite(stopPin,0)"), (stopPin))
      digitalWrite(stopPin,0); 
    } else {
      DBINFOAL(("STSTP3W::IOValue[SET]STOP. digitalWrite(stopPin,1)"), (stopPin)) 
      digitalWrite(stopPin,1);
    }
    LastStopMS = millis();
    
  } else if (_Value==1) {
    // Activate the START (ON) Relay for 3-Seconds
    if ( startPinMode == PM_OHIOFF ) {
      DBINFOAL(("STSTP3W::IOValue[SET]START. PM_OHIOFF. digitalWrite(startPin,0)"),(startPin))
      digitalWrite(startPin,0);
    } else {
      DBINFOAL(("STSTP3W::IOValue[SET]START. digitalWrite(startPin,1)"),(startPin))
      digitalWrite(startPin,1);      
    }
    LastStartMS = millis();
    
  } else {
    DBERRORL(("STSTP3W::IOValue[SET]. Value is NOT 1/0!"))
  }
}
//-----------------------------------------------------------------------------------------------------
int STSTP3W::IOValue() {                                               
DBENTERL(("STSTP3W::IOValue[GET]"))
  if ( 70 < statusPin || statusPin < 0 ) { DBINFOL(("STSTP3W::IOValue[GET] 70<statusPin<0")) return -1; }
  DBINFOAL( ("STSTP3W::IOValue[GET] digitalRead(statusPin)"),(statusPin) )
  return digitalRead(statusPin);
}
//-----------------------------------------------------------------------------------------------------
byte STSTP3W::Loop() {
  if (!DoLoop()) return 0;
  if ( LastStopMS==0 && LastStartMS==0 ) return 0;

  if ( LastStopMS!=0 ) {
    if ( !SettingsValid ) { LastStartMS=0; return -1; }
    if ( (millis() - LastStopMS)>=3000 ) {
      if ( stopPinMode == PM_OHIOFF ) { 
        DBINFOAL(("STSTP3W::Loop. Momentary Release. digitalWrite(stopPin,1)"), (stopPin))
        digitalWrite(stopPin,1); 
      } else {
        DBINFOAL(("STSTP3W::Loop. Momentary Release. digitalWrite(stopPin,0)"), (stopPin)) 
        digitalWrite(stopPin,0); 
      }
      LastStopMS=0; return 0;
    }
  }

  if ( LastStartMS!=0 ) {
    if ( !SettingsValid ) { LastStartMS=0; return -1; }
    if ( (millis() - LastStartMS)>=3000 ) {
      if ( startPinMode == PM_OHIOFF ) { 
        DBINFOAL(("STSTP3W::Loop. Momentary Release. digitalWrite(startPin,1)"), (startPin))
        digitalWrite(startPin,1); 
      } else {
        DBINFOAL(("STSTP3W::Loop. Momentary Release. digitalWrite(startPin,0)"), (startPin)) 
        digitalWrite(startPin,0); 
      }
      LastStartMS=0; return 0; 
    }
  }
}
//#####################################################################################################################
AnaOutput::AnaOutput(byte _TodeIndex, byte _RFID, bool _NewDevice):
Device(_TodeIndex, _RFID, VTRW+VTDIG)  {                
DBINITAAL(("AnaOutput::AnaOutput(TodeIndex,RFID)"),(_TodeIndex),(_RFID))

  DevType = DT_RW_ANAOUTPUT;
  ValueRange(0,255);

  if (_NewDevice) {         // EE write default settings on _NewDevice
    int EA = (_TodeIndex*AEB_TODEALLOC)+AEB_TODEHEAD+(_RFID*AEB_DEVALLOC);
    EEPROM.put(EA+2,"?ANAOUT?");                  //default Name
    EEPROM.update(EESettingAddrBO(0),BVNOTSET);   //default Pin
  }

  if (IsLocal) { ApplySettings(); }
}
//-----------------------------------------------------------------------------------------------------
void AnaOutput::BuildSubList() {
DBENTERL(("AnaOutput::BuildSubList()"))
  if (IsLocal) {
    SubList = new MenuList(F("AnaOutput"));
    SubList->Add(new MenuNameSet(EENameAddress));
    SubList->Add(new PinSelect(F("Pin"),        RFID,0,PT_PWM ));
    SubList->Add( new MenuName(F("Del Device"), NAVDELDEV     ));
    SubList->DestEEClear=false;
  }
}
//-----------------------------------------------------------------------------------------------------
void AnaOutput::ApplySettings() {
DBENTERL(("AnaOutput::ApplySettings()"))
  
  // Get Device Settings
  EPin =      EEPROM.read(EESettingAddrBO(0));

  // Apply & Validate - Pin & PinMode Settings
  SettingsValid=true;
  if (!ApplyPinSettings(EPin,PM_OLOOFF)) SettingsValid = false;
}
//-----------------------------------------------------------------------------------------------------
int AnaOutput::IOValue() {                                                 
DBENTERL(("AnaOutput::IOValue[GET]"))
  if ( !SettingsValid ) { DBERRORL(("AnaOutput::IOValue[GET] !SettingsValid")) return -1; }
  DBINFOAL(("AnaOutput::IOValue[GET] LastAnalogWr(EPin)"),(EPin))
  return LastAnalogWriteValue;
}
//-----------------------------------------------------------------------------------------------------
void AnaOutput::IOValue(int _Value) {             
DBENTERAL(("AnaOutput::IOValue[SET]"),(_Value))
  if ( !SettingsValid ) { DBERRORL(("AnaOutput::IOValue[SET] !SettingsValid")) return -1; }
  DBINFOAAL(("AnaOutput::IOValue[SET] analogWrite(EPin,Value)"),(EPin),(_Value))
  if ( _Value < VMin ) _Value = VMin;
  if ( _Value > VMax ) _Value = VMax;
  LastAnalogWriteValue = _Value;
  analogWrite(EPin,_Value);
}
//-----------------------------------------------------------------------------------------------------
byte AnaOutput::Loop() { 
  if (!DoLoop()) return 0;
  if (!OnDisplay()) return 0;

  int GetIOValue = IOValue();
  if ( GetIOValue == LastGetIOValue ) return 0;
  LastGetIOValue = GetIOValue;
  DisplayValue();
}
//_____________________________________________________________________________________________________________________
#endif
