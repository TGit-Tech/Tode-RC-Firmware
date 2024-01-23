/******************************************************************************************************************//**
 * @file    iDCL.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _IDCL_CPP
#define _IDCL_CPP

#include "iDCL.h"
//#####################################################################################################################
//                                    Control
//#####################################################################################################################
Control::Control(byte _TodeIndex, byte _RFID):
Device(_TodeIndex, _RFID, VTRW+VTDIG) {
DBINITAAL(("Control::Control(TodeIndex, RFID)"),(_TodeIndex),(_RFID))
  
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
int Control::IOValue() {                                                 
DBENTERL(("Control::IOValue[GET]"))
  int rtn = IVNOTSET;
  EEPROM.get(EEValAddress, rtn);
  DBINFOAAL(("Control::IOValue[GET] EEPROM.get(EEValAddress,rtn)"),(EEValAddress,HEX),(rtn))
  return rtn;
}
//-----------------------------------------------------------------------------------------------------
void Control::IOValue(int _Value) {             
DBENTERAL(("Control::IOValue[SET]"),(_Value))
  DBINFOAAL(("Control::IOValue[SET] EEPROM.put(EEValAddress,Value)"),(EEValAddress,HEX),(_Value))
  EEPROM.put(EEValAddress, _Value);
}
//-----------------------------------------------------------------------------------------------------
bool Control::DoLoop() {
  LoopReturn=BNONE;
  if (!IsLocal) return false;
  if ((millis() - LocalRefreshMS) < 500 ) return false;                               // Only Loop 1/2-Seconds
  LocalRefreshMS = millis();
  
  if ( SubList != 0 ) { ChangingSettings = true; return false; }                      // Currently Changing Settings now
  if ( ChangingSettings && SubList==0 ) { 
    ChangingSettings=false; 
    SettingsValid=false;
    ApplySettings(); 
  }  // Changing Settings Complete

  if ( !SettingsValid ) {
    if ( DLCDevs==0 ) { return false; }
    if ( DevType == DT_DC_MATH ) {
      if ( DLCDevs->InDev != 0 ) { 
        DBINFOL(("Control::DoLoop() DT_DC_MATH SettingsValid=true"))
        SettingsValid=true;
        return true; 
      }
    } else {
      if ( DLCDevs->InDev != 0 && DLCDevs->OutDev != 0 ) { 
        DBINFOL(("Control::DoLoop() SettingsValid=true"))
        SettingsValid=true; 
        return true; 
      }
    }
    if ( DLCDevs->InDevIdx!=BNONE && DLCDevs->OutTodeIdx!=BNONE && DLCDevs->OutDevIdx!=BNONE) {
      LoopReturn=LP_GETDEVICES; return false;
    }
  }
  return true;
}
//#####################################################################################################################
//                                                DCSetPoint
//#####################################################################################################################
DCSetPoint::DCSetPoint(byte _TodeIndex, byte _RFID):
Control(_TodeIndex, _RFID) {  
    DevType = DT_DC_SETPOINT;
    SetNumberName(-9999, F("Off"));
    DLCDevs = new DLCDevPtrs;
    EEValAddress = EESettingAddrBO(0);
    if (IsLocal) { 
      DBINFOL(("Control::Control(TodeIndex,RFID) IsLocal")) 
      EEValAddress = EESettingAddrBO(0); 
      ApplySettings(); 
    }
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void DCSetPoint::BuildSubList() {
DBENTERL(("DCSetPoint::BuildSubList()"))
  if (IsLocal) {  
      SubList = new MenuList(F("DCSetPoint"));
      SubList->RF = this->RF;
      
      // Menu Settable Device Settings
      MenuEEValue*  IDev=0;
      MenuEEValue*  InTol=0;
      MenuEEValue*  OTode=0;
      MenuEEValue*  ODev=0;
      MenuEEValue*  HOSet=0;
      MenuEEValue*  LOSet=0;
    
      SubList->Add(IDev =new MenuEEValue(F("InputDev")  ,EESettingAddrBO(2), VTRW+VTBYTE));
      SubList->Add(InTol=new MenuEEValue(F("InTolernc") ,EESettingAddrBO(3), VTRW+VTDIG));
      SubList->Add(OTode=new MenuEEValue(F("OutTode")   ,EESettingAddrBO(5), VTRW+VTBYTE));
      SubList->Add(ODev =new MenuEEValue(F("OutDev")    ,EESettingAddrBO(6), VTRW+VTBYTE));
      SubList->Add(LOSet=new MenuEEValue(F("LOutSet")   ,EESettingAddrBO(7), VTRW+VTBYTE));
      SubList->Add(HOSet=new MenuEEValue(F("HOutSet")   ,EESettingAddrBO(8), VTRW+VTBYTE));
      SubList->Add(new MenuName         (F("Del Device"),NAVDELDEV                      ));
      SubList->DestEEClear=false;

      IDev->ValueRange(0,8);
      OTode->ValueRange(0,8);
      ODev->ValueRange(0,8);
      
      HOSet->ValueRange(DCSPO_MIN,DCSPO_MAX);
      HOSet->SetNumberName(DCSPO_OFF,  F("OFF"));
      HOSet->SetNumberName(DCSPO_ON,   F("ON"));
      HOSet->SetNumberName(DCSPO_INCR, F("INCR"));
      HOSet->SetNumberName(DCSPO_DECR, F("DECR"));

      LOSet->ValueRange(DCSPO_MIN,DCSPO_MAX);
      LOSet->SetNumberName(DCSPO_OFF,  F("OFF"));
      LOSet->SetNumberName(DCSPO_ON,   F("ON"));
      LOSet->SetNumberName(DCSPO_INCR, F("INCR"));
      LOSet->SetNumberName(DCSPO_DECR, F("DECR"));
  }
}
//-----------------------------------------------------------------------------------------------------
void DCSetPoint::ApplySettings() {
DBENTERL(("DCSetPoint::ApplySettings()"))

  // Get Device Settings
  if ( DLCDevs==0 ) { DBERRORL(("DCSetPoint::ApplySettings() DLCDevs==0")) }
  else {
    DLCDevs->InDev = 0;     // Clear any previously set pointers
    DLCDevs->OutDev = 0;
    DLCDevs->InDevIdx =     EEPROM.read(EESettingAddrBO(2));
                            EEPROM.get( EESettingAddrBO(3), Tolerance);
    DLCDevs->OutTodeIdx =   EEPROM.read(EESettingAddrBO(5));
    DLCDevs->OutDevIdx =    EEPROM.read(EESettingAddrBO(6));
              LOSetV   =    EEPROM.read(EESettingAddrBO(7));
              HOSetV   =    EEPROM.read(EESettingAddrBO(8));
  }
  // SettingsValid=true in Loop() after Device pointers received from Sys.Loop()
}
//-----------------------------------------------------------------------------------------------------
byte DCSetPoint::Loop() { 
  if (!DoLoop()) return LoopReturn;   
  if (!OnDisplay()) return 0;

  int SetPoint = Value();                                                     // Get current SetPoint
  if ( DLCDevs->InDev != 0 && DLCDevs->OutDev != 0 && SetPoint != -9999 ) {   // -9999 = Automation 'OFF'
      int InDevValue = DLCDevs->InDev->Value();                               // Get the current Input Device Reading            
      int SetOutDevValue = IVNOTSET;                                          // Variable to SetTo
      if ( InDevValue != LastInDevValue || SetPoint != LastSetPoint ) {       // Updates require Input or Setpoint change.
      
        if ( InDevValue < SetPoint-Tolerance ) { SetOutDevValue = LOSetV; } 
        else if ( InDevValue > SetPoint+Tolerance ) { SetOutDevValue = HOSetV; }
        
        // Output Assign if not same as last ( outside last tolerance for DIFF )
        if ( DLCDevs->OutTodeIdx == 0 || SetOutDevValue == DCSPO_OFF || SetOutDevValue == DCSPO_ON ) {
          if ( SetOutDevValue == DCSPO_INCR ) SetOutDevValue = DLCDevs->OutDev->Value() + abs(InDevValue-SetPoint); // Adjust INCR
          if ( SetOutDevValue == DCSPO_DECR ) SetOutDevValue = DLCDevs->OutDev->Value() - abs(InDevValue-SetPoint); // Adjust DECR
          if ( LastSetOutDevValue == SetOutDevValue || SetOutDevValue == IVNOTSET ) {
            DBINFOAL(("DCSetPoint::Loop() LastSetOutDevValue == SetOutDevValue || SetOutDevValue == IVNOTSET"),(SetOutDevValue))
          } else {
            DBINFOAAL(("DCSetPoint::Loop() ?LastSetOutDevValue? != ?SetOutDevValue?"),(LastSetOutDevValue),(SetOutDevValue))
            DLCDevs->OutDev->Value(SetOutDevValue,STSUSERSET);
            LastInDevValue = InDevValue;
            LastSetOutDevValue = SetOutDevValue;   
            DBINFOAL(("DCSetPoint::Loop() DLCDevs->OutDev->Value(SetOutDevValue)"),(SetOutDevValue))
          }

        // Preform a REMOTE TODE Set with DIFF, INCR or DECR
        } else {
          if ( millis() - LastRFSetMS > 14000 ) {       // 14118ms = 14.1sec intervals
            LastRFSetMS = millis();
            int CurrentOutDevValue = IVNOTSET;
            if ( LastSetOutDevValue != IVNOTSET ) CurrentOutDevValue = LastSetOutDevValue;          // Use Last Set Value
            if ( CurrentOutDevValue == IVNOTSET ) {
              DLCDevs->OutTode->Update();
              CurrentOutDevValue = DLCDevs->OutDev->Value();    // Or Get Current Value
              DBINFOAL(("DCSetPoint::Loop() 14s ?CurrentOutDevValue? = DLCDevs->OutDev->Value()"),(CurrentOutDevValue))
            }
            if ( CurrentOutDevValue != IVNOTSET ) {
              if ( SetOutDevValue == DCSPO_INCR ) SetOutDevValue = CurrentOutDevValue + abs(InDevValue-SetPoint); // Adjust INCR
              if ( SetOutDevValue == DCSPO_DECR ) SetOutDevValue = CurrentOutDevValue - abs(InDevValue-SetPoint); // Adjust DECR
              if ( LastSetOutDevValue == SetOutDevValue || SetOutDevValue == IVNOTSET ) {
                DBINFOAL(("DCSetPoint::Loop() 14s LastSetOutDevValue == SetOutDevValue || SetOutDevValue == IVNOTSET"),(SetOutDevValue))
              } else {
                //LastRFSetMS = millis();
                DBINFOAAL(("DCSetPoint::Loop() 14s ?LastSetOutDevValue? != ?SetOutDevValue?"),(LastSetOutDevValue),(SetOutDevValue))
                DLCDevs->OutDev->Value(SetOutDevValue,STSUSERSET);
                LastInDevValue = InDevValue;
                LastSetOutDevValue = SetOutDevValue;
                DBINFOAL(("DCSetPoint::Loop() 14s DLCDevs->OutDev->Value(SetOutDevValue)"),(SetOutDevValue))
              }
            } else { DBINFOL(("DCSetPoint::Loop() 14s CurrentOutDevValue == IVNOTSET")) }
          }
        }
      }

  } else { DBINFOL(("DCSetPoint::Loop() DLCDevs->InDev == 0 || DLCDevs->OutDev == 0")) }
  
  // If Value has Changed Display Change
  if ( SetPoint == LastSetPoint ) return 0;
  LastSetPoint = SetPoint;
  DisplayValue();

}
//#####################################################################################################################
//                                    DCMath
//#####################################################################################################################
DCMath::DCMath(byte _TodeIndex, byte _RFID):
Control(_TodeIndex, _RFID) {  
    DevType = DT_DC_MATH;
    DLCDevs = new DLCDevPtrs;
    if (IsLocal) { 
      DBINFOL(("DCMath::DCMath(TodeIndex,RFID) IsLocal")) 
      ApplySettings(); 
    }
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
int DCMath::IOValue() {                                                 
DBENTERL(("DCMath::IOValue[GET]"))
  if ( !SettingsValid ) { DBERRORL(("DCMath::IOValue[GET] !SettingsValid")) return -1; }
  
  if ( DLCDevs->InDev == 0 ) {
    DBINFOL(("DCMath::IOValue[GET] DLCDevs->InDev == 0"))
    return 0;    
  }

  int InDevValue = DLCDevs->InDev->Value();
  if ( Operator == DCLM_COPY ) { return InDevValue; }
  if ( Operator == DCLM_ICOPY ) { 
    if ( InDevValue == 0 ) { return 1; }
    else if ( InDevValue == 1 ) { return 0; } 
    else { return InDevValue*-1; }
  }
  
  if ( DLCDevs->In2Dev == 0 ) { 
    DBINFOL(("DCMath::IOValue[GET] DLCDevs->InDev == 0 || DLCDevs->In2Dev == 0"))
    return 0;
  }

  // Do Math
  int In2DevValue = DLCDevs->In2Dev->Value();
  DBINFOAAL(("DCMath::IOValue[GET] ?InDevValue?, ?In2DevValue?"),(InDevValue),(In2DevValue))
  switch(Operator) {
    case DCLM_ADD:
      return DLCDevs->InDev->Value() + DLCDevs->In2Dev->Value(); break;
    case DCLM_SUBTRACT:
      return DLCDevs->InDev->Value() - DLCDevs->In2Dev->Value(); break;
    case DCLM_MULTIPLY:
      return DLCDevs->InDev->Value() * DLCDevs->In2Dev->Value(); break;
    case DCLM_DIVIDE:
      int Denominator = DLCDevs->In2Dev->Value();
      if (Denominator != 0 ) return DLCDevs->InDev->Value() / DLCDevs->In2Dev->Value();
    case DCLM_AVERAGE:
      return (DLCDevs->InDev->Value() + DLCDevs->In2Dev->Value())/2; break;
  }
  return 0;
}
//-----------------------------------------------------------------------------------------------------
void DCMath::IOValue(int _Value) {  DBENTERAL(("DCMath::IOValue[SET]: "),(_Value)) /*Read-Only Device*/ }           
//-----------------------------------------------------------------------------------------------------
void DCMath::BuildSubList() {
DBENTERL(("DCMath::BuildSubList()"))
  if (IsLocal) {
      //if ( Hardware==0 ) { DBERRORL(("DCMath::BuildSubList() IsLocal but Hardware==0")) return; } 
      SubList = new MenuList(F("DCL Math"));
      SubList->RF = this->RF;

      MenuEEValue* In1Dev=0;
      MenuEEValue* In2Dev=0;
      MenuEEValue* MathOpr=0;
      
      // Menu Settable Device Settings
      SubList->Add(In1Dev  =new MenuEEValue(F("In1Dev")     ,EESettingAddrBO(2), VTRW+VTBYTE));
      SubList->Add(MathOpr =new MenuEEValue(F("Operator")   ,EESettingAddrBO(3), VTRW+VTBYTE));
      SubList->Add(In2Dev  =new MenuEEValue(F("In2Dev")     ,EESettingAddrBO(4), VTRW+VTBYTE));
      SubList->Add(         new MenuEEValue(F("Tolerenc")   ,EESettingAddrBO(5), VTRW+VTDIG));
      SubList->Add(         new MenuEEValue(F("OutTode")    ,EESettingAddrBO(7), VTRW+VTBYTE));
      SubList->Add(         new MenuEEValue(F("OutDev")     ,EESettingAddrBO(8), VTRW+VTBYTE));
      SubList->Add(         new MenuName   (F("Del Device") ,NAVDELDEV                      ));
      SubList->DestEEClear=false;

      In1Dev->ValueRange(0,16);
      In1Dev->SetNumberName(16,   F("NONE"));
      In1Dev->SetNumberName(255,  F("NotSet"));
      In2Dev->ValueRange(0, 16);
      In2Dev->SetNumberName(16,   F("NONE"));
      In2Dev->SetNumberName(255,  F("NotSet"));
      
      MathOpr->ValueRange(DCLM_ADD, DCLM_ICOPY);
      MathOpr->SetNumberName(DCLM_ADD,      F("+"));
      MathOpr->SetNumberName(DCLM_SUBTRACT, F("-"));
      MathOpr->SetNumberName(DCLM_MULTIPLY, F("x"));
      MathOpr->SetNumberName(DCLM_DIVIDE,   F("/"));
      MathOpr->SetNumberName(DCLM_AVERAGE,  F("AVE"));
      MathOpr->SetNumberName(DCLM_COPY,     F("COPY"));
      MathOpr->SetNumberName(DCLM_ICOPY,    F("ICOPY"));
  }
}
//-----------------------------------------------------------------------------------------------------
void DCMath::ApplySettings() {
DBENTERL(("DCMath::ApplySettings()"))

  // Get Device Settings
  if ( DLCDevs==0 ) { DBERRORL(("DCMath::ApplySettings() DLCDevs==0")) }
  else {
    DLCDevs->InDev = 0;     // Clear any previously set pointers
    DLCDevs->In2Dev = 0;
    DLCDevs->OutDev = 0;
    LastSetOutDevValue = IVNOTSET;
    DLCDevs->InDevIdx =     EEPROM.read(EESettingAddrBO(2));
    Operator =              EEPROM.read(EESettingAddrBO(3));
    DLCDevs->In2DevIdx =    EEPROM.read(EESettingAddrBO(4));
                            EEPROM.get(EESettingAddrBO(5), Tolerenc);
    DLCDevs->OutTodeIdx =   EEPROM.read(EESettingAddrBO(7));
    DLCDevs->OutDevIdx =    EEPROM.read(EESettingAddrBO(8));
  }
  // SettingsValid=true in Loop() after Device pointers received from Sys.Loop()
}
//-----------------------------------------------------------------------------------------------------
byte DCMath::Loop() { 
  if (!DoLoop()) return LoopReturn;   
  if (!OnDisplay()) return 0;

  int TheValue = IOValue();                      // Get the current calculated Reading
  
  // If an Output Device was given Send the Value to Out Device when outside tolerances
  bool bRemoteSet = false;
  if ( DLCDevs->OutDev != 0 ) { 
    if ( TheValue < LastSetOutDevValue-Tolerenc || TheValue > LastSetOutDevValue+Tolerenc  ) {
      if ( DLCDevs->OutTodeIdx == 0 ) { bRemoteSet=true; }
      else if ( millis() - LastRFSetMS > 14000 ) { LastRFSetMS = millis(); bRemoteSet=true; }
      if ( bRemoteSet ) {
        LastSetOutDevValue = TheValue;
        DLCDevs->OutDev->Value(TheValue,STSUSERSET);   
        DBINFOAL(("DCMath::Loop() DLCDevs->OutDev->Value(?TheValue?,STSUSERSET)"),(TheValue))      
      }
    }
  }
  
  // If on display check if AveValue has changed and Display if it has.
  if (!OnDisplay()) return 0;
  int Ave = TheValue;
  if ( Ave == LastGetIOValue ) return 0;
  LastGetIOValue = Ave;
  DisplayValue();
}
//_____________________________________________________________________________________________________________________
#endif
