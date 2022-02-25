/*! \mainpage Main Page
 * 
 * \section Files Files & Abreviations
 *  - Firmware DateCode [YYMD]
 *    - YY = Last two digits of the Year
 *    - M = (1-9 is Jan-Sept),(A-C is Oct-Dec)
 *    - D = (1-9 is 1-9),(A-V is 10-31)
 *  <br><br>
 *  - lib/ABC   = Analog Button Capture
 *  - lib/DB    = Debug messages directives
 *  - Disp      = Instantiates the Display Object to use
 *  - E32       = Control of the EBYTE E32-Radio Module
 *  - RFC       = RF Control Protocol Implementation ( RxPacket, TxPacket, RadioI )
 *  - Menu      = Menu Base Classes ( MenuItem, MenuList, Navigator )
 *  <br><u><b>Menu-[L]ists</b></u><br>
 *  - LHdw      = Tode, TodeList
 *  <br><u><b>Menu-[i]tems</b></u><br>
 *  - iLib      = MenuName, MenuValue; for Basic Name-to-Value Operations
 *  - iHdw      = PinSelect, Device; for Hardware Selection and Device Interface
 *  - iDev      = OnOff; for IO devices control
 *  - iRF       = E32Radio; puts E32 Controls on the Menu
 *  
 *
 * \section Overview Conceptual Overview
 *  Every IO-Device Control is a functionally expanded Menu-Item
 *  
 *  - MenuName      for use when no value is needed; like a menu line that allows linking to other 
 *                    'SubList' of items or carrying out a specified \ref NAV Command.
 *  
 *  - MenuValue     Inherits MenuName; The Value() is stored as a variable in MEM
 *    
 *    - \ref VT Value Type Properties
 *      - VTRW    = Value is Read & Writable on the Menu
 *      - VTDIG   = Per Digit Setable on the Menu
 *      - VTHEX   = Value is displayed as a Positive Hexadecimal Value on the Menu
 *      - VTBYTE  = Value is a single Positive Byte Size
 *    
 *  - MenuEEValue   Inherits MenuValue; The Value() is stored in an EEPROM Location for Power On/Off persistance
 *  - Device        Inherits MenuValue; The Value() is coordinated by RF-Communications or by local IO-depending on 'IsLocal'
 *    
 *  - Common Accronyms in Code
 *      - AEB         [A]llocation-Size of [E]EPROM in [B]ytes
 *      - EEA         [EE]PROM [A]ddress
 *      - EMC         [E]EPROM [M]emory [C]onstants
 *      - EMO         [E]EPROM [M]emory Address [O]ffsets
 * 
 * \section IO-HDW New IO-HDW
 *    - Create (2) 32-bit longs and (1) byte and assign bit-wise usable Pins
 *        1. Used in DevLib.cpp PinSelect::ChangeSetTo
 *        2. Name Pin Usage in PinSelect::PinSelect Constructor
 *        
 * \section Display Display-Type Encapsulation
 *    - Encapsulated and changeable in Display.h 
 *        - Creates static object *oDisplay* that is assigned to *Display* in every Constructor
 *        - DSPCLASS Constant is used to change Display Libraries
 *        - Implements various display call changes by Constants
 *        
 * \section Navigation User-Navigation
 *    The round-robin \ref KEY processing is done by passing the KEY down to the deepest Link-List pointer level
 *    available at which point the KEY is either USED, REPLACED or RETURNED back up the Link-List tree.  The display
 *    is also controlled by Navigate() calls.
 *    
 *    - Navigator::Navigate
 *        1. SENDS the KEY to CurrList
 *        2. RETURN Changes the CurrList if NAVKEYRIGHT or NAVKEYLEFT or NAVGOTOSUBLIST
 *    - MenuList::Navigate
 *        1. SENDS the KEY to CurrItem if one is selected.
 *        2. RETURN Changes CurrItem if NAVKEYUP or NAVKEYDOWN
 *    - MenuItem::Navigate (Handled by Each Item as coded)
 *        1. Depending on *Selected* Item-Component
 *        2. NAVKEYRIGHT changes 'Selected' Item-Component
 *        3. NAVKEYOKAY checks for *SubList* and returns NAVGOTOSUBLIST
 *        4. If nothing handles the key within Item it gets returned back to List
 *        
 * \section DEVICE New Devices        
 *    - Each Device-Type is a class
 *        - *Local* Devices has device *Settings* (i.e. IO-Pin)
 *        - *All* Devices have *Control* (i.e. On/Off and Read-Only)
 *    - Each Device-Type has a \ref KEY
 *    - Tode.cpp AddDevice
 *    - Sys.cpp FinalKey
 *    
 * \section EEPROM EEPROM Allocation Map
 *    1.  ThisTode Configuration will always start from the beginning of EEPROM Space
 *    2.  Remote Tode's
 *    3.  System Settings
 *        - Including ThisTode's Device Settings (not control)
 *    
 * \section STRUCTURES STRUCTURES How EEPROM, INDEX, and RFID are related
 *    1.  TodeIndex (0-9) 10-Total is exactly equivalent to EEPROM Address by Calculation
 *    2.  However; TodeIndex is only static to ThisTode (Todes can have different Index on different Todes)
 *    3.  Network-wise the Tode Address is it's unique identifier and stored first on Tode's local EEPROM config
 *    4.  RFID for Devices is stored at paticular and equivalent EEPROM Addresses by Calculation
 *    5.  RFID is also static by TodeAddress::RFID so RFID 1 is always 1 on all Todes
 *    6.  
 */
