/******************************************************************************************************************//**
 * @file E22.cpp
 * @brief Ref.h
 *********************************************************************************************************************/
#ifndef _E22_CPP
#define _E22_CPP

#include "E22.h"
//#####################################################################################################################
E22Radio::E22Radio(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX):
  RadioI() {
    DBINITL(("E22Radio::E22Radio"))
    PinM0 = _PinM0; PinM1 = _PinM1; PinTX = _PinTX; PinRX = _PinRX; PinAUX = _PinAUX;
    if ( PinM0 != 0 ) pinMode(PinM0, OUTPUT); 
    if ( PinM1 != 0 ) pinMode(PinM1, OUTPUT);
    if ( PinTX != 0 ) pinMode(PinTX, OUTPUT);
    if ( PinRX != 0 ) pinMode(PinRX, INPUT_PULLUP);
    if ( PinAUX != 0 ) pinMode(PinAUX, INPUT_PULLUP);
    
    // Check RF Settings
    Mode(E22_SLEEPMODE);
    unsigned int RFAddress=0;RFAddress=Address();
    DBINFOAL(("E22Radio::E22Radio Radio Address: "),    (RFAddress,HEX))
    DBINFOAL(("E22Radio::E22Radio Radio Freq[mhz]: "),  (Frequency()))
    DBINFOAL(("E22Radio::E22Radio RF->AirSpeed"),       (AirSpeed()))
    DBINFOAL(("E22Radio::E22Radio RF->UARTSpeed"),      (UARTSpeed()))
    DBINFOAL(("E22Radio::E22Radio RF->FECWakeIOFixed"), (FECWakeIOFixed()))
    DBINFOAL(("E22Radio::E22Radio Radio Power[db]: "),  (TxPower()))
    unsigned int EETode0RFAddress=0; EEPROM.get(0, EETode0RFAddress); //TodeIndex*AEB_TODEALLOC
    if ( RFAddress != EETode0RFAddress ) {      // Check EEPROM Saved Address == RF->Address()
      DBINFOAL( ("E22Radio::E22Radio RFAddress!=?EETode0RFAddress? UPDATING EEPROM"), ((unsigned int)EETode0RFAddress,HEX) )
      EEPROM.put(0, RFAddress);
    }
    if ( UARTSpeed() != 12 ) {
      DBINFOL(("E22Radio::E22Radio UPDATING RFPARAM RF->UARTSpeed!=12"))
      UARTSpeed(12); //0110 0010 - 0110 0xxx so '011'00' = 9600bps,8N1
    }
    if ( FECWakeIOFixed() != 67 ) {
      DBINFOL(("E22Radio::E22Radio UPDATING RFPARAM RF->FECWakeIOFixed!=67"))
      FECWakeIOFixed(67);
      //This is correct REG3 is 0x43 on Fixed Transmission WOR=2000ms(default)
    }
    Mode(E22_NORMMODE);
}
//---------------------------------------------------------------------------------------------------------------------
void E22Radio::Address(unsigned int _RFAddress) {
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);    // Enter Sleep-Mode
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E22Param[E22ADDH] = highByte(_RFAddress);                   // Set bytes[00H]
  E22Param[E22ADDL] = lowByte(_RFAddress);                    // Set bytes[01H]
  EEPROM.put(0, _RFAddress);                                    // Save new address in EEPROM too.
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
unsigned int E22Radio::Address() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return word(E22Param[E22ADDH], E22Param[E22ADDL]);        // bytes[12]
}
//---------------------------------------------------------------------------------------------------------------------
void E22Radio::NetID(byte _NetID) {
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);    // Enter Sleep-Mode
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E22Param[E22NETID] = _NetID;
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//---------------------------------------------------------------------------------------------------------------------
byte E22Radio::NetID() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return E22Param[E22NETID];
}
//-----------------------------------------------------------------------------------------------------
void E22Radio::AirSpeed(byte _Speed) {
  if ( _Speed > 7 ) return;                                     // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);    // Enter Sleep-Mode
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E22Param[E22REG0] &= 0xF8;                                  // 0 - previous settings ( bits[012] )
  E22Param[E22REG0] += _Speed;                                // Set Frequency in Parameters
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte E22Radio::AirSpeed() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return (E22Param[E22REG0] & 0x07);                          // Return REG0 bit[012]
}
//-----------------------------------------------------------------------------------------------------
void E22Radio::UARTSpeed(byte _SpeedParity) {                  // E22 is ParitySpeed opposite E32 SpeedParity
  if ( _SpeedParity > 31 ) return;                              // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);    // Enter Sleep-Mode
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E22Param[E22REG0] &= 0x07;                                  // 0 - previous settings ( bits[34567] )
  E22Param[E22REG0] += (_SpeedParity << 3);                   // Set UART Speed in Parameters
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte E22Radio::UARTSpeed() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return ((E22Param[E22REG0] & 0xF8) >> 3);
}
//-----------------------------------------------------------------------------------------------------
// E22 - 'Channel' is REG2 calculated by MHz = 410.125 + REG2 *1M (0CH=410.125 & 83CH=493.125)
// Each (1)Channel is (1)MHz adjust [1F=31].  Default is 17H(23DEC) + 410 = 433.
//-----------------------------------------------------------------------------------------------------
void E22Radio::Frequency(int _RFFrequency) {
  if ( 410 > _RFFrequency || _RFFrequency > 493 ) return;       // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);    // Enter Sleep-Mode
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E22Param[E22REG2] = _RFFrequency-410;                       // Set Frequency in Parameters[4]
  SetParam();                                                   // Write Parameters
  Mode(CurrentMode);                                            // Return to Current Mode
}
//---------------------------------------------------------------------------------------------------------------------
int E22Radio::Frequency() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return 410 + E22Param[E22REG2];                             // Return Channel in MHz form[4]
}
//-----------------------------------------------------------------------------------------------------
byte E22Radio::Channel() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return E22Param[E22REG2];                                   // Return Channel in Channel form[4]
}
//---------------------------------------------------------------------------------------------------------------------
void E22Radio::TxPower(byte _TxPower) {
  if ( _TxPower > 3 ) return;                                   // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);    // Enter Sleep-Mode
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E22Param[E22REG1] &= 0xFC;                                  // 0 - previous setting ( bits 0 & 1 )
  E22Param[E22REG1] += _TxPower;                              // Set Frequency in Parameters
  SetParam();                                                   // Write Parameters
  Mode(CurrentMode);                                            // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte E22Radio::TxPower() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return (E22Param[E22REG1] & 0x03);                          // REG1-bits[01]
  // Decoded by MenuValue (s) SetNumberName() see RadioTxPower
}
//-----------------------------------------------------------------------------------------------------RETURN TO FIX
void E22Radio::FECWakeIOFixed(byte _BitCombo) {
  if ( _BitCombo > 255 ) return;                                // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);    // Enter Sleep-Mode
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E22Param[E22REG3] = _BitCombo;                              // Set Parameter
  SetParam();                                                   // Write Parameters
  Mode(CurrentMode);                                            // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------RETURN TO FIX
byte E22Radio::FECWakeIOFixed() {
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return E22Param[E22REG3];                                   // return REG3
}
//---------------------------------------------------------------------------------------------------------------------
void E22Radio::GetParam() {                                           //DBENTERL(("E32Radio::GetParam"))
  byte CurrentMode = Mode();
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);
  RFSERIAL.write(0xC1);RFSERIAL.write(0x00);RFSERIAL.write(0x09);
  int ReceivedByte = -1;
  unsigned long StartMillis = millis();
  while ( millis() - StartMillis < 200 ) {
    if ( RFSERIAL.available() ) {
      ReceivedByte++; E22Param[ReceivedByte] = RFSERIAL.read();
      //DBINFOAL(("ReceivedByte : "),(E22Param[ReceivedByte]))
      if (ReceivedByte>=10) break;
    }
  }
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
void E22Radio::SetParam() {                                           DBENTERL(("E32Radio::SetParam"))
  byte CurrentMode = Mode();
  if ( CurrentMode != E22_SLEEPMODE ) Mode(E22_SLEEPMODE);
  RFSERIAL.write(0xC0);RFSERIAL.write(0x00);RFSERIAL.write(0x09);      //#define E22HEAD       0
  RFSERIAL.write(E22Param[E22ADDH]);    //1 00H ADDH
  RFSERIAL.write(E22Param[E22ADDL]);    //2 01H ADDL
  RFSERIAL.write(E22Param[E22NETID]);   //3 02H NETID
  RFSERIAL.write(E22Param[E22REG0]);    //4 03H REG0 UART.765,PARITY.43,AIR.210
  RFSERIAL.write(E22Param[E22REG1]);    //5 04H REG1 SUB.76,RSSI.5,NA.432,TX.01
  RFSERIAL.write(E22Param[E22REG2]);    //6 05H REG2 FREQ.76543210
  RFSERIAL.write(E22Param[E22REG3]);    //7 06H REG3 RSSI.7,FIX.6,REPEATER.5,LBT.4,WOR.3,WOR.210
  RFSERIAL.write(E22Param[E22CRYPTH]);  //8 07H CRYPT_H Key-byte
  RFSERIAL.write(E22Param[E22CRYPTL]);  //9 08H CRYPT_L Key-byte
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
/*
                          //  M1, M0  - E22 - Operating Modes (SPECIFIC)
#define E22_NORMMODE  0   //  0   0   - TXTNORM OPERATION
#define E22_WAKEMODE  1   //  0   1   - WOR Mode (E32 & E220 is WAKE UP)
#define E22_PWRSAVE   2   //  1   1   - DEEP SLEEP ( NO TRANSmiT, AIR-AWAKE )
#define E22_SLEEPMODE 3   //  1   0   - CONFIGURATION (E32 & E220 is SLEEP/COMMAND MODE)
*/
//---------------------------------------------------------------------------------------------------------------------
void E22Radio::Mode(byte _Mode) {
  if ( _Mode == E22_NORMMODE ) {      digitalWrite(PinM1, LOW );digitalWrite(PinM0, LOW );}
  else if ( _Mode == E22_WAKEMODE ) { digitalWrite(PinM1, LOW );digitalWrite(PinM0, HIGH);}
  else if ( _Mode == E22_PWRSAVE ) {  digitalWrite(PinM1, HIGH);digitalWrite(PinM0, HIGH);}
  else if ( _Mode == E22_SLEEPMODE ) {digitalWrite(PinM1, HIGH);digitalWrite(PinM0, LOW );}
  delay(20); // Radio will not respond faster than this.
}
//-----------------------------------------------------------------------------------------------------
byte E22Radio::Mode() {
  if ( digitalRead(PinM1) == LOW ) {
    if ( digitalRead(PinM0) == LOW ) { return E22_NORMMODE; } else { return E22_WAKEMODE; }
  } else {
    // PinM1=HIGH
    if ( digitalRead(PinM0) == LOW ) { return E22_SLEEPMODE; } else { return E22_PWRSAVE; }
  }
}
//---------------------------------------------------------------------------------------------------------------------
bool E22Radio::PacketAvailable() {                                        // Check for incoming communications
  
  // Check that Radio is in Normal Mode
  if ( Mode() != E22_NORMMODE ) {
    DBINFOL(("E22Radio::PacketAvailable() Mode() != E22_NORMMODE"))
    return false;
  }

  // Check if data exists
  if ( RFSERIAL.available() == 0 ) return false;
  DBINFOAL(("E22Radio::PacketAvailable - RFSERIAL.available() = "),(RFSERIAL.available()));
  
  if ( Packet != 0 ) {                                                    // Expire Packet build after 1-Second
    if ( millis() - Packet->PacketStartTimeMS > 1000 ) { 
      DBINFOL(("E22Radio::PacketAvailable PACKET EXPIRED > 1s"))
      delete Packet; Packet = 0; 
    }
  }
  if ( Packet == 0 ) {                                                    // Create new packet if needed
    Packet = new RxPacket();                   
    Packet->PacketStartTimeMS = millis();
  }

  while ( RFSERIAL.available() ) {                                         // Read RF Data
    if ( Packet->RxByte(RFSERIAL.read()) ) {                               // Returns True when packet size reached
      DBINFOL(("E22Radio::PacketAvailable FULL Packet Received"))
      if ( !Packet->IsValid() ) {
        DBINFOL(("E22Radio::PacketAvailable NOT VALID")) 
        delete(Packet); Packet=0; return false;                           // Return 'false' for InValid Packet
      }
      DBINFOL(("E22Radio::PacketAvailable Packet->IsValid"))
      return true;                                                        // Return 'true' for Valid Packet
    }
  }
  return false;
}
//-----------------------------------------------------------------------------------------------------
void E22Radio::Send(TxPacket* Tx) {
  DBENTERAL(("****************E22Radio::Send(Tx)*********************"),(int(Tx),HEX))
  int i=0;
  
  // Prep & Check
  if ( Tx == 0 ) return;
  if ( E22Param[E22HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  if ( Mode() != E22_NORMMODE ) { DBERRORAL(("E22Radio::Send(Tx) Radio in Mode : "),(Mode())) return; }

  // Wait for not-busy
  RFSERIAL.flush();
  while ( digitalRead(PinAUX) == LOW ) { };
  
  // Set FromRF
  unsigned int FromRF = Address();
  if ( FromRF == 0xFFFF ) { DBERRORL(("E22Radio::Send(Tx) FromRF=0xFFFF")) return; }
  Tx->Bytes[PKB_FROM_RFH] = highByte(FromRF);
  Tx->Bytes[PKB_FROM_RFL] = lowByte(FromRF);
  
  // Set Channel & Secure
  Tx->Bytes[PKB_CHANNEL] = Channel();
  Tx->Secure();

  // Re-Pack Bytes into Send Queue and show DEBUG INFO
  ArduinoQueue<byte> bSendQueue(518); 
  DBINFOAAL(("To[0][1]: "),(Tx->Bytes[PKB_TO_RFH],HEX),(Tx->Bytes[PKB_TO_RFL],HEX))
  DBINFOAL(("Channel[2]: "),(Tx->Bytes[PKB_CHANNEL],HEX))
  DBINFOAAL(("SecNet,Size: "),(EEPROM.read(EMC_SECNET)),(Tx->Size))
  DBINFOAAL(("SecNet,Size[3][4]: "),(Tx->Bytes[PKB_SECH],HEX),(Tx->Bytes[PKB_SECL],HEX))
  DBINFOAL(("HSize+PacketType[5]: "),(Tx->Bytes[PKB_TYPE],HEX))
  DBINFOAAL(("From[6][7]: "),(Tx->Bytes[PKB_FROM_RFH],HEX),(Tx->Bytes[PKB_FROM_RFL],HEX))
  DBINFOAL(("Tode Version[8]: "),(Tx->Bytes[PKB_TODEVER],HEX))
  //#define PKB_TYPE        5     // This point forward may differ
  //#define PKB_FROM_RFH    6
  //#define PKB_FROM_RFL    7
  //#define PKB_TODEVER     8
  //#define PKB_TODECONFIG  9     // Start Tode Config Data
  //#define PKB_RFID        9     // First RFID on GETVALS
  //#define PKB_VALUEH      10
  //#define PKB_VALUEL      11
  
  i=0; byte Temp=0;
  while ( i<Tx->Size ) {
    if ( i%12 == 0 ) { DBENTERL((" ")) } // new line
    if ( i<58 ) { 
      bSendQueue.enqueue(Tx->Bytes[i]);      
      DBENTERAA(("Tx"),(i),(Tx->Bytes[i],HEX))
    } else {
      if ( Tx->ExtraBytes == 0 ) { DBERRORAL(("E22Radio::Send Tx->ExtraBytes == 0 @i: "),(i)) break; }
      else {
        if ( Tx->ExtraBytes->isEmpty() ) { DBINFOL(("E22Radio::Send Tx-ExtraBytes->isEmpty()")) break; }
        Temp = Tx->ExtraBytes->dequeue();
        DBENTERAA(("Ex"),(i),(Temp))
        bSendQueue.enqueue(Temp);
      }
    }
    i++;
  }
  DBENTERL((""))

  // Send the Send-Queue
  i=0; 
  while ( !bSendQueue.isEmpty() ) { 
    i++;
    if (i==58) {
      RFSERIAL.flush();
      while ( digitalRead(PinAUX) == LOW ) { };     // Wait for not busy to send another 512-bytes
      RFSERIAL.write(Tx->Bytes[PKB_TO_RFH]);         // Resend Header and continue
      RFSERIAL.write(Tx->Bytes[PKB_TO_RFL]);
      RFSERIAL.write(Tx->Bytes[PKB_CHANNEL]);   
    }
    RFSERIAL.write(bSendQueue.dequeue());
  }
}
//_____________________________________________________________________________________________________________________
#endif
