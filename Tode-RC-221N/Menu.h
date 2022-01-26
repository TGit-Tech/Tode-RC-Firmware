/******************************************************************************************************************//**
 * @file    Menu.h
 * @brief   Abstract Base Classes for Menu Structure hierarchy
 *          - Top-Level Navigator of MenuList (s)
 *          - MenuList of MenuItem (s)
 *          - MenuItem
 *********************************************************************************************************************/
#ifndef _MENU_H
#define _MENU_H

#include "lib/ABC.h"
#include "Disp.h"
#include <EEPROM.h>
#include "E32.h"

/******************************************************************************//**
 * @defgroup NAV NAV = Navigate() Constants
 *           Defined in Menu.h
 * @{
 *********************************************************************************/
  /******************************************************************************//**
  * @defgroup NAVKEY NAVKEY = Keypad Navigate() Constants
  * @{
  *********************************************************************************/
  // 0x[F]-Key Presses
  #define NAVKEYNONE        0xFF
  #define NAVKEYRIGHT       0xFE
  #define NAVKEYLEFT        0xFD
  #define NAVKEYUP          0xFC
  #define NAVKEYDOWN        0xFB
  #define NAVKEYSET         0xFA
  #define NAVKEYOKAY        0xF9
  ///@}
  /******************************************************************************//**
  * @defgroup NAVDSP NAVDSP = Display Navigate() Constants
  * @{
  *********************************************************************************/
  // 0x[D]isplay Control
  #define NAVDSPNEWLIST     0xDF    ///< [C]lear & Show MenuList on Display
  #define NAVDSPLIST        0xDE    ///< Show MenuList on Display
  #define NAVDSPNEWNAMESET  0xDD    ///< [C]lear & Show the Name[SET]Tool on Display
  #define NAVDSPNAMESET     0xDC    ///< Show the Name[SET]Tool on Display
  #define NAVDSPROW         0xDB    ///< Show a single List-Row on the Display
  ///@}
  /******************************************************************************//**
  * @defgroup NAVCMD NAVCMD = Command Navigate() Constants
  * @{
  *********************************************************************************/  
  // 0x[C]ommand Control
  #define NAVGOTOSUBLIST    0xCF    ///< Activated in NavName() of MenuName
  #define NAVEXITNAMESET    0xCE
  #define NAVSAVENAMESET    0xCD
  #define NAVDELTODE        0xCB
  ///@}
///@}
/******************************************************************************//**
 * @defgroup SEL SEL = Item-Element 'NavSelected'
 *           defined in Menu.h
 * @{
 *********************************************************************************/
#define SEL_NONE      0       ///< The Item is NOT NavSelected in a List
#define SEL_NAME      1       ///< NAME element is NavSelected in a List
#define SEL_VALUE     2       ///< VALUE element is NavSelected in a List
#define SEL_SETNAME   3       ///< The NAME element is being Set
#define SEL_SETVALUE  4       ///< The VALUE element is being Set in a List
///@}
/******************************************************************************//**
 * @defgroup STS STS Status
 *           defined in Menu.h
 * @{
 *********************************************************************************/
#define STSNEVERSET  0xFF     ///< Value hasn't been set
#define STSNORMAL    0xFE     ///< N/A Setting
#define STSUSERSET   0xFD     ///< User-Set Value
#define STSEXPIRED   0xFD     ///< Value is time-expired (RF GOT too long ago)
#define STSEEGOT     0xFC     ///< Value was set from EEPROM
#define STSRFGETTING 0xFB     ///< A request for the value was sent by RF
#define STSRFSET     0xFA     ///< An RF Rx Packet requests a Value-Set
#define STSRFGOT     0xF9     ///< RF Rx Packet contains new Value to be Set
///@}
//#####################################################################################################################
class MenuList; // Forward Declare
/******************************************************************************************************************//**
 * @class   MenuItem
 * @brief   **ITEM** Menu-Item Base Class ie *Interface*.
 *          1.  Object 'Display' is available in every Item assigned by Constructor.
 *          Each Item-Type can rely on 'Display' being defined but each Item must contain
 *          its own implementation of Navigate() and corresponding Display methods.
 *          
 *********************************************************************************************************************/
class MenuItem {
  friend class MenuList;
  friend class Navigator;
  
  public:
    MenuItem();
    virtual byte    Navigate(byte _Key);                      ///< Keypad Navigate by \ref KEY
    virtual void    DispItem(byte _XPos=0, byte _YPos=0);     ///< Display the Item
    virtual byte    Loop();                                   ///< System loop()
            byte    NavSelected = SEL_NONE;                   ///< Display focus point
            byte    RFID = 0xFF;                              ///< RFID of Item for RxTx
    virtual byte    Status();
    virtual void    Status(byte _Status);
    MenuList*       SubList = 0;                              ///< Item links->to a Link-List
  
  protected:
    virtual void    OnDisplay(bool _OnDisplay);
    virtual bool    OnDisplay();
    bool            bOnDisplay = false;
    DSPCLASS*       Display = 0;                              ///< Display Object Pointer
    RFCLASS*        RF = 0;
    byte            XPos=0, YPos=0;
    byte            bStatus = STSNEVERSET;                    ///< Current Status of this Item
    
  private:
    MenuItem*       NextItem = 0;                             ///< Item link->to Next Item Link-List-Pointer
    MenuItem*       PrevItem = 0;                             ///< Item link->to Prev Item Link-List-Pointer
        
};
//#####################################################################################################################
/******************************************************************************************************************//**
 * @class  MenuList
 * @brief  **LIST** of Menu-Item (s) which can have a Title() name.
 *********************************************************************************************************************/
class MenuList {
  friend class Navigator;
  
  public:
    MenuList(const char* _CTitle=0);                                              ///< NEW List optional Title Name
    
    virtual byte          Navigate(byte _Key);                                    ///< Keypad Navigate by \ref KEY
    virtual void          DispList(bool Clear=false, byte _XPos=0, byte _YPos=0); ///< Diplay the List on the Display
    virtual void          OffDisplay();                                           ///< Remove List from the Display
    virtual MenuItem*     Add(MenuItem* _Item);                                   ///< ADD MenuItem to the MenuList
    virtual void          Del(MenuItem* _Item);                                   ///< DEL MenuItem from the MenuList
    virtual void          DelAllItems();                                          ///< DEL ALL MenuItems from the MenuList
    virtual void          Update();                                               ///< Preform updates on Current List
    virtual byte          Loop();                                                 ///< System loop()
    virtual const char*   Title();                                                ///< GET List Title-Name
    
    MenuList*             NextList = 0;
    MenuList*             PrevList = 0;
      
  protected:
    RFCLASS*        RF = 0;
    DSPCLASS*       Display = 0;
    byte            XPos=0, YPos=0;
    MenuItem*       FirstItem = 0;
    MenuItem*       LastItem = 0;
    MenuItem*       CurrItem = 0;
  
  private:
    const char*     CTitle = 0;
    
};
//#####################################################################################################################
/******************************************************************************************************************//**
 * @class  Navigator
 * @brief  **LIST of LISTS** Top-Level Navigator of multiple MenuList (s).
 *          Navigation is preformed by sending keypad pressed to the Top-Level Navigator.  The key
 *          will undergo a round-robin calling sequence to List-Navigate then Item-Navigate 
 *          objects.  If the object has \ref SEL the key is used by said object and a returning
 *          *command-type* non-keypad key is returned.  See \ref KEY
 *********************************************************************************************************************/
class Navigator {
  
  public:
    Navigator();                                  ///< Constructor sets Display = oDisplay
    virtual byte        Navigate(byte _Key);      ///< Keypad Navigate by \ref KEY
    virtual MenuList*   Add(MenuList* _List);     ///< ADD MenuList to the Navigator
    virtual void        Del(MenuList* _List);
    virtual byte        Loop(byte _FinalKey);     ///< System loop()

  protected:
    MenuList*     FirstList = 0;
    MenuList*     LastList = 0;
    MenuList*     CurrList = 0;
    RFCLASS*      RF = 0;
    DSPCLASS*     Display = 0;
     
  private:
    byte          bInSubList = 0;
    bool          StartScreen=false;              ///< Initialize Screen ONCE
};
//_____________________________________________________________________________________________________________________
#endif
