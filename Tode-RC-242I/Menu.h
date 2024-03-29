/******************************************************************************************************************//**
 * @file    Menu.h
 * @brief   Abstract Base Classes for Menu Structure hierarchy
 *          - Top-Level Navigator of MenuList (s)
 *          - MenuList of MenuItem (s)
 *          - MenuItem
 *********************************************************************************************************************/
#ifndef _MENU_H
#define _MENU_H

//#include "lib/DBC.h"
#include "Disp.h"
#include "Ebyte.h"

#define EEMAXTITLESIZE 10
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
  #define NAVEXITVALUESET   0xCB
  #define NAVSAVEVALUESET   0XCA
  #define NAVDELTODE        0xC9
  #define NAVEDITDEV        0xC8
  #define NAVDELDEV         0xC7
  #define NAVSYSSIMPLRST    0xC6
  #define NAVSYSFCTRYRST    0xC5
  #define NAVSYSPUBADISC    0xC4
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
 * @defgroup STS STS Set Status
 *           defined in Menu.h
 * @{
 *********************************************************************************/
#define STSNEVERSET  0xFF     ///< Value hasn't been set
#define STSNORMAL    0xFE     ///< N/A Setting
#define STSUSERSET   0xFD     ///< User-Set Value
#define STSRFSET     0xFC     ///< An RF Rx Packet requests a Value-Set
#define STSEXPIRED   0xFB     ///< Value is time-expired (RF GOT too long ago)
#define STSEEGOT     0xFA     ///< Value was set from EEPROM
#define STSRFGETTING 0xF9     ///< A request for the value was sent by RF
#define STSRFGETFAIL 0xF8     ///< A request for the value was sent by RF but no return.
#define STSRFGOT     0xF7     ///< RF Rx Packet contains new Value to be Set

///@}
/******************************************************************************//**
 * @defgroup DM DM Display Mode
 *           defined in Menu.h
 * @{
 *********************************************************************************/
#define DMNORMAL    0xFF      ///< Display Normal Read Mode
#define DMNAMELIST  0xFE      ///< Display as an Edit List
///@}
/******************************************************************************************************************//**
 * @class   BaseSetMenuName
 * @brief   Used entirely to get an &oDisplay pointer into iLib uninherited classes.
 *********************************************************************************************************************/
class GetDisplay {
  public:
    GetDisplay();            ///< Display Object Pointer);
  protected:
    DSPCLASS*             Display = 0;                                  ///< Display Object Pointer
};
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
    virtual   ~MenuItem();
            byte    bSaveSubList = 0;                         ///< Don't delete SubList in ~MenuItem
            byte    bSaveItem = 0;
    virtual byte    Navigate(byte _Key);                      ///< Keypad Navigate by \ref KEY
    virtual void    DispItem(byte _XPos=0, byte _YPos=0, byte _DispMode = DMNORMAL);     ///< Display the Item
            byte    DispMode = DMNORMAL;
    virtual byte    Loop();                                   ///< System loop()
            byte    NavSelected = SEL_NONE;                   ///< Display focus point
            byte    RFID = 0xFF;                              ///< RFID of Item for RxTx
    virtual byte    Status();
    virtual void    Status(byte _Status);
    virtual void    EEClear();
    MenuList*       SubList = 0;                              ///< Item links->to a Link-List
    RFCLASS*        RF = 0;
    
  protected:
    virtual void    OnDisplay(bool _OnDisplay);
    virtual bool    OnDisplay();
    bool            bOnDisplay = false;
    DSPCLASS*       Display = 0;                              ///< Display Object Pointer
    
    byte            XPos=0, YPos=0;
    byte            yStatus = STSNEVERSET;                    ///< Current Status of this Item
    bool            BuiltSubList = false;                     ///< Used for dynamic load of SubList
    
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
    MenuList(const __FlashStringHelper* _CTitle=0, byte _DispMode = DMNORMAL);                 ///< NEW List optional Title Name
    virtual ~MenuList();
    
    virtual byte          Navigate(byte _Key);                                    ///< Keypad Navigate by \ref KEY
    virtual void          DispList(bool Clear=false, byte _XPos=0, byte _YPos=0); ///< Diplay the List on the Display
    virtual void          OffDisplay();                                           ///< Remove List from the Display
    virtual MenuItem*     Add(MenuItem* _Item);                                   ///< ADD MenuItem to the MenuList
    virtual void          Del(MenuItem* _Item);                                   ///< DEL MenuItem from the MenuList
    virtual void          DelAllItems(bool _EEClear=false);                        ///< DEL ALL MenuItems from the MenuList
    virtual void          Update();                                               ///< Preform updates on Current List
    virtual byte          Loop();                                                 ///< System loop()
    virtual const char*   Title();                                                ///< GET List Title-Name
    
    MenuList*             NextList = 0;
    MenuList*             PrevList = 0;
    MenuItem*             CurrItem = 0;  
    RFCLASS*              RF = 0;
    bool                  DestEEClear=true;
    byte                  TodeIndex = BNONE;                    ///< TodeIndex(0-9) assigned by the Constructor
  
  protected:  
    DSPCLASS*       Display = 0;
    byte            XPos=0, YPos=0, DispMode = DMNORMAL;
    MenuItem*       FirstItem = 0;
    MenuItem*       LastItem = 0;
  
  private:
    const __FlashStringHelper*     CTitle = 0;
    char            FTitle[EEMAXTITLESIZE+1];                 ///< C-Style Name Char[] for Name set/get.    
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
    virtual void        Del(MenuList* _List);     ///< DEL MenuList from the Navigator
    virtual byte        Loop(byte _FinalKey);     ///< System loop()
    RFCLASS*            RF = 0;
    virtual void        EnterSetupMenu();         ///< Called by Navigator and defined in Sys Class
    MenuList*           SetupMenu = 0;            ///< System Setup Menu

  protected:
    MenuList*     FirstList = 0;
    MenuList*     LastList = 0;
    MenuList*     CurrList = 0;
    MenuItem*     SublistEnterItem = 0;
    DSPCLASS*     Display = 0;
    byte          bInSetupMenu = 0;
     
  private:
    byte          bInSubList = 0;
    bool          StartScreen=false;              ///< Initialize Screen ONCE
};
//_____________________________________________________________________________________________________________________
#endif
