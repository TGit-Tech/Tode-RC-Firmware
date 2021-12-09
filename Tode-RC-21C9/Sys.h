/******************************************************************************************************************//**
 * @file Sys.h
 * @brief Top-Level Tode Navigator and System Operations.
 *    - Setup Menu
 *    - Todes List
 *    - Devices List -> AddDevicesList
 *    - Radio Settings ( MenuList )
 * @todo
 *  - Complete the IO-Hdw setting
 *  - Check 'RF' object checkpoints to prevent unknown lock-up
 *  - Add device delete list
 *  - Fix device rename exit procedure
 *  - Fix the way device Value()'s are '-1'
 *  
 * @authors
 *    TGit-Tech          12/2019       Original
 *********************************************************************************************************************/
#ifndef _SYS_H
#define _SYS_H

#include "LHdw.h"
#include "iRF.h"
#define FIRMWARE      "Reset21C9"

/**********************************************************************************************//**
 *  @class  Sys
 *  @brief  **Sys:Navigator** Top-Level System for Setup, Todes & ThisTode's HDW / Device Setup.
 *************************************************************************************************/
class Sys : public Navigator {
  public:
    Sys();

    MenuList*   NewTode(byte _Index);                   ///< Creates a Tode at specified Index or finds one
    byte        NewTode();                              ///< Create a Tode at a free Index
    Tode*       RFTode(unsigned int RFAddress, bool Create = false);    ///< Get Tode pointer by RF Address
    AddTode*    AddATode = 0;                           ///< Menu Item used to Add a Tode Config
    
    MenuList*   SetupMenu = 0;                          ///< Setup the System Menu
    MenuList*   DeviceList = 0;                         ///< ThisTode Device List
    MenuList*   TodesMenu = 0;
    MenuList*   AddDeviceList = 0;                      ///< Add Device drivers for Adding by-menu
    MenuList*   DelTodesList = 0;
    MenuList*   RadioSettings = 0;                      ///< RF Radio Settings on Sublist
    
    byte        Loop(byte _FinalKey) override;          ///< Last Return key for Adding Devices
    Tode*       TodesPtr[10] = {0,0,0,0,0,0,0,0,0,0};   ///< Indexed Pointers
    
    
  protected:
    Tode*         ThisTode = 0;
    int           RFAddr = 0;

  private:
    void          BuildSetupMenu();
    void          RFLoop();
    RadioPCConn*  RFPCConn = 0;
    MenuEEValue*  SecNt = 0;
};

//_____________________________________________________________________________________________________________________
#endif
