/******************************************************************************************************************//**
 * @file    Menu.cpp
 * @brief   Implements MenuList and Navigator
 *********************************************************************************************************************/
#ifndef _MENU_CPP
#define _MENU_CPP
#include "Menu.h" 

//#####################################################################################################################
MenuItem::MenuItem() { this->Display = &oDisplay; }
MenuItem::~MenuItem() { delete(SubList); }
//-----------------------------------------------------------------------------------------------------
byte MenuItem::Status() { return yStatus; }
void MenuItem::Status(byte _Status) { yStatus = _Status; }
//-----------------------------------------------------------------------------------------------------
byte MenuItem::Navigate(byte _Key) { return _Key; }
//-----------------------------------------------------------------------------------------------------
byte MenuItem::Loop() { return 0; }
//-----------------------------------------------------------------------------------------------------
void MenuItem::DispItem(byte _XPos, byte _YPos, byte _DispMode) { 
DBENTERAAL(("MenuItem::DispItem"),(_XPos),(_YPos))
  DispMode = _DispMode;
  if (_XPos!=0) XPos=_XPos; 
  if (_YPos!=0) YPos=_YPos; 
  OnDisplay(true);
}
//-----------------------------------------------------------------------------------------------------
void MenuItem::OnDisplay(bool _OnDisplay) { bOnDisplay = _OnDisplay; }
bool MenuItem::OnDisplay() { return bOnDisplay; }
void MenuItem::EEClear() { }
//#####################################################################################################################
MenuList::MenuList(const char* _CTitle, byte _DispMode) { 
DBENTERL(("MenuList::MenuList"))
  CTitle = _CTitle; 
  this->Display = &oDisplay; 
  DispMode = _DispMode; 
}
//.....................................................................................................
MenuList::~MenuList() {
DBENTERL(("MenuList::~MenuList")) 
  //This gets called in Tode->DelDevice() for Device->Sublist.
  DelAllItems();
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
byte MenuList::Loop() { return 0; }
void MenuList::Update() { return; }
const char* MenuList::Title() { if (CTitle==0) return "?Name?"; return CTitle; }
//-----------------------------------------------------------------------------------------------------
MenuItem* MenuList::Add(MenuItem* _Item) {
DBENTERAL(("MenuList::Add(MenuItem* _Item)"),(int(_Item)))
  
  _Item->RF = this->RF;
  if ( _Item->RF == 0 ) { DBERRORL(("MenuList::Add Item->RF == 0")) }
  
  if ( _Item->SubList != 0 ) {
    _Item->SubList->PrevList = this;
    _Item->SubList->Display = &oDisplay;
    _Item->SubList->RF = this->RF;
  }
  
  if (FirstItem == 0) {
    FirstItem = _Item;
    LastItem = FirstItem;
  } else {
    LastItem->NextItem = _Item;
    LastItem->NextItem->PrevItem = LastItem;
    LastItem = LastItem->NextItem;
  }
  return _Item;
}
//-----------------------------------------------------------------------------------------------------
void MenuList::Del(MenuItem* _Item) {
DBENTERL(("MenuList::Del(MenuItem* _Item)"))
  
  // Find the Item in the Link-List
  MenuItem* ToDel = 0;
  MenuItem* LastItemPtr = 0;
  MenuItem* Current = FirstItem;

  if ( FirstItem == 0 ) return;
  if ( FirstItem == _Item ) { 
    ToDel = FirstItem;
    if ( FirstItem->NextItem != 0 ) { FirstItem = FirstItem->NextItem; } else { FirstItem = 0; }
    delete(ToDel); 
    return; 
  } else {
    while ( Current != 0 ) {
      LastItemPtr = Current;
      Current = Current->NextItem;
      if ( Current == _Item ) {
        DBINFOL(("MenuList::Del Item Found."))
        ToDel = Current;
        LastItemPtr->NextItem = Current->NextItem;            // Close the linkage (crashing right here)
        delete(ToDel);
        break;
      }
    }
  } 

  // Now must find the LastItem
  Current = FirstItem;
  if ( Current == 0 ) { LastItem = 0; return; }
  while ( Current != 0 ) { LastItemPtr = Current; Current = Current->NextItem; }
  LastItem = LastItemPtr;
  return;
}
//-----------------------------------------------------------------------------------------------------
void MenuList::DelAllItems() {
DBENTERL(("MenuList::DelAllItems"))

  CurrItem = FirstItem;
  MenuItem* Next;
  while ( CurrItem != NULL ) {
    Next = CurrItem->NextItem;
    delete(CurrItem);
    CurrItem = Next;
  }
  FirstItem = 0;
  LastItem = 0;
  CurrItem = 0;

}
//-----------------------------------------------------------------------------------------------------
byte MenuList::Navigate(byte _Key) {                        
DBENTERAL(("MenuList::Navigate"),(_Key,HEX))

  // 1. Key Current Item
  if ( Display==0 ) { DBERRORL(("Display==0")) return NAVKEYNONE; }
  if ( CurrItem != 0 ) _Key = CurrItem->Navigate(_Key);             // Key the current Item
  if ( _Key == NAVKEYNONE ) return _Key;                            // If Item used Key exit
  
  // 2. Check for NAVKEYUP ( Previous Item )
  if ( _Key == NAVKEYUP ) {                                 DBINFOL(("MenuList::Navigate NAVKEYUP"))
    if (CurrItem == 0) return _Key;                                                 // Top of List? Exit.
    CurrItem->NavSelected = SEL_NONE; CurrItem->DispItem(0,0,DispMode);             // Reset Current
    CurrItem = CurrItem->PrevItem;                                                  // Move to Previous
    if (CurrItem != 0) { CurrItem->NavSelected = SEL_NAME; CurrItem->DispItem(0,0,DispMode); }  // Show
    else {                                                                          // Refresh Title
      Display->fillRect(0,0,SCREENWIDTH,TITLECHARHEIGHT+6, (CurrItem==0)? BLUE: LIGHTGREY);
      Display->SETCURSOR(2, TITLECHARHEIGHT); Display->TXTNORM;
      Display->TITLEFONT; Display->print(Title());Display->ITEMFONT;
    }
  
  // 3. Check for NAVKEYDOWN ( Next Item )
  } else if ( _Key == NAVKEYDOWN ) {                        DBINFOL(("MenuList::Navigate NAVKEYDOWN"))
    if (CurrItem == 0) {
      CurrItem = FirstItem;                                     // FirstItem
      if (CurrItem != 0) {                                      // Refresh Title
        Display->fillRect(0,0,SCREENWIDTH,TITLECHARHEIGHT+6, (CurrItem==0)? BLUE: LIGHTGREY);
        Display->SETCURSOR(2, TITLECHARHEIGHT); Display->TXTNORM;
        Display->TITLEFONT; Display->print(Title());Display->ITEMFONT;
        CurrItem->NavSelected = SEL_NAME;  
        CurrItem->DispItem(0,0,DispMode);                                                       // Show Item
      }
    } else if (CurrItem->NextItem != 0) {                       // NextItem 
      CurrItem->NavSelected = SEL_NONE; CurrItem->DispItem(0,0,DispMode);
      CurrItem = CurrItem->NextItem;
      CurrItem->NavSelected = SEL_NAME; CurrItem->DispItem(0,0,DispMode);
    }  

  // 4. Check for NAVKEYOKAY
  } else if ( _Key == NAVKEYOKAY ) {
    Update();
    return NAVKEYNONE;
  }

  // 5. Check for NAVDSP-Constants
  if ( _Key==NAVDSPLIST ) { DispList(); return NAVKEYNONE; }
  if ( _Key==NAVDSPNEWLIST ) { DispList(true); return NAVKEYNONE; }
  return _Key;
  
}
//-----------------------------------------------------------------------------------------------------
void MenuList::DispList(bool Clear, byte _XPos, byte _YPos) { 
DBENTERAL(("MenuList::DispList(CLEAR):"),(Clear))
  if ( Display == 0 ) { DBERRORL(("MenuList::DispList Display == 0")) return; }
  if ( Clear ) Display->CLEARSCREEN;
  if ( _XPos != 0 ) XPos = _XPos;
  if ( _YPos != 0 ) YPos = _YPos;
    
  MenuItem* pItem = 0;

  // Display List Title
  Display->fillRect(0,0,SCREENWIDTH,TITLECHARHEIGHT+6, (CurrItem==0)? BLUE: LIGHTGREY);
  Display->SETCURSOR(2, TITLECHARHEIGHT); Display->TXTNORM;
  Display->TITLEFONT; Display->print(Title());Display->ITEMFONT;

  // --- Show EACH Item ---
  Display->ITEMFONT; int i = 0;
  pItem = FirstItem; byte Row = TITLECHARHEIGHT+20;
  while ( pItem != 0 ) {
    DBINFOAL(("MenuList::DispList pItem->DispItem @ Row"),(Row))
    pItem->DispItem(0,Row,DispMode);
    Row = Row + ITEMCHARHEIGHT +2;
    pItem = pItem->NextItem;
  }
}
//-----------------------------------------------------------------------------------------------------
void MenuList::OffDisplay() {                                       DBENTERL(("MenuList::OffDisplay"))
  MenuItem* pItem = FirstItem;
  while ( pItem != 0 ) {
    pItem->OnDisplay(false);
    pItem = pItem->NextItem;
  }
  CurrItem = 0;  
}
//#####################################################################################################################
Navigator::Navigator() { this->Display = &oDisplay; }
//-----------------------------------------------------------------------------------------------------
byte Loop(byte _FinalKey) { return 0; }           ///< System Loop
//-----------------------------------------------------------------------------------------------------
byte Navigator::Navigate(byte _Key) {                     
//DBENTERAL(("Navigator::Navigate"),(_Key,HEX))

  // 1. Initate Screen Once
  if (!StartScreen) { DBINFOL(("Start Screen"))
    if (_Key == NAVKEYNONE) _Key=NAVDSPNEWLIST;
    StartScreen = true;
    Display->DSP_BEGIN;
    Display->DSP_SETROTATION;
    Display->ITEMFONT;
    if (CurrList == 0) CurrList = FirstList;
  }

  // 2. Key Current List
  if (_Key == NAVKEYNONE || CurrList == 0) return _Key;       // Exit on NOKEY or No List
  DBINFOAL(("Navigator::Navigate"),(_Key,HEX))  
  if (CurrList != 0) _Key = CurrList->Navigate(_Key);         // Sent KEY to List

  //--------------------------------------------------------------------------
  // 3. PROCESS RETURNED KEY
  //--------------------------------------------------------------------------
  if ( _Key == NAVGOTOSUBLIST ) {
    MenuList* pList = CurrList;
    if ( CurrList == 0 ) { DBERRORL(("Navigator::Navigate NAVGOTOSUBLIST CurrList == 0")) return NAVKEYNONE; }
    if ( CurrList->CurrItem == 0 ) { DBERRORL(("Navigator::Navigate NAVGOTOSUBLIST CurrList->CurrItem == 0")) return NAVKEYNONE; }
    if ( CurrList->CurrItem->SubList == 0 ) { DBERRORL(("Navigator::Navigate NAVGOTOSUBLIST CurrList->CurrItem->SubList == 0")) return NAVKEYNONE; }
    DBINFOL(("Navigator::Navigate NAVGOTOSUBLIST"))
    SublistEnterItem = CurrList->CurrItem;
    CurrList->CurrItem->SubList->PrevList = CurrList;   // Set Return List ( This did fix SubList Nav )
    CurrList = CurrList->CurrItem->SubList;
    if ( pList != 0 ) pList->OffDisplay();
    CurrList->DispList(true);
    return NAVKEYNONE;
  }
  
  // 4. Check for NAVKEYRIGHT ( NextList )
  else if ( _Key == NAVKEYRIGHT ) {
    if (CurrList->NextList != 0) {
      if (CurrList != 0) CurrList->OffDisplay();
      CurrList = CurrList->NextList;
      CurrList->DispList(true);
      return NAVKEYNONE;
    }
  }
  
  // 5. Check for NAVKEYLEFT ( PrevList )
  else if ( _Key == NAVKEYLEFT ) {
    if (this->CurrList->PrevList != 0) {
      if (CurrList != 0) CurrList->OffDisplay();
      CurrList = CurrList->PrevList;
      CurrList->DispList(true);
      return NAVKEYNONE;
    } else {
      DBINFOL(("Navigator::Navigate NAVKEYLEFT this->CurrList->PrevList == 0"))
    }
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
MenuList* Navigator::Add(MenuList* _List) {
DBENTERL(("Navigator::Add _List"))

  _List->Display = &oDisplay;
  _List->RF = this->RF;
  if ( _List->RF == 0 ) { DBERRORL(("Navigator::Add List->RF == 0")) }
  
  if (FirstList == 0) {
    FirstList = _List;
    LastList = FirstList;
  } else {
    LastList->NextList = _List;
    LastList->NextList->PrevList = LastList;
    LastList = LastList->NextList;
  }
  return _List;
}
//-----------------------------------------------------------------------------------------------------
void Navigator::Del(MenuList* _List) {
DBENTERL(("Navigator::Del _List"))
  
  // Find the Item in the Link-List
  MenuList* LastListPtr = 0;
  MenuList* ThisListPtr = FirstList;
  while ( ThisListPtr != 0 ) {
    if ( ThisListPtr == _List ) {
      if ( LastListPtr == 0 ) { 
        // Delete First Item
        ThisListPtr->PrevList = LastListPtr;
        delete(FirstList);
        FirstList = ThisListPtr;
      } else {
        LastListPtr->NextList = ThisListPtr->NextList;            // Last Item Skips This Item
        ThisListPtr->NextList->PrevList = LastListPtr->PrevList;  // Next Item's Previous is LastItems Previous
        delete(ThisListPtr);
        break;
      }
    }
    LastListPtr = ThisListPtr;
    ThisListPtr = ThisListPtr->NextList;
  }
}
//_____________________________________________________________________________________________________________________
#endif
