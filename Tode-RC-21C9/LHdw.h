/******************************************************************************************************************//**
 * @file    LHdw.h
 * @brief   **LIST** Extends MenuList for a Tode and it's Devices(s)
 *********************************************************************************************************************/
#ifndef _LHDW_H
#define _LHDW_H

#include "iDev.h"
/******************************************************************************************************************//**
 * @class   Tode
 * @brief   **LIST** represents a Physical Tode with a list of Device (s)
 *********************************************************************************************************************/
class Tode : public MenuList {
  public:
    Tode(byte _TodeIndex);                                      ///< Constructor Index(0-9) eqv-to. EEPROM Address of
    
    virtual const char*   Title() override;                     ///< Points to TodeName->Name()
    virtual void          Update() override;                    ///< Update the Tode Information
    
    virtual MenuItem*     NewDevice(byte _DTKey);               ///< Creates & Adds a New Device
    virtual MenuItem*     AddDevice(byte _DTKey, byte _RFID);   ///< Menu add Device using \ref KEY
    virtual void          EELoadDevices();
    
    virtual unsigned int  RFAddr();                             ///< GET RF-Address stored in EEPROM
    virtual void          RFAddr(unsigned int _RFAddr);         ///< SET RF-Address stored in EEPROM
    virtual byte          Version();                            ///< GET Tode's Configuration Ver
    virtual void          Version(byte _Version);               ///< SET Tode's Configuration Ver
    virtual bool          IsLocal();                            ///< GET (TodeIndex == 0) Set in the Constructor
    virtual int           EEAddress();                          ///< GET EEPROM Address of This Tode (Calc by TodeIndex)
            byte          TodeIndex = BNONE;                    ///< TodeIndex(0-9) assigned by the Constructor
            
    MenuName*             TodeName=0;                           ///< TodeName Menu-Item
    Device*               Devices[AEB_MAXDEVICES] = {0};        ///< Device Pointers - Index is RFID
    HdwSelect*            Hardware = 0;                         ///< This Tode's Hardware Attached Menu-Item
    
    
  protected:
    
    
  private:
    bool            bIsLocal = false;
    byte            yVersion = 0;

};
/******************************************************************************************************************//**
 * @class   TodeList
 * @brief   **LIST** A list of the Todes for deleting
 *********************************************************************************************************************/
class TodeList : public MenuList {
  public:
    TodeList();
    
};
//_____________________________________________________________________________________________________________________
#endif
