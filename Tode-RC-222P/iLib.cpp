/******************************************************************************************************************//**
 * @file    MenuLib.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _ILIB_CPP
#define _ILIB_CPP

#include "iLib.h" 

//#####################################################################################################################
MenuName::MenuName(const char* _CName, MenuList* _SubList):
MenuItem() {
DBINITAAL(("MenuName::MenuName(CName,MenuList)"),(_CName),(int(_SubList),HEX))
  CName = _CName; 
  SubList = _SubList;
}
//.....................................................................................................
MenuName::MenuName(const char* _CName, byte _ReturnKey):
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
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char* MenuName::Name() {                                        
DBENTERL(("MenuName::Name[GET]"))
  if (CName != 0) return CName;
  if ( EENameAddress >= 0 ) { 
    DBINFOAL(("EEPROM.get(?EENameAddress?, FName)"),(EENameAddress,HEX))
    for (int i=0;i<EEMAXNAMESIZE;i++) { FName[i] = ' '; }       // Clean get array 'EEFName'
    EEPROM.get(EENameAddress, FName);                           // Get characters
    for (int i=0;i<EEMAXNAMESIZE;i++) {                         // Copy characters from get array...
      FNameTrm[i]=FName[i];                                     // To larger (by+1) 'FNameTrm' transfer array.
      //DBINFOAAL(("EEFName[i]"),(i),(EEFName[i]))
    }
    FNameTrm[EEMAXNAMESIZE]='\0';                               // Add Terminator in +1 FName
    return FNameTrm;
  }
  DBERRORAL(("CName==0 and ?EENameAddress?<0 at : "),(EENameAddress)) 
  return "?Name?";
}
//-----------------------------------------------------------------------------------------------------
void MenuName::Name(const char* _Name) {                  
DBENTERAL(("MenuName::Name[SET] = "),(_Name))
  if ( CName != 0 ) { CName = _Name; return; }
  if ( EENameAddress < 0 ) { DBERRORL(("EENameAddress<0")) return; }

  for ( int i=0; i<EEMAXNAMESIZE; i++) { 
    if (i>strlen(_Name)) { FName[i]=0xFF; }
    else { FName[i] = _Name[i]; }
  }
  EEPROM.put(EENameAddress, FName);
}
//-----------------------------------------------------------------------------------------------------
byte MenuName::Navigate(byte _Key) {                 DBENTERAL(("MenuName::Navigate(key):"),(_Key,HEX))

  // SELECTOR
  if ( NavSelected == SEL_NONE ) { _Key = NavNone(_Key); }
  if ( NavSelected == SEL_NAME ) { _Key = NavName(_Key); }
  if ( NavSelected == SEL_SETNAME ) { _Key = NavNameSet(_Key); }
  if ( _Key == NAVEXITNAMESET ) { 
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
  if ( _Key == NAVKEYSET && NameSettable ) { NavSelected = SEL_SETNAME; DisplayNameSet(); return NAVKEYNONE; }
  else if ( _Key == NAVKEYOKAY ) {
    if ( SubList != 0 ) { DBINFOL(("MenuName::NavName NAVGOTOSUBLIST")) return NAVGOTOSUBLIST; }
    if ( ReturnKey != NAVKEYNONE ) return ReturnKey;
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
void MenuName::DisplayName() {
//DBENTERAL(("MenuName::DisplayName"),(Name()))
  if ( Display==0 ) { DBERRORL(("MenuName::DisplayName Display==0")) return; }
  Display->SETCURSOR(XPos,YPos);
  if ( NavSelected==SEL_NAME ) { Display->BGNAMESELT; Display->TXTSELT; }
  else { Display->BGNAMENORM; Display->TXTNORM; }
  Display->print(Name());
  if ( EENameAddress >= 0 && DispMode == DMNORMAL) { Display->SETCURSOR(VALCOL,YPos); Display->print("[NAME]"); }
}
//-----------------------------------------------------------------------------------------------------
byte MenuName::NavNameSet(byte _Key) {                   
//DBENTERAL(("MenuName::NavNameSet"),(_Key,HEX))

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
    else if ( cSetTo==NSSAVE ) { Name(SetToName); return NAVKEYNONE; }
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
void MenuName::DisplayNameSet() {
  
  if ( Display==0 ) { DBERRORL(("Display==0")) return 0; }
  byte chars_per_row = SCREENWIDTH / (ITEMCHARWIDTH+2);
  
  if ( !NameSetOnScreen ) { 
    Display->fillScreen(0x0000); NameSetOnScreen = true;                // Clear Screen, Mark Displayed 
    const char* CurrentName = Name();                                   // Get Current Name
    for ( int i=0; i<EEMAXNAMESIZE; i++ ) { SetToName[i] = ' '; }       // Clear 'SetToName' and load-it
    for ( int i=0; i<strlen(CurrentName); i++ ) { SetToName[i] = CurrentName[i]; }
  }

  for ( int i=28; i<127; i++ ) { cSetTo=i; DspChar(); }                 // Display ALL Chars
  cSetTo=NSTITLE;
}
//-----------------------------------------------------------------------------------------------------
void MenuName::DspSetToName(bool _Selected) {
  Display->SETCURSOR(0, 28);
  Display->setTextColor(BLACK);
  Display->fillRect(0,28-ITEMCHARHEIGHT+2,(ITEMCHARWIDTH+1)*EEMAXNAMESIZE,ITEMCHARHEIGHT, _Selected ? BLUE: ORANGE);  //Name BG
  Display->fillRect( SetIdx*(ITEMCHARWIDTH+1), 28-ITEMCHARHEIGHT+2, ITEMCHARWIDTH+2,ITEMCHARHEIGHT,WHITE);            //Per-Char
  Display->print(SetToName);  
}
//-----------------------------------------------------------------------------------------------------
void MenuName::DspChar(bool _Selected) {
  
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

//#####################################################################################################################
MenuValue::MenuValue(const char* _CName, byte _ValueType):
  MenuName(_CName) { 
    VType = _ValueType; 
    if ( VType&VTBYTE ) { VMin=0x00; VMax=0xFF; } 
}
//.....................................................................................................
MenuValue::MenuValue(const char* _CName, byte _ValueType, byte _ReturnKey):
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
  else if ( NavSelected == SEL_SETNAME ) {  _Key = NavNameSet(_Key); }
  else if ( NavSelected == SEL_VALUE ) {    _Key = NavValue(_Key); }
  else if ( NavSelected == SEL_SETVALUE ) { _Key = NavValueSet(_Key); }
  
  // Return Key Processing
  if ( _Key == NAVEXITNAMESET ) { 
    DBINFOL(("MenuName::Navigate Key == NAVEXITNAMESET"))
    NavSelected=SEL_NAME; 
    return NAVDSPNEWLIST; 
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
    iSetTo=Value(); NavSelected=SEL_SETVALUE; 
    DispItem(); return NAVKEYNONE;
  } else if ( _Key==NAVKEYLEFT ) {
    NavSelected=SEL_NAME;
    DispItem(); return NAVKEYNONE;
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
byte MenuValue::NavValueSet(byte _Key) {               DBENTERAL(("MenuValue::NavValueSet"),(_Key,HEX))

  // RIGHT & LEFT
  if (_Key==NAVKEYRIGHT && VType&VTDIG ) {                                    // Move Digit Right
    if (VDigitPos>0) { VDigitPos--; DisplayValue(); } 
  } 
  
  if (_Key==NAVKEYLEFT) { 
    if ( VType&VTDIG ) {
      if ( VDigitPos<VDigits-1 ) { VDigitPos++; DisplayValue(); }                     //Move Digit Left
      else { NavSelected=SEL_NAME; VDigitPos = 0; DispItem(); return NAVKEYNONE; }    //el| Move to Name
    } else {
      NavSelected=SEL_NAME; DispItem(); return NAVKEYNONE; // NO-SAVE and EXIT
    }
  }

  if ( _Key==NAVKEYUP || _Key==NAVKEYDOWN ) {
    //if| PER-DIGIT SET
    if ( VType&VTDIG ) {
      uint16_t base = VType&VTHEX?HEX:DEC;                                          // If (VType is HEX) 16 else 10;
      if (base==HEX) {
        uint8_t nibble=0;
        nibble = (iSetTo & uint16_t(0xF<<(VDigitPos*4))) >> VDigitPos*4;                //Pulls one nibble-out
        if ( _Key==NAVKEYUP ) { if (nibble==0xF) { nibble=0; } else { nibble++; } }     // +nibble
        if ( _Key==NAVKEYDOWN ) { if (nibble==0x0) { nibble=0xF; } else { nibble--; } } // -nibble
        iSetTo = (iSetTo & ~uint16_t(0xF<<(VDigitPos*4)));                              // ZERO where nibble goes
        iSetTo = iSetTo | uint16_t(nibble << (VDigitPos*4));                            // Plug nibble back-in
        
      } else {
        uint16_t digatpos = uint16_t(uint16_t(iSetTo)/(pow(base,VDigitPos)))%base;    // VDigitPos is where setting
        if ( _Key==NAVKEYUP ) {
          if ( digatpos >= base-1 ) { iSetTo = iSetTo - digatpos*pow(base,VDigitPos); }
          else { iSetTo = iSetTo + pow(base,VDigitPos); }      
        }
        if ( _Key==NAVKEYDOWN ) {
          if ( digatpos == 0 ) { iSetTo = iSetTo + (base-1)*pow(base,VDigitPos); }
          else { iSetTo = iSetTo - pow(base,VDigitPos); }
        }
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
      if (iSetTo>VMax) iSetTo=VMin; 
      if (iSetTo<VMin) iSetTo=VMax;
    }
    DisplayValue(); return NAVKEYNONE;
  }
  
  // SET & OKAY
  if ( _Key==NAVKEYSET ) { NavSelected=SEL_VALUE; Value(iSetTo, STSUSERSET); DispItem(); return NAVKEYNONE; }  // SAVE and EXIT
  if ( _Key==NAVKEYOKAY ) { NavSelected=SEL_VALUE; DispItem(); return NAVKEYNONE; }                // NO-SAVE and EXIT
}
//-----------------------------------------------------------------------------------------------------
void MenuValue::DisplayValue() {
//DBENTERL(("MenuValue::DisplayValue()"))
  if ( Display==0 ) { DBERRORL(("MenuValue::DisplayValue Display==0")) return; }

  // Set Cursor Position & Color Scheme
  Display->SETCURSOR(XPos+VALCOL,YPos);                
  if ( NavSelected==SEL_VALUE ) { Display->BGVALSELT; Display->TXTSELT; }
  else if ( NavSelected==SEL_SETVALUE )  { Display->BGVST; Display->TXTSET; }
  else { Display->BGVALNORM; Display->TXTNORM; }

  int TVal = (NavSelected==SEL_SETVALUE) ? iSetTo : Value();    // SetTo or Value(GET)
  const char* ValueName = 0;                                    // Get Value Name if one assigned
  //DBINFOAAL(("MenuValue::DisplayValue VMin,VMax: "),(uint16_t(VMin),HEX),(uint16_t(VMax),HEX))

  if ( VType&VTDIG ) {
    if ( VDigits==0 ) {
      if ( VType&VTHEX ) {                        // Count Digits in Range
        uint16_t i = uint16_t(VMax);
        while ( i!=0 ) { VDigits++; i/= HEX; }
      } else { 
        int i = VMax; 
        while ( i!=0 ) {VDigits++; i/= DEC; }
      }
    }
    DBINFOAL(("MenuValue::DisplayValue VTDIG Digits = "),(VDigits))
  }
  
  if ( !(VType&VTDIG)) { ValueName = GetNumberName(TVal); }    // ValueName NOT compatible with VTDIG

  // Print
  if ( ValueName==0 ) {

    // Show ALL digits and selection box during Digit-Set
    if ( VType&VTDIG && NavSelected==SEL_SETVALUE ) {
      int Digits=0;                     // Full Width - Digit Pos
      Display->fillRect( XPos+VALCOL+ ( (ITEMCHARWIDTH+2)*VDigits - VDigitPos*(ITEMCHARWIDTH+2) - (ITEMCHARWIDTH+2) ), 
                         YPos-ITEMCHARHEIGHT+2, ITEMCHARWIDTH,ITEMCHARHEIGHT,WHITE);

      
      if ( VType&VTHEX ) { uint16_t V = uint16_t(TVal); do { Digits++; V/=HEX; } while ( V!=0); }
      else { int V = TVal; do { Digits++; V/=DEC; } while ( V!=0 ); }
      for ( int i=Digits; i<VDigits; i++ ) { Display->print("0"); }                       // Display Leading Zero's
    }
    if ( VType&VTHEX ) { Display->print(uint16_t(TVal), HEX); }                           // Display HEX Value
    else { Display->print(TVal, DEC); }                                                   // Display DEC Value
  } else {  Display->print(ValueName); }                                                  // Display NAMED Value

  // Per-Digit Box

}
//-----------------------------------------------------------------------------------------------------
void MenuValue::SetNumberName(int _Value, const char* _NName) {
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
const char* MenuValue::GetNumberName(int _Value) {
  CurrNumberName = FirstNumberName;
  while ( CurrNumberName != 0 ) {
    //DBINFOAL(("MenuValue::GetNumberName Searchfor: "),(CurrNumberName->Number))
    if ( CurrNumberName->Number == _Value ) return CurrNumberName->Name;
    CurrNumberName = CurrNumberName->Next;
  }
  return 0;
}
//#####################################################################################################################
MenuEEValue::MenuEEValue(const char* _CName, int _EEValAddress, byte _ValueType):
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
  if ( SubList != 0 ) SubList->DelAllItems(true);
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
//DBENTERAAL(("MenuEEValue::Value[SET](?Value?,Status)@?EE?"),(_Value),(EEValAddress,HEX))
  if ( !(VType&VTRW)) return;                                       // Block-Set on Read-Only 
  
  if ( VType&VTBYTE ) { EEPROM.update(EEValAddress, byte(_Value)); }    // Write Byte-Value
  else { EEPROM.put(EEValAddress,iValue); }                             // Write Integer-Value
  iValue = _Value;                                                      // Use iValue for Integers
}
//_____________________________________________________________________________________________________________________
#endif
