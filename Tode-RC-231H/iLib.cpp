/******************************************************************************************************************//**
 * @file    MenuLib.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _ILIB_CPP
#define _ILIB_CPP

#include "iLib.h" 
//#####################################################################################################################
SetMenuName::SetMenuName(int _EENameAddress):
GetDisplay() {
  DBINITAL(("SetMenuName::SetMenuName(EENameAddress)"),(_EENameAddress))
  EENameAddress=_EENameAddress;
  DisplayNameSet();
}
//.....................................................................................................
byte SetMenuName::NavNameSet(byte _Key) {                   
DBENTERAL(("SetMenuName::NavNameSet(Key)"),(_Key,HEX))

  // NSTITLE=28, NSPOS=29, NSEXIT=30, NSSAVE=31
  DspChar();                                                // Unselect Current Char
  byte chars_per_row = SCREENWIDTH / (ITEMCHARWIDTH+2);     // gets 14chrs on 128pxlw
  if ( _Key == NAVKEYUP ) {
    if ( cSetTo>chars_per_row+32 ) { cSetTo = cSetTo - chars_per_row; } // 2nd Char-Row > Prev Row
    else if ( cSetTo>=32 ) { cSetTo = NSEXIT; }                         // 1st Char-Row > Exit
    else if ( cSetTo>=NSEXIT ) { cSetTo = NSPOS; }                      // Exit/Save > Text Pos
    else if ( cSetTo==NSPOS ) { cSetTo = NSTITLE; }                     // Text Pos ->Title
    
  } else if ( _Key == NAVKEYDOWN ) {
    if ( cSetTo==NSTITLE ) { cSetTo = NSPOS; }                          // Title->Pos
    else if ( cSetTo==NSPOS ) { cSetTo = NSEXIT; }                      // Pos->Exit
    else if ( cSetTo<=NSSAVE ) { cSetTo = 32; }                         // Exit/Save to Space
    else { cSetTo = cSetTo + chars_per_row; }                           // Next Char Row
    if ( cSetTo>126 ) cSetTo = 126;                                     // Check overflow
      
  } else if ( _Key == NAVKEYRIGHT ) {
    if ( cSetTo==NSPOS ) { if (SetIdx<EEMAXNAMESIZE-1) SetIdx++; }
    else if ( cSetTo<126 ) { cSetTo++; }
      
  } else if ( _Key == NAVKEYLEFT ) {
    if ( cSetTo==NSPOS ) { if (SetIdx>0) SetIdx--; }
    else if (cSetTo==NSTITLE) { NameSetOnScreen=false; return NAVEXITNAMESET; }
    else if (cSetTo>NSTITLE) { cSetTo--; }
      
  } else if ( _Key == NAVKEYSET || _Key == NAVKEYOKAY ) {
    if ( cSetTo==NSEXIT ) { NameSetOnScreen=false; return NAVEXITNAMESET; }
    else if ( cSetTo==NSSAVE ) { EEPROM.put(EENameAddress, SetToName); return NAVKEYNONE; }
    else if ( cSetTo>=32 ) { 
      SetToName[SetIdx] = cSetTo; 
      if (SetIdx<EEMAXNAMESIZE-1) SetIdx++;
      DspSetToName();
    }

  }
  DspChar(1);     // Display Character Selected
  return NAVKEYNONE;
}
//-----------------------------------------------------------------------------------------------------
void SetMenuName::DisplayNameSet() {
DBENTERL(("SetMenuName::DisplayNameSet()"))
  if ( Display==0 ) { DBERRORL(("Display==0")) return 0; }
  byte chars_per_row = SCREENWIDTH / (ITEMCHARWIDTH+2);
  
  if ( !NameSetOnScreen ) { 
    Display->fillScreen(0x0000); NameSetOnScreen = true;                // Clear Screen, Mark Displayed 
    for (int i=0;i<EEMAXNAMESIZE;i++) { FName[i] = ' '; }       // Clean get array 'EEFName'
    EEPROM.get(EENameAddress, FName);
    FName[EEMAXNAMESIZE]='\0';                               // Add Terminator in +1 FName
    const char* CurrentName = FName;
    for ( int i=0; i<EEMAXNAMESIZE; i++ ) { SetToName[i] = FName[i]; }       // Clear 'SetToName' and load-it
    //for ( int i=0; i<strlen(CurrentName); i++ ) { SetToName[i] = CurrentName[i]; }
  }

  for ( int i=28; i<127; i++ ) { cSetTo=i; DspChar(); }                 // Display ALL Chars
  cSetTo=NSTITLE;
}
//-----------------------------------------------------------------------------------------------------
void SetMenuName::DspSetToName(bool _Selected) {
  Display->SETCURSOR(0, 28);
  Display->setTextColor(BLACK);
  Display->fillRect(0,28-ITEMCHARHEIGHT+2,(ITEMCHARWIDTH+1)*EEMAXNAMESIZE,ITEMCHARHEIGHT, _Selected ? BLUE: ORANGE);  //Name BG
  Display->fillRect( SetIdx*(ITEMCHARWIDTH+1), 28-ITEMCHARHEIGHT+2, ITEMCHARWIDTH+2,ITEMCHARHEIGHT,WHITE);            //Per-Char
  Display->print(SetToName);  
}
//-----------------------------------------------------------------------------------------------------
void SetMenuName::DspChar(bool _Selected) {
  
  if ( cSetTo==NSTITLE ) {
    Display->SETCURSOR(0, TITLECHARHEIGHT); Display->TITLEFONT;
    Display->setTextColor( _Selected ? BLACK: WHITE );
    Display->fillRect( 0, 0, TITLECHARWIDTH*8, ITEMCHARHEIGHT+3, _Selected ? BLUE: BLACK);
    Display->print("Set Name"); Display->ITEMFONT;
    
  } else if ( cSetTo==NSPOS ) { 
    DspSetToName(_Selected);
    
  } else if ( cSetTo==NSEXIT ) {
    Display->SETCURSOR(0, 45);
    Display->setTextColor( _Selected ? BLACK: WHITE );
    Display->fillRect( 0, 45-ITEMCHARHEIGHT, (ITEMCHARWIDTH+1)*4, ITEMCHARHEIGHT+3, _Selected ? BLUE: BLACK);
    Display->print("EXIT");
        
  } else if ( cSetTo==NSSAVE ) {
    Display->SETCURSOR(50, 45);
    Display->setTextColor( _Selected ? BLACK: WHITE );
    Display->fillRect( 50, 45-ITEMCHARHEIGHT, (ITEMCHARWIDTH+1)*4, ITEMCHARHEIGHT+3, _Selected ? BLUE: BLACK);
    Display->print("SAVE");
    
  } else {              // Calculate the position for char keypad
    int X=0, Y=0, RowNum=0, ColNum=0;
    byte chars_per_row = SCREENWIDTH / (ITEMCHARWIDTH+2); // gets 14 on 128
    RowNum=(cSetTo-32)/chars_per_row; Y=RowNum*(ITEMCHARHEIGHT+2)+60;
    ColNum=(cSetTo-32)-(RowNum*chars_per_row); X=ColNum*(ITEMCHARWIDTH+2);
    Display->SETCURSOR(X,Y);
    Display->setTextColor( _Selected? BLACK: WHITE);
    Display->fillRect( X-1, Y - (ITEMCHARHEIGHT), ITEMCHARWIDTH+4, ITEMCHARHEIGHT+3, _Selected? YELLOW: BLACK); 
    Display->print(cSetTo);
  }
}
//-----------------------------------------------------------------------------------------------------

//#####################################################################################################################
MenuName::MenuName(const __FlashStringHelper* _CName, MenuList* _SubList):
MenuItem() {
DBINITAAL(("MenuName::MenuName(CName,MenuList)"),(_CName),(int(_SubList),HEX))
  CName = _CName; 
  SubList = _SubList;
}
//.....................................................................................................
MenuName::MenuName(const __FlashStringHelper* _CName, byte _ReturnKey):
MenuItem() {
DBINITAAL(("MenuName::MenuName(CName,ReturnKey)"),(_CName),(_ReturnKey,HEX))
  CName = _CName; 
  ReturnKey = _ReturnKey;
}
//.....................................................................................................
MenuName::MenuName(int _EENameAddress, bool _NameSettable):
MenuItem() {
DBINITAAL(("MenuName::MenuName(EENameAddress,NameSettable)"),(_EENameAddress),(_NameSettable))
  EENameAddress = _EENameAddress;
  NameSettable = _NameSettable;
}
//.....................................................................................................
MenuName::MenuName(int _EENameAddress, byte _Offset, byte _ReturnKey):
MenuItem() {
DBINITAAL(("MenuName::MenuName(?EENameAddress?,Offset,?ReturnKey?)"),(_EENameAddress),(_ReturnKey,HEX))
  EENameAddress = _EENameAddress + _Offset;
  NameSettable = false;
  ReturnKey = _ReturnKey;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MenuName::BuildSubList() { }
//-----------------------------------------------------------------------------------------------------
const char* MenuName::Name() {                                        
DBENTERL(("MenuName::Name[GET]"))
  if (CName != 0) {
    DBINFOL(("MenuName::Name() pgm_read_byte"))
    for (int i=0;i<EEMAXNAMESIZE;i++) { FName[i] = ' '; }       // Clean get array 'EEFName'
    PGM_P p = reinterpret_cast<PGM_P>(CName);
    size_t n = 0;
    while (1) {
      unsigned char c = pgm_read_byte(p++);
      if (c==0) break;
      FName[n]=c; n++;
    }
    FName[EEMAXNAMESIZE]='\0';                               // Add Terminator in +1 FName
    return FName;
  }
  if ( EENameAddress >= 0 ) { 
    DBINFOAL(("MenuName::Name() EEPROM.get(?EENameAddress?, FName)"),(EENameAddress,HEX))
    for (int i=0;i<EEMAXNAMESIZE;i++) { FName[i] = ' '; }       // Clean get array 'EEFName'
    EEPROM.get(EENameAddress, FName);                           // Get characters
    FName[EEMAXNAMESIZE]='\0';                               // Add Terminator in +1 FName
    return FName;
  }
  DBERRORAL(("CName==0 and ?EENameAddress?<0 at : "),(EENameAddress)) 
  return "?Name?";
}
//-----------------------------------------------------------------------------------------------------
void MenuName::Name(const char* _Name) {                  
DBENTERAL(("MenuName::Name[SET] = "),(_Name))
  if ( EENameAddress < 0 ) { DBERRORL(("EENameAddress<0")) return; }

  for ( int i=0; i<EEMAXNAMESIZE; i++) { 
    if (i>strlen(_Name)) { FName[i]=0xFF; }
    else { FName[i] = _Name[i]; }
  }
  EEPROM.put(EENameAddress, FName);
}
//-----------------------------------------------------------------------------------------------------
void MenuName::Name(const __FlashStringHelper* _Name) {
DBENTERAL(("MenuName::Name[SET] = "),(_Name))
  CName = _Name;
}
//-----------------------------------------------------------------------------------------------------
byte MenuName::Navigate(byte _Key) {                 DBENTERAL(("MenuName::Navigate(key):"),(_Key,HEX))

  // SELECTOR
  if ( NavSelected == SEL_NONE ) { _Key = NavNone(_Key); }
  if ( NavSelected == SEL_NAME ) { _Key = NavName(_Key); }
  if ( NavSelected == SEL_SETNAME ) { 
    if (MenuNameSet==0) MenuNameSet=new SetMenuName(EENameAddress);
    _Key = MenuNameSet->NavNameSet(_Key); 
  }
  if ( _Key == NAVEXITNAMESET ) { 
    delete(MenuNameSet);
    MenuNameSet=0;
    DBINFOL(("MenuName::Navigate Key == NAVEXITNAMESET"))
    NavSelected=SEL_NAME; 
    return NAVDSPNEWLIST; 
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
void MenuName::OnDisplay(bool _OnDisplay) { 
  if (!_OnDisplay) NavSelected = SEL_NONE;
  bOnDisplay = _OnDisplay; 
}
//-----------------------------------------------------------------------------------------------------
bool MenuName::OnDisplay() { return bOnDisplay; }
//-----------------------------------------------------------------------------------------------------
void MenuName::DispItem(byte _XPos, byte _YPos, byte _DispMode) {
  DispMode = _DispMode;
  if (_XPos!=0) XPos=_XPos; if (_YPos!=0) YPos=_YPos;
  OnDisplay(true);
  DisplayName();
}
//-----------------------------------------------------------------------------------------------------
byte MenuName::NavNone(byte _Key) {
DBENTERAL(("MenuName::NavNone"),(_Key,HEX))
  //NavSelected=SEL_NAME;
  //return NavName(_Key);
}
//-----------------------------------------------------------------------------------------------------
byte MenuName::NavName(byte _Key) {                         
DBENTERAL(("MenuName::NavName"),(_Key,HEX))
  // Processes a SET or OKAY
  if ( _Key == NAVKEYSET && NameSettable ) { 
    NavSelected = SEL_SETNAME;
    if (MenuNameSet==0) MenuNameSet=new SetMenuName(EENameAddress);
    return NAVKEYNONE; 
  }
  else if ( _Key == NAVKEYOKAY ) {
    if ( SubList == 0 ) {
      BuildSubList();
      if ( SubList != 0 ) BuiltSubList = true;
    }
    if ( SubList != 0 ) { DBINFOL(("MenuName::NavName NAVGOTOSUBLIST")) return NAVGOTOSUBLIST; }
    if ( ReturnKey != NAVKEYNONE ) return ReturnKey;
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
void MenuName::DisplayName(byte _ID) {
//DBENTERAL(("MenuName::DisplayName"),(Name()))
  if ( Display==0 ) { DBERRORL(("MenuName::DisplayName Display==0")) return; }
  Display->SETCURSOR(XPos,YPos);
  if ( NavSelected==SEL_NAME ) { Display->BGNAMESELT; Display->TXTSELT; }
  else { Display->BGNAMENORM; Display->TXTNORM; }
  if (_ID!=BNONE) Display->print(_ID);
  Display->print(Name());
  if ( EENameAddress >= 0 && DispMode == DMNORMAL && ReturnKey == NAVKEYNONE) { 
    Display->SETCURSOR(VALCOL,YPos); Display->print(F("[NAME]")); 
  }
}

//-----------------------------------------------------------------------------------------------------

//#####################################################################################################################
MenuValue::MenuValue(const __FlashStringHelper* _CName, byte _ValueType):
  MenuName(_CName) { 
    VType = _ValueType; 
    if ( VType&VTBYTE ) { VMin=0x00; VMax=0xFF; } 
}
//.....................................................................................................
MenuValue::MenuValue(const __FlashStringHelper* _CName, byte _ValueType, byte _ReturnKey):
  MenuName(_CName,_ReturnKey) {
    VType = _ValueType;
    if ( VType&VTBYTE ) { VMin=0x00; VMax=0xFF; } 
}
//.....................................................................................................
MenuValue::MenuValue(int _EENameAddress, bool _NameSettable=false, byte _ValueType):
  MenuName(_EENameAddress, _NameSettable) { 
    VType = _ValueType; 
    if ( VType&VTBYTE ) { VMin=0x00; VMax=0xFF; } 
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MenuValue::ValueRange(int _VMin, int _VMax) { VMin = _VMin; VMax = _VMax; }
//-----------------------------------------------------------------------------------------------------
int MenuValue::Value() {                                      
//DBENTERAL(("MenuValue::Value(GET): "),(iValue)) 
  return iValue; 
}
//-----------------------------------------------------------------------------------------------------
void MenuValue::Value(int _Value, byte _Status = STSNORMAL) { 
//DBENTERAAL(("MenuValue::Value(SET,STS): "),(_Value),(_Status,HEX))
  iValue = _Value; yStatus = _Status;
}
//-----------------------------------------------------------------------------------------------------
byte MenuValue::Navigate(byte _Key) {                         DBENTERAL(("MenuValue::Navigate"),(_Key,HEX))

  // SELECTOR
  if (      NavSelected == SEL_NONE ) {     _Key = NavNone(_Key); }
  else if ( NavSelected == SEL_NAME ) {     _Key = NavName(_Key); if (_Key==NAVKEYRIGHT) { NavSelected=SEL_VALUE; DispItem(); return NAVKEYNONE; } }
  else if ( NavSelected == SEL_SETNAME ) {  
    if (MenuNameSet==0) MenuNameSet=new SetMenuName(EENameAddress);
    _Key = MenuNameSet->NavNameSet(_Key); 
  }
  else if ( NavSelected == SEL_VALUE ) {
    _Key = NavValue(_Key);    
    if ( NavSelected==SEL_SETVALUE ) {
      if (ValueSet==0) ValueSet=new SetValue(XPos, YPos, FirstNumberName, Value(), VMin, VMax, VType);
      DBINFOL(("MenuValue::Navigate new SetValue("))
    }
  }
  else if ( NavSelected == SEL_SETVALUE ) {
    if (ValueSet==0) { DBERRORL(("MenuValue::Navigate ValueSet==0")) }
    else { _Key = ValueSet->NavValueSet(_Key); }
  }
  
  // Return Key Processing
  if ( _Key == NAVEXITNAMESET ) { 
    delete(MenuNameSet);
    MenuNameSet=0;
    DBINFOL(("MenuValue::Navigate Key == NAVEXITNAMESET"))
    NavSelected=SEL_NAME; 
    return NAVDSPNEWLIST;
  } 
  if ( _Key == NAVEXITVALUESET || _Key == NAVSAVEVALUESET ) { 
    if ( _Key == NAVSAVEVALUESET ) {
      Value(ValueSet->iSetTo, STSUSERSET);
      DBINFOAL(("MenuValue::Navigate Value(iSetTo, STSUSERSET)"),(ValueSet->iSetTo))
    }
    delete(ValueSet);
    ValueSet=0;
    NavSelected=SEL_VALUE;
    DispItem();
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
void MenuValue::DispItem(byte _XPos, byte _YPos, byte _DispMode) {
//DBENTERAAL(("MenuValue::DispItem"),(_XPos),(_YPos))
  DispMode = _DispMode;
  if (_XPos!=0) XPos=_XPos; if (_YPos!=0) YPos=_YPos;
  DBINFOL(("OnDisplay(true)"))
  OnDisplay(true);
  DisplayName();
  if ( DispMode == DMNORMAL ) DisplayValue();
}
//-----------------------------------------------------------------------------------------------------
byte MenuValue::NavValue(byte _Key) {                     
//DBENTERAL(("MenuValue::NavValue"),(_Key,HEX))
  if ( _Key == NAVKEYSET && (VType&VTRW) ) { 
    NavSelected=SEL_SETVALUE;     
    return NAVKEYNONE;
    
  } else if ( _Key==NAVKEYLEFT ) {
    NavSelected=SEL_NAME;
    DispItem(); return NAVKEYNONE;
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
void MenuValue::DisplayValue() {
DBENTERL(("MenuValue::DisplayValue()"))
  if ( Display==0 ) { DBERRORL(("MenuValue::DisplayValue Display==0")) return; }

  // Set Cursor Position & Color Scheme
  Display->SETCURSOR(XPos+VALCOL,YPos);                
  if ( NavSelected==SEL_VALUE ) { Display->BGVALSELT; Display->TXTSELT; }
  else { Display->BGVALNORM; Display->TXTNORM; }                                  
  //DBINFOAAL(("MenuValue::DisplayValue VMin,VMax: "),(uint16_t(VMin),HEX),(uint16_t(VMax),HEX))
  
  // Get Value Name if one assigned
  const __FlashStringHelper* ValueName = 0;  
  //if ( !(VType&VTDIG)) { ValueName = GetNumberName(Value()); }     // ValueName NOT compatible with VTDIG
  ValueName = GetNumberName(Value());
  if ( ValueName!=0 ) { Display->print(ValueName); return; }    // Display NAMED Value
  if ( !(VType&VTDIG && NavSelected==SEL_SETVALUE) ) {          // NOT in Per-Digit SETVALUE
    if ( VType&VTHEX ) { Display->print(uint16_t(Value()), HEX); } // Display HEX Value
    else { Display->print(Value(), DEC); }                         // Display DEC Value
    return;
  }   
}
//-----------------------------------------------------------------------------------------------------
void MenuValue::SetNumberName(int _Value, const __FlashStringHelper* _NName) {
  if (FirstNumberName==0) {
    FirstNumberName = new NumberName;
    FirstNumberName->Number = _Value; 
    FirstNumberName->Name = _NName;
  } else {
    CurrNumberName = FirstNumberName;
    while ( CurrNumberName->Next != 0 ) { CurrNumberName = CurrNumberName->Next; }
    CurrNumberName->Next = new NumberName; 
    CurrNumberName->Next->Number = _Value;
    CurrNumberName->Next->Name = _NName;
  }
}
//-----------------------------------------------------------------------------------------------------
const __FlashStringHelper* MenuValue::GetNumberName(int _Value) {
  CurrNumberName = FirstNumberName;
  while ( CurrNumberName != 0 ) {
    //DBINFOAL(("MenuValue::GetNumberName Searchfor: "),(CurrNumberName->Number))
    if ( CurrNumberName->Number == _Value ) return CurrNumberName->Name;
    CurrNumberName = CurrNumberName->Next;
  }
  return 0;
}
//#####################################################################################################################
SetValue::SetValue(byte _XPos, byte _YPos, NumberName* _FirstNumberName, int _CurrValue, int _VMin, int _VMax, byte _VType):
GetDisplay() {
DBINITAAL(("SetValue::SetValue(XPos,YPos)"),(_XPos),(_YPos))

  XPos = _XPos; YPos = _YPos;
  FirstNumberName = _FirstNumberName;
  iSetTo = _CurrValue;
  VMin = _VMin; VMax = _VMax;
  VType = _VType;
  
  if ( _VType&VTDIG ) {                                           // If Value is Per-Digit Set
    uint16_t base = (VType&VTHEX) ? HEX : DEC;                      // Select the 'base'
    byte MinDigits = DigitCount(VMin,base);                         // How many digits in VMin
    byte MaxDigits = DigitCount(VMax,base);                         // How many digits in VMax
    VRngDigits = (MinDigits>MaxDigits) ? MinDigits : MaxDigits;     // Set VRngDigits to larger of the two
    
    if ( !(VType&VTHEX) && (VMin<0||VMax<0 ) ) {                  // SIGNED Value? (HEX is always +)
        bVSigned = true; VRngDigits++; yVSignDig = VRngDigits;      // Add (1) Digit for +/- and mark its location
        DBINFOL(("SetValue::SetValue bVSigned=true"))
    }
    DBINFOAAL(("SetValue::SetValue <VRngDigits>,<yVSignDig>"),(VRngDigits),(yVSignDig))
  }
  
  DisplayValue();
}
//-----------------------------------------------------------------------------------------------------
byte SetValue::DigitCount(int _Value, byte _Base) {
DBENTERAAL(("SetValue::DigitCount(Value,Base)"),(_Value),(_Base))

  if ( _Base<=0 ) { DBERRORAAL(("MenuValue::DigitCount(Value,Base) Base<=0"),(_Value),(_Base)) return 0; }
  if ( _Value==0 ) return 1;
  byte Count=0;
  if ( _Base==HEX ) {
    uint16_t Val = uint16_t(_Value);
    while ( Val!=0 ) { Count++; Val/=_Base; }
  } else {
    uint16_t Val = abs(_Value); 
    while ( Val!=0 ) { Count++; Val/=_Base; }
  }
  return Count; 
}
//-----------------------------------------------------------------------------------------------------
const __FlashStringHelper* SetValue::GetNumberName(int _Value) {
  CurrNumberName = FirstNumberName;
  while ( CurrNumberName != 0 ) {
    //DBINFOAL(("MenuValue::GetNumberName Searchfor: "),(CurrNumberName->Number))
    if ( CurrNumberName->Number == _Value ) return CurrNumberName->Name;
    CurrNumberName = CurrNumberName->Next;
  }
  return 0;
}
//-----------------------------------------------------------------------------------------------------
byte SetValue::NavValueSet(byte _Key) {               DBENTERAL(("SetValue::NavValueSet(Key)"),(_Key,HEX))

  // RIGHT & LEFT
  if (_Key==NAVKEYRIGHT && VType&VTDIG ) {                                    // Move Digit Right
    if (VDigitPos>0) { VDigitPos--; DisplayValue(); } 
  } 
  
  if (_Key==NAVKEYLEFT) { 
    if ( VType&VTDIG ) {
      if ( VDigitPos<VRngDigits-1 ) { VDigitPos++; DisplayValue(); }                     //Move Digit Left
      else { VDigitPos = 0; return NAVEXITVALUESET; }    //el| Move to Name
    } else {
      return NAVEXITVALUESET; // NO-SAVE and EXIT
    }
  }

  if ( _Key==NAVKEYUP || _Key==NAVKEYDOWN ) {
    
    //if| PER-DIGIT SET
    if ( VType&VTDIG ) {
      uint16_t base = VType&VTHEX?HEX:DEC;                                          // If (VType is HEX) 16 else 10;
      
      if (base==HEX) {
        DBINFOL(("SetValue::NavValueSet() base==HEX"))
        uint8_t nibble=0;
        nibble = (iSetTo & uint16_t(0xF<<(VDigitPos*4))) >> VDigitPos*4;                //Pulls one nibble-out
        if ( _Key==NAVKEYUP ) { if (nibble==0xF) { nibble=0; } else { nibble++; } }     // +nibble
        if ( _Key==NAVKEYDOWN ) { if (nibble==0x0) { nibble=0xF; } else { nibble--; } } // -nibble
        iSetTo = (iSetTo & ~uint16_t(0xF<<(VDigitPos*4)));                              // ZERO where nibble goes
        iSetTo = iSetTo | uint16_t(nibble << (VDigitPos*4));                            // Plug nibble back-in
        
      } else { // base==DEC
        DBINFOL(("SetValue::NavValueSet() base==DEC"))
        DBINFOAAL(("SetValue::NavValueSet[DEC] <iSetTo><VDigitPos>"),(iSetTo),(VDigitPos))
        DBINFOAAL(("SetValue::NavValueSet[DEC] <bVSigned><yVSignDig>"),(bVSigned),(yVSignDig))
        if ( bVSigned && yVSignDig==(VDigitPos+1) ) {                                   // VDigitPos base [0]
          DBINFOAL(("SetValue::NavValueSet[DEC] SIGN-FLIP <VDigitPos>"),(VDigitPos)) 
          iSetTo = -1*iSetTo; 
        } else {
          uint16_t digatpos = 0;bool IsNeg = (iSetTo<0);                                  // Get digit at position
          if ( VDigitPos == 0 ) { digatpos = (abs(iSetTo)) % base; }
          else { digatpos = (abs(iSetTo)) / (uint16_t(pow(base,VDigitPos))) % base; }           // VDigitPos is where setting
          DBINFOAL(("SetValue::NavValueSet <Digit-At-Pos>"),(digatpos))
          if ( _Key==NAVKEYUP ) {
            if ( digatpos >= base-1 ) { iSetTo = iSetTo - digatpos*pow(base,VDigitPos); }
            else { iSetTo = iSetTo + pow(base,VDigitPos); }
          }      
          if ( _Key==NAVKEYDOWN ) {
            if ( digatpos == 0 ) { iSetTo = iSetTo + (base-1)*pow(base,VDigitPos); }
            else { iSetTo = iSetTo - pow(base,VDigitPos); }
          }
        }
        //if (IsNeg) iSetTo = -(iSetTo);
        DBINFOAAL(("SetValue::NavValueSet else|HEX <iSetTo>,<VDigitPos>"),(iSetTo),(VDigitPos))
      }
    
    //el| NOT-PER-DIGIT SET
    } else {
      if ( _Key==NAVKEYUP ) iSetTo++;
      if ( _Key==NAVKEYDOWN ) iSetTo--;
    }

    // BOUNDARY CHECK
    if ( VType&VTHEX ) { 
      if (uint16_t(iSetTo)>uint16_t(VMax)) iSetTo=VMax;
      if (uint16_t(iSetTo)<uint16_t(VMin)) iSetTo=VMin; 
  
    } else {
      if (iSetTo>VMax) { DBINFOAAL(("SetValue::NavValueSet iSetTo>VMax"),(iSetTo),(VMax)); iSetTo=VMin; }
      if (iSetTo<VMin) { DBINFOAAL(("SetValue::NavValueSet iSetTo<VMin"),(iSetTo),(VMin)); iSetTo=VMax; }
    }
    DisplayValue(); return NAVKEYNONE;
  }
  
  // SET & OKAY
  if ( _Key==NAVKEYSET || _Key==NAVKEYOKAY ) {
    if ( _Key==NAVKEYSET ) { 
      DBINFOL(("SetValue::NavValueSet() return NAVSAVEVALUESET")) 
      return NAVSAVEVALUESET;
    }
    else { 
      iSetTo=VMin;
      DBINFOAL(("SetValue::NavValueSet() return iSetTo=?VMin?"),(iSetTo)) 
      DisplayValue(); return NAVKEYNONE;
    }
    return NAVKEYNONE;
  }
}
//-----------------------------------------------------------------------------------------------------
void SetValue::DisplayValue() {
DBENTERL(("SetValue::DisplayValue()"))
  if ( Display==0 ) { DBERRORL(("SetValue::DisplayValue() Display==0")) return; }
  DBINFOAAL(("SetValue::DisplayValue() VMin,VMax: "),(uint16_t(VMin),HEX),(uint16_t(VMax),HEX))
  
  // Set Cursor Position & Color Scheme
  Display->SETCURSOR(XPos+VALCOL,YPos);                
  Display->BGVST; Display->TXTSET;
  
  const __FlashStringHelper* ValueName = 0;                         // Get Value Name if one assigned
  //if ( !(VType&VTDIG)) { ValueName = GetNumberName(iSetTo); }
  ValueName = GetNumberName(iSetTo);       // ValueName NOT compatible with VTDIG
  if ( ValueName!=0 ) { Display->print(ValueName); return; }        // Display NAMED Value
  
  if ( !(VType&VTDIG ) ) {                                          // NOT in Per-Digit SETVALUE
    if ( VType&VTHEX ) {  Display->print(uint16_t(iSetTo), HEX); }  // Display HEX Value
    else {                Display->print(iSetTo, DEC); }            // Display DEC Value
    return;
  }  

  //vvv[ Digit-Set ]vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  int Digits=0;                                             // Full Width - Digit Pos
  Display->fillRect( XPos+VALCOL+ ( (ITEMCHARWIDTH+2)*VRngDigits - VDigitPos*(ITEMCHARWIDTH+2) - (ITEMCHARWIDTH+2) ), 
                     YPos-ITEMCHARHEIGHT+2, ITEMCHARWIDTH,ITEMCHARHEIGHT,WHITE);
  
  if ( VType&VTHEX ) { Digits = DigitCount(iSetTo,HEX); }   // HEX - Count Up Printable/Assigned Digits
  else {               Digits = DigitCount(iSetTo,DEC); }   // DEC - Count Up Printable/Assigned Digits
  
  if ( bVSigned && !(VType&VTHEX)) {                                                  // Signed DEC
    if ( iSetTo<0 ) { Display->print("-"); } else { Display->print("+"); }            // Print + or -
    if (VRngDigits>0) {for ( int i=Digits; i<(VRngDigits-1); i++ ){ Display->print("0");}}  // Print Leading Zero's
  } else {
    for ( int i=Digits; i<VRngDigits; i++ ) { Display->print("0"); }     // Display Leading Zero's  (?HERE?)
  }
  
  if ( VType&VTHEX ) { Display->print(uint16_t(iSetTo), HEX); }             // Display HEX Value
  else { Display->print(abs(iSetTo), DEC); }                                // Display DEC Value
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^    
}
//#####################################################################################################################
MenuEEValue::MenuEEValue(const __FlashStringHelper* _CName, int _EEValAddress, byte _ValueType):
  MenuValue(_CName,_ValueType) {
  EEValAddress = _EEValAddress;
}
//.....................................................................................................
MenuEEValue::MenuEEValue(int _EENameAddress, bool _NameSettable, int _EEValAddress, byte _ValueType):
  MenuValue(_EENameAddress, _NameSettable, _ValueType) {
  EEValAddress = _EEValAddress;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MenuEEValue::EEClear() {
DBENTERL(("MenuEEValue::EEClear()"))
  if ( VType&VTBYTE ) {
    EEPROM.update(EEValAddress, 0xFF);
    DBINFOAL(("EEPROM.update(EEValAddress, 0xFF)"),(EEValAddress,HEX))
  } else {
    EEPROM.put(EEValAddress, 0xFFFF);
    DBINFOAL(("EEPROM.put(EEValAddress, 0xFFFF)"),(EEValAddress,HEX))
  }
  if ( SubList != 0 ) SubList->DelAllItems(SubList->DestEEClear);
}
//-----------------------------------------------------------------------------------------------------
int MenuEEValue::Value() {                                       
//DBENTERAL(("MenuEEValue::Value[GET] @EE"),(EEValAddress,HEX)) 
  if ( VType&VTBYTE ) { iValue = (int)EEPROM.read(EEValAddress); } 
  else { EEPROM.get(EEValAddress, iValue); }
  return iValue;
}
//-----------------------------------------------------------------------------------------------------
void MenuEEValue::Value(int _Value, byte _Status) {                   
DBENTERAAL(("MenuEEValue::Value[SET](?Value?,Status)@?EE?"),(_Value),(EEValAddress,HEX))
  if ( !(VType&VTRW)) { DBINFOL(("MenuEEValue::Value[SET] BLOCKED Read-Only")) return; }  // Block-Set on Read-Only 
  if ( VType&VTBYTE ) { EEPROM.update(EEValAddress, byte(_Value)); }                      // Write Byte-Value
  else { EEPROM.put(EEValAddress,_Value); }                                               // Write Integer-Value
  iValue = _Value;                                                                        // Use iValue for Integers
}
//_____________________________________________________________________________________________________________________
#endif
