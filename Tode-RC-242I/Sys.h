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

#include <Arduino.h>
#include "serial2mqtt.h"

/******************************************************************************//**
 * @defgroup DCSP [DT] Device Control Set Point enumeration
 * @{
 *********************************************************************************/
#define USBSERUSE_NONE    0
#define USBSERUSE_MQTT    1    ///< Less Than or Equal To
#define USBSERUSE_KBC     2
#define USBSERUSE_RFPC    3
///@}
/**********************************************************************************************//**
 *  @class  USBSerialUse
 *  @brief  **Sys:Navigator** Top-Level System for Setup, Todes & ThisTode's HDW / Device Setup.
 *************************************************************************************************/
class USBSerialUse : public MenuEEValue {
  public:
    USBSerialUse();
    virtual int           Value() override;                                                   ///< GET Item Value
    virtual void          Value(int _Value, byte _Status = STSNORMAL) override;               ///< SET Item Value
  private:
    void(* resetFunc) (void) = 0;  // declare reset fuction at address 0
};
/**********************************************************************************************//**
 *  @class  PublishValues
 *  @brief  MQTT publish get values timing system.
 *************************************************************************************************/
class PublishValues{
  public:
    PublishValues(byte _PubTimes=2, unsigned long _OnIntervalMS = 5000);
    void      PubTode(Tode* _pTode);
    bool      ForLoopCount(int _index);
    Tode*     OnTode();
  private:
    byte            PubTimes=0;
    byte            index=0;
    unsigned long   OnIntervalMS=0;
    unsigned long   LastResultMS[5]={0,0,0,0,0};
    byte            PubCount[5]={0,0,0,0,0};
    Tode*           pTode[5]={0,0,0,0,0};
};
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
    
    
    MenuList*   AddDeviceList = 0;                      ///< Add Device drivers for Adding by-menu
    MenuList*   DelTodesList = 0;                       ///< Delete a Remote Tode
    MenuList*   RadioSettings = 0;                      ///< RF Radio Settings on Sublist
    MenuList*   SystemCommands = 0;                     ///< All System Commands
    
    void        EnterSetupMenu() override;
    byte        Loop(byte _FinalKey) override;          ///< Last Return key for Adding Devices
    Tode*       TodesPtr[AEB_MAXTODES] = {0,0,0,0,0,0,0,0,0,0};   ///< Indexed Pointers
    bool        mqttActive=false;
    
  protected:
    Tode*         ThisTode = 0;
    int           RFAddr = 0;

  private:
    void(* resetFunc) (void) = 0;  // declare reset fuction at address 0
    PublishValues*  PubValues = new PublishValues();
    void            RFLoop();
    MenuEEValue*    SecNt = 0;
    MenuName*       AddDeviceItem = 0;
    void            MQTTLoop();
    void            PubHomieMQTT();
    serial2mqtt*    mqtt=0;
    bool            mqttOnline = false;
    String          mqttForm(String _string);
    bool            USBSerialeqRFPC=false;
};

//_____________________________________________________________________________________________________________________
#endif
