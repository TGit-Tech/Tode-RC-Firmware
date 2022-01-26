/******************************************************************************************************************//**
 * @file    iDev.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _IDEV_CPP
#define _IDEV_CPP

#include "iDev.h"
//#####################################################################################################################
OnOff::OnOff(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware):
  Device(_TodeIndex, _RFID)  {                DBINITAAL(("OnOff::OnOff(TodeIndex,RFID,Hardware)"),(_TodeIndex),(_RFID))
    ValueRange(0,1);
    SetNumberName(0, "Off");
    SetNumberName(1, "On");
    if (IsLocal) {
      if ( _Hardware==0 ) { DBERRORL(("OnOff::OnOff IsLocal but Hardware==0")) return; } 
      SubList = new MenuList("OnOff Setup");
      IOPin = SubList->Add(new PinSelect("Pin",_RFID,0,_Hardware));
    }
}
//-----------------------------------------------------------------------------------------------------
int OnOff::IOValue() {                                                 DBENTERL(("OnOff::IOValue(GET)"))
  if ( IOPin == 0 ) { DBERRORL(("OnOff::Value GET - IOPin==0")) return -1; }
  if ( IOPin->Value() < 0 || IOPin->Value() > 70 ) { DBERRORL(("OnOff::Value GET - IOPin OUT OF BOUNDS")) return -1; }
  DBINFOAL(("OnOff::IOValue(GET) digitalRead"),(IOPin->Value()))
  return digitalRead(IOPin->Value());
}
//-----------------------------------------------------------------------------------------------------
void OnOff::IOValue(int _Value) {                         DBENTERAL(("OnOff::IOValue(SET): "),(_Value))

  if ( IOPin == 0 ) { DBERRORL(("OnOff::Value SET - IOPin==0")) return -1; }
  if ( IOPin->Value() < 0 || IOPin->Value() > 70 ) { DBERRORL(("OnOff::Value SET - IOPin OUT OF BOUNDS")) return -1; }
  DBINFOAAL(("OnOff::IOValue(SET) digitalWrite"),(IOPin->Value()),(_Value))
  digitalWrite(IOPin->Value(),_Value);
}
//-----------------------------------------------------------------------------------------------------
byte OnOff::Loop() {  
  if ((millis() - LocalRefreshMS) < 500 ) return 0;             // Only read 1/2-Seconds
  LocalRefreshMS = millis();
  if (!IsLocal) return 0;
  if (!OnDisplay()) return 0;
  
  // Check for update
  int GetIOValue = digitalRead(IOPin->Value());
  if ( GetIOValue == LastGetIOValue ) return 0;
  LastGetIOValue = GetIOValue;
  DisplayValue();
}
//#####################################################################################################################
Press::Press(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware):
  Device(_TodeIndex, _RFID)  {                
    DBINITAAL(("Press::Press(TodeIndex,RFID,Hardware)"),(_TodeIndex),(_RFID))
    if (IsLocal) {
      if ( _Hardware==0 ) { DBERRORL(("Press::Press IsLocal but Hardware==0")) return; } 
      SubList = new MenuList("Press Setup");
      IOPin = SubList->Add(new PinSelect("Pin",_RFID,0,_Hardware));
    }
}
//-----------------------------------------------------------------------------------------------------
int Press::IOValue() {                                                 DBENTERL(("Press::IOValue(GET)"))
  // PSI = (Value - 97) * 0.2137  
  
  int AveSum=0; int AveVal = 0;
  if ( IOPin == 0 ) { DBERRORL(("Press::IOValue GET - IOPin==0")) return -1; }
  if ( IOPin->Value() < 0 || IOPin->Value() > 70 ) { DBERRORL(("Press::IOValue GET - IOPin OUT OF BOUNDS")) return -1; }
  DBINFOAL(("Press::IOValue(GET) digitalRead"),(IOPin->Value()))
  for ( int i=0; i<10; i++) AveSum = AveSum + AveValues[i]; AveVal = AveSum/10;
  if ( AveVal < 97 ) AveVal = 97;
  return (AveVal - 97) * 0.2137;
}
//-----------------------------------------------------------------------------------------------------
void Press::IOValue(int _Value) { DBENTERAL(("Dist::IOValue[SET]: "),(_Value)) /*Read-Only Device*/ }
//-----------------------------------------------------------------------------------------------------
byte Press::Loop() {
  if ((millis() - LocalRefreshMS) < 500 ) return 0;             // Only read 1/2-Seconds
  LocalRefreshMS = millis();
  if (!IsLocal) return 0;

  if ( IOPin == 0 ) { DBERRORL(("Press::IOValue GET - IOPin==0")) return 1; }
  if ( IOPin->Value() < 0 || IOPin->Value() > 70 ) { DBERRORL(("Press::IOValue GET - IOPin OUT OF BOUNDS")) return 1; }

  // Add Read Value to Values to Average
  AveIter++;if ( AveIter > 9 ) AveIter = 0;
  pinMode(IOPin->Value(),INPUT);
  AveValues[AveIter] = analogRead(IOPin->Value());

  // If on display check if AveValue has changed and Display if it has.
  if (!OnDisplay()) return 0;
  int Ave = IOValue();
  if ( Ave == AveValue ) return 0;
  AveValue = Ave;
  DisplayValue();
}
//#####################################################################################################################
Dist::Dist(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware):
  Device(_TodeIndex, _RFID) {
    DBINITAAL(("Dist::Dist(TodeIndex,RFID,Hardware)"),(_TodeIndex),(_RFID))
    if (IsLocal) {
      if ( _Hardware==0 ) { DBERRORL(("Dist::Dist IsLocal but Hardware==0")) return; } 
      SubList = new MenuList("Dist Setup");
      TrigPin = SubList->Add(new PinSelect("Trig",_RFID,0,_Hardware));
      EchoPin = SubList->Add(new PinSelect("Echo",_RFID,1,_Hardware));
    }
}
//-----------------------------------------------------------------------------------------------------
void Dist::IOValue(int _Value) { DBENTERAL(("Dist::IOValue[SET]: "),(_Value)) /*Read-Only Device*/ }
//-----------------------------------------------------------------------------------------------------
int Dist::IOValue() {                                               DBENTERL(("Dist::IOValue[GET]"))

  if ( trigPin == 0 || echoPin == 0 ) {
    if ( TrigPin == 0 ) { DBERRORL(("Dist::IOValue[GET] - TrigPin==0")) return -1; }
    if ( EchoPin == 0 ) { DBERRORL(("Dist::IOValue[GET] - EchoPin==0")) return -1; }
    if ( 0 > TrigPin->Value() || TrigPin->Value() > 70 ) { DBERRORAL( ("Dist::IOValue[GET] - 0>TrigPin>70 OUT-OF-BOUNDS: "),( TrigPin->Value() ) ) return -1; }
    if ( 0 > EchoPin->Value() || EchoPin->Value() > 70 ) { DBERRORAL( ("Dist::IOValue[GET] - 0>EchoPin>70 OUT-OF-BOUNDS: "),( EchoPin->Value() ) ) return -1; }
    trigPin = TrigPin->Value();
    echoPin = EchoPin->Value();
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);    
  }

  int Sum=0; int Val = 0;
  for ( int i=0; i<10; i++) Sum = Sum + AveValues[i]; 
  Val = Sum/10;
  return Val;
}
//-----------------------------------------------------------------------------------------------------
byte Dist::Loop() {
  if ((millis() - LocalRefreshMS) < 500 ) return 0;             // Only read 1/2-Seconds
  LocalRefreshMS = millis();
  if (!IsLocal) return 0;

  if ( trigPin == 0 || echoPin == 0 ) {
    if ( TrigPin == 0 ) { DBERRORL(("Dist::IOValue[GET] - TrigPin==0")) return -1; }
    if ( EchoPin == 0 ) { DBERRORL(("Dist::IOValue[GET] - EchoPin==0")) return -1; }
    if ( 0 > TrigPin->Value() || TrigPin->Value() > 70 ) { DBERRORAL( ("Dist::IOValue[GET] - 0>TrigPin>70 OUT-OF-BOUNDS: "),( TrigPin->Value() ) ) return -1; }
    if ( 0 > EchoPin->Value() || EchoPin->Value() > 70 ) { DBERRORAL( ("Dist::IOValue[GET] - 0>EchoPin>70 OUT-OF-BOUNDS: "),( EchoPin->Value() ) ) return -1; }
    trigPin = TrigPin->Value();
    echoPin = EchoPin->Value();
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);    
  }

  // generate 10-microsecond pulse to TRIG pin
  digitalWrite(trigPin, LOW);delayMicroseconds(2);digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); digitalWrite(trigPin, LOW);
  
  float duration_us = pulseIn(echoPin, HIGH);     // measure duration of pulse from ECHO pin
  float distance_in = duration_us / 74 / 2;       // calculate the distance

  // Add Read Value to Values to Average
  AveIter++;if ( AveIter > 9 ) AveIter = 0;
  AveValues[AveIter] = (int)distance_in;

  // If on display check if AveValue has changed and Display if it has.
  if (!OnDisplay()) return 0;
  int Ave = IOValue();
  if ( Ave == AveValue ) return 0;
  AveValue = Ave;
  DisplayValue();
}
//#####################################################################################################################
STSTP3W::STSTP3W(byte _TodeIndex, byte _RFID, HdwSelect* _Hardware):
Device(_TodeIndex, _RFID) {
DBINITAAL(("STSTP3W::STSTP3W(TodeIndex,RFID,Hardware)"),(_TodeIndex),(_RFID))
  ValueRange(0,1);
  SetNumberName(0, "Off");
  SetNumberName(1, "On");
  if (IsLocal) {
    if ( _Hardware==0 ) { DBERRORL(("STSTP3W::STSTP3W IsLocal but Hardware==0")) return; } 
    SubList = new MenuList("3Wire Setup");
    StartPin = SubList->Add(new PinSelect("Start",_RFID,0,_Hardware));
    StopPin = SubList->Add(new PinSelect("Stop",_RFID,1,_Hardware));
    StatusPin = SubList->Add(new PinSelect("Status",_RFID,2,_Hardware));
  }    
}
//-----------------------------------------------------------------------------------------------------
void STSTP3W::IOValue(int _Value) {                        
DBENTERAL(("STSTP3W::IOValue[SET]: "),(_Value))

  if ( StartPin == 0 ) { DBERRORL(("STSTP3W::IOValue[SET]: StartPin==0")) return -1; }
  if ( StopPin == 0 ) { DBERRORL(("STSTP3W::IOValue[SET]: StopPin==0")) return -1; }
  if ( StartPin->Value() < 0 || StartPin->Value() > 70 ) { DBERRORL(("STSTP3W::IOValue[SET]. StartPin->Value() OUT OF BOUNDS")) return -1; }
  if ( StopPin->Value() < 0 || StopPin->Value() > 70 ) { DBERRORL(("STSTP3W::IOValue[SET]. StopPin->Value() OUT OF BOUNDS")) return -1; }
  
  if (_Value==0) {
    DBINFOAAL(("STSTP3W::IOValue[SET]. digitalWrite"),(StopPin->Value()),(0))
    digitalWrite(StopPin->Value(),0);
    LastStopMS = millis();
    
  } else if (_Value==1) {
    DBINFOAAL(("STSTP3W::IOValue[SET]. digitalWrite"),(StartPin->Value()),(0))
    digitalWrite(StartPin->Value(),0);
    LastStartMS = millis();
    
  } else {
    DBERRORL(("STSTP3W::IOValue[SET]. Value is NOT 1/0!"))
  }
}
//-----------------------------------------------------------------------------------------------------
int STSTP3W::IOValue() {                                               
DBENTERL(("STSTP3W::IOValue[GET]"))

  if ( StatusPin==0 ) { DBERRORL(("STSTP3W::IOValue[GET]: StatusPin==0")) return -1; }
  if ( StatusPin->Value() < 0 || StatusPin->Value() > 70 ) { DBERRORAL(("STSTP3W::IOValue[GET]: StatusPin OUT OF BOUNDS"),(StatusPin->Value())) return -1; }
  DBINFOAL( ("STSTP3W::IOValue[GET]. digitalRead "),(StatusPin->Value()) )
  return digitalRead(StatusPin->Value());
}
//-----------------------------------------------------------------------------------------------------
byte STSTP3W::Loop() {
  if ( LastStopMS==0 && LastStartMS==0 ) return 0;

  if ( LastStopMS!=0 ) {
    if ( StopPin == 0 ) { 
      DBERRORL(("STSTP3W::IOValue[SET]: IOPin==0")) 
      LastStopMS=0; return -1; 
    }
    if ( StopPin->Value() < 0 || StopPin->Value() > 70 ) { 
      DBERRORL(("STSTP3W::IOValue[SET]: StopPin->Value() OUT OF BOUNDS")) 
      LastStopMS=0; return -1;
    }
    if ( (millis() - LastStopMS)>=3000 ) {
      DBINFOAAL( ("STSTP3W::Loop. Momentary Release. digitalWrite"), (StopPin->Value()), (0) )
      digitalWrite(StopPin->Value(),1); 
      LastStopMS=0; return 0;
    }
  }

  if ( LastStartMS!=0 ) {
    if ( StartPin == 0 ) { 
      DBERRORL(("STSTP3W::IOValue[SET]: IOPin==0")) 
      LastStartMS=0; return -1; 
    }
    if ( StartPin->Value() < 0 || StartPin->Value() > 70 ) { 
      DBERRORL(("STSTP3W::IOValue[SET]: StartPin->Value() OUT OF BOUNDS")) 
      LastStartMS=0; return -1; 
    }
    if ( (millis() - LastStartMS)>=3000 ) {
      DBINFOAAL( ("STSTP3W::Loop. Momentary Release. digitalWrite"), (StartPin->Value()), (0) )
      digitalWrite(StartPin->Value(),1); LastStartMS=0; LastStopMS=0;
      LastStartMS=0; return 0; 
    }
  }
}
//_____________________________________________________________________________________________________________________
#endif
