/******************************************************************************************************************//**
 * @file    Menu.cpp
 * @brief   Implements MenuList and Navigator
 *********************************************************************************************************************/
#ifndef _MENU_CPP
#define _MENU_CPP
#include "Menu.h" 

//#####################################################################################################################
MenuItem::MenuItem() { this->Display = &oDisplay; }
//-----------------------------------------------------------------------------------------------------
byte MenuItem::Status() { return bStatus; }
void MenuItem::Status(byte _Status) { bStatus = _Status; }
//-----------------------------------------------------------------------------------------------------
byte MenuItem::Navigate(byte _Key) { return _Key; }
//-----------------------------------------------------------------------------------------------------
byte MenuItem::Loop() { return 0; }
//-----------------------------------------------------------------------------------------------------
void MenuItem::DispItem(byte _XPos, byte _YPos) { 
  if (_XPos!=0) XPos=_XPos; 
  if (_YPos!=0) YPos=_YPos; 
  OnDisplay(true); 
}
//-----------------------------------------------------------------------------------------------------
void MenuItem::OnDisplay(bool _OnDisplay) { bOnDisplay = _OnDisplay; }
bool MenuItem::OnDisplay() { return bOnDisplay; }
//#####################################################################################################################
MenuList::MenuList(const char* _CTitle) { CTitle = _CTitle; this->Display = &oDisplay; }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
byte MenuList::Loop() { return 0; }
void MenuList::Update() { return; }
const char* MenuList::Title() { if (CTitle==0) return "?Name?"; return CTitle; }
//-----------------------------------------------------------------------------------------------------
MenuItem* MenuList::Add(MenuItem* _Item) {
  
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

  // Find the Item in the Link-List
  MenuItem* LastItemPtr = 0;
  MenuItem* ThisItemPtr = FirstItem;
  while ( ThisItemPtr != 0 ) {
    if ( ThisItemPtr == _Item ) {
      if ( LastItemPtr == 0 ) { 
        ThisItemPtr->PrevItem = LastItemPtr;
        delete(FirstItem);                                        // Delete First Item
        FirstItem = ThisItemPtr;
      } else {
        LastItemPtr->NextItem = ThisItemPtr->NextItem;            // Last Item Skips This Item
        ThisItemPtr->NextItem->PrevItem = LastItemPtr->PrevItem;  // Next Item's Previous is LastItems Previous
        delete(ThisItemPtr);
        break;
      }
    }
    LastItemPtr = ThisItemPtr;
    ThisItemPtr = ThisItemPtr->NextItem;
  }
  
}
//-----------------------------------------------------------------------------------------------------
void MenuList::DelAllItems() {
  CurrItem = FirstItem;
  MenuItem* Next;
  while ( CurrItem != NULL ) {
    Next = CurrItem->NextItem;
    free(CurrItem);
    CurrItem = Next;
  }
  FirstItem = 0;
  LastItem = 0;
  CurrItem = 0;
}
//-----------------------------------------------------------------------------------------------------
byte MenuList::Navigate(byte _Key) {                        DBENTERAL(("MenuList::Navigate"),(_Key,HEX))

  if ( Display==0 ) { DBERRORL(("Display==0")) return NAVKEYNONE; }
  if ( CurrItem != 0 ) _Key = CurrItem->Navigate(_Key);             // Key the current Item
  if ( _Key == NAVKEYNONE ) return _Key;                            // If Item used Key exit
  
  if ( _Key == NAVKEYUP ) {                                 DBINFOL(("MenuList::Navigate NAVKEYUP"))
    if (CurrItem == 0) return _Key;                                                 // Top of List? Exit.
    CurrItem->NavSelected = SEL_NONE; CurrItem->DispItem();                         // Reset Current
    CurrItem = CurrItem->PrevItem;                                                  // Move to Previous
    if (CurrItem != 0) { CurrItem->NavSelected = SEL_NAME; CurrItem->DispItem(); }  // Show
    else {                                                                          // Refresh Title
      Display->fillRect(0,0,SCREENWIDTH,TITLECHARHEIGHT+6, (CurrItem==0)? BLUE: LIGHTGREY);
      Display->SETCURSOR(2, TITLECHARHEIGHT); Display->TXTNORM;
      Display->TITLEFONT; Display->print(Title());Display->ITEMFONT;
    }
  
  } else if ( _Key == NAVKEYDOWN ) {                        DBINFOL(("MenuList::Navigate NAVKEYDOWN"))
    if (CurrItem == 0) {
      CurrItem = FirstItem;                                     // FirstItem
      if (CurrItem != 0) {                                                      // Refresh Title
        Display->fillRect(0,0,SCREENWIDTH,TITLECHARHEIGHT+6, (CurrItem==0)? BLUE: LIGHTGREY);
        Display->SETCURSOR(2, TITLECHARHEIGHT); Display->TXTNORM;
        Display->TITLEFONT; Display->print(Title());Display->ITEMFONT;
        CurrItem->NavSelected = SEL_NAME;  
        CurrItem->DispItem();                                                       // Show Item
      }
    } else if (CurrItem->NextItem != 0) {                       // NextItem 
      CurrItem->NavSelected = SEL_NONE; CurrItem->DispItem();
      CurrItem = CurrItem->NextItem;
      CurrItem->NavSelected = SEL_NAME; CurrItem->DispItem();
    }   
  } else if ( _Key == NAVKEYOKAY ) {
    Update();
    return NAVKEYNONE;
  }
  
  //------------------------------------------------------
  if ( _Key==NAVDSPLIST ) { DispList(); return NAVKEYNONE; }
  if ( _Key==NAVDSPNEWLIST ) { DispList(true); return NAVKEYNONE; }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
void MenuList::DispList(bool Clear, byte _XPos, byte _YPos) { DBENTERAL(("MenuList::DispList(CLEAR):"),(Clear))
  if ( Clear ) Display->CLEARSCREEN;
  if ( _XPos != 0 ) XPos = _XPos;
  if ( _YPos != 0 ) YPos = _YPos;
    
  MenuItem* pItem = 0;

  // Display List Title
  Display->fillRect(0,0,SCREENWIDTH,TITLECHARHEIGHT+6, (CurrItem==0)? BLUE: LIGHTGREY);
  Display->SETCURSOR(2, TITLECHARHEIGHT); Display->TXTNORM;
  Display->TITLEFONT; Display->print(Title());Display->ITEMFONT;

  // --- Show EACH Item ---
  Display->ITEMFONT;
  pItem = FirstItem; byte Row = TITLECHARHEIGHT+20;
  while ( pItem != 0 ) {
    pItem->DispItem(0,Row);
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

  if (!StartScreen) { DBINFOL(("Start Screen"))
    if (_Key == NAVKEYNONE) _Key=NAVDSPNEWLIST;
    StartScreen = true;
    Display->DSP_BEGIN;
    Display->DSP_SETROTATION;
    Display->ITEMFONT;
    if (CurrList == 0) CurrList = FirstList;
  }
  if (_Key == NAVKEYNONE || CurrList == 0) return _Key;       // Exit on NOKEY or No List
  DBINFOAL(("Navigator::Navigate"),(_Key,HEX))  
  if (CurrList != 0) _Key = CurrList->Navigate(_Key);         // Sent KEY to List

  //--------------------------------------------------------------------------
  // PROCESS RETURNED KEY (Sublist needs fixed)
  //--------------------------------------------------------------------------
  if ( _Key == NAVGOTOSUBLIST ) {
    MenuList* pList = CurrList;
    if ( CurrList == 0 ) { DBERRORL(("Navigator::Navigate NAVGOTOSUBLIST CurrList == 0")) return NAVKEYNONE; }
    if ( CurrList->CurrItem == 0 ) { DBERRORL(("Navigator::Navigate NAVGOTOSUBLIST CurrList->CurrItem == 0")) return NAVKEYNONE; }
    if ( CurrList->CurrItem->SubList == 0 ) { DBERRORL(("Navigator::Navigate NAVGOTOSUBLIST CurrList->CurrItem->SubList == 0")) return NAVKEYNONE; }
    DBINFOL(("Navigator::Navigate NAVGOTOSUBLIST"))
    CurrList = CurrList->CurrItem->SubList;
    if ( pList != 0 ) pList->OffDisplay();
    CurrList->DispList(true);
    return NAVKEYNONE;
  }
  else if ( _Key == NAVKEYRIGHT ) {
    if (CurrList->NextList != 0) {
      if (CurrList != 0) CurrList->OffDisplay();
      CurrList = CurrList->NextList;
      CurrList->DispList(true);
      return NAVKEYNONE;
    }
  }
  else if ( _Key == NAVKEYLEFT ) {
    if (this->CurrList->PrevList != 0) {
      if (CurrList != 0) CurrList->OffDisplay();
      CurrList = CurrList->PrevList;
      CurrList->DispList(true);
      return NAVKEYNONE;
    }
  }
  return _Key;
}
//-----------------------------------------------------------------------------------------------------
MenuList* Navigator::Add(MenuList* _List) {

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
