/******************************************************************************************************************//**
 * @file E220.cpp
 * @brief Ref.h
 *********************************************************************************************************************/
#ifndef _E220_CPP
#define _E220_CPP

#include "E220.h"
//#####################################################################################################################
E220Radio::E220Radio(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX):
  RadioI() {
    DBINITL(("E220Radio::E220Radio"))
    PinM0 = _PinM0; PinM1 = _PinM1; PinTX = _PinTX; PinRX = _PinRX; PinAUX = _PinAUX;
    if ( PinM0 != 0 ) pinMode(PinM0, OUTPUT); 
    if ( PinM1 != 0 ) pinMode(PinM1, OUTPUT);
    if ( PinTX != 0 ) pinMode(PinTX, OUTPUT);
    if ( PinRX != 0 ) pinMode(PinRX, INPUT_PULLUP);
    if ( PinAUX != 0 ) pinMode(PinAUX, INPUT_PULLUP);
    
    // Check RF Settings
    Mode(E220_SLEEPMODE);
    unsigned int RFAddress=0;RFAddress=Address();
    DBINFOAL(("E220Radio::E220Radio Radio Address: "),    (RFAddress,HEX))
    DBINFOAL(("E220Radio::E220Radio Radio Freq[mhz]: "),  (Frequency()))
    DBINFOAL(("E220Radio::E220Radio RF->AirSpeed"),       (AirSpeed()))
    DBINFOAL(("E220Radio::E220Radio RF->UARTSpeed"),      (UARTSpeed()))
    DBINFOAL(("E220Radio::E220Radio RF->FECWakeIOFixed"), (FECWakeIOFixed()))
    DBINFOAL(("E220Radio::E220Radio Radio Power[db]: "),  (TxPower()))
    unsigned int EETode0RFAddress=0; EEPROM.get(0, EETode0RFAddress); //TodeIndex*AEB_TODEALLOC
    if ( RFAddress != EETode0RFAddress ) {      // Check EEPROM Saved Address == RF->Address()
      DBINFOAL( ("E220Radio::E220Radio RFAddress!=?EETode0RFAddress? UPDATING EEPROM"), ((unsigned int)EETode0RFAddress,HEX) )
      EEPROM.put(0, RFAddress);
    }
    if ( UARTSpeed() != 12 ) {
      DBINFOL(("E220Radio::E220Radio UPDATING RFPARAM RF->UARTSpeed!=12"))
      UARTSpeed(12); //0110 0010 - 0110 0xxx so '011'00' = 9600bps,8N1
    }
    if ( FECWakeIOFixed() != 67 ) {
      DBINFOL(("E220Radio::E220Radio UPDATING RFPARAM RF->FECWakeIOFixed!=67"))
      FECWakeIOFixed(67);
      //This is correct REG3 is 0x43 on Fixed Transmission WOR=2000ms(default)
    }
    Mode(E220_NORMMODE);
}
//---------------------------------------------------------------------------------------------------------------------
void E220Radio::Address(unsigned int _RFAddress) {
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);    // Enter Sleep-Mode
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E220Param[E220ADDH] = highByte(_RFAddress);                   // Set bytes[00H]
  E220Param[E220ADDL] = lowByte(_RFAddress);                    // Set bytes[01H]
  EEPROM.put(0, _RFAddress);                                    // Save new address in EEPROM too.
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
unsigned int E220Radio::Address() {
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return word(E220Param[E220ADDH], E220Param[E220ADDL]);        // bytes[12]
}
//-----------------------------------------------------------------------------------------------------
void E220Radio::AirSpeed(byte _Speed) {
  if ( _Speed > 7 ) return;                                     // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);    // Enter Sleep-Mode
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E220Param[E220REG0] &= 0xF8;                                  // 0 - previous settings ( bits[012] )
  E220Param[E220REG0] += _Speed;                                // Set Frequency in Parameters
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte E220Radio::AirSpeed() {
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return (E220Param[E220REG0] & 0x07);                          // Return REG0 bit[012]
}
//-----------------------------------------------------------------------------------------------------
void E220Radio::UARTSpeed(byte _SpeedParity) {                  // E220 is ParitySpeed opposite E32 SpeedParity
  if ( _SpeedParity > 31 ) return;                              // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);    // Enter Sleep-Mode
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E220Param[E220REG0] &= 0x07;                                  // 0 - previous settings ( bits[34567] )
  E220Param[E220REG0] += (_SpeedParity << 3);                   // Set UART Speed in Parameters
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte E220Radio::UARTSpeed() {
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return ((E220Param[E220REG0] & 0xF8) >> 3);
}
//-----------------------------------------------------------------------------------------------------
// E220 - 'Channel' is REG2 calculated by MHz = 410.125 + REG2 *1M (0CH=410.125 & 83CH=493.125)
// Each (1)Channel is (1)MHz adjust [1F=31].  Default is 17H(23DEC) + 410 = 433.
//-----------------------------------------------------------------------------------------------------
void E220Radio::Frequency(int _RFFrequency) {
  if ( 410 > _RFFrequency || _RFFrequency > 493 ) return;       // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);    // Enter Sleep-Mode
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E220Param[E220REG2] = _RFFrequency-410;                       // Set Frequency in Parameters[4]
  SetParam();                                                   // Write Parameters
  Mode(CurrentMode);                                            // Return to Current Mode
}
//---------------------------------------------------------------------------------------------------------------------
int E220Radio::Frequency() {
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return 410 + E220Param[E220REG2];                             // Return Channel in MHz form[4]
}
//-----------------------------------------------------------------------------------------------------
byte E220Radio::Channel() {
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return E220Param[E220REG2];                                   // Return Channel in Channel form[4]
}
//---------------------------------------------------------------------------------------------------------------------
void E220Radio::TxPower(byte _TxPower) {
  if ( _TxPower > 3 ) return;                                   // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);    // Enter Sleep-Mode
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E220Param[E220REG1] &= 0xFC;                                  // 0 - previous setting ( bits 0 & 1 )
  E220Param[E220REG1] += _TxPower;                              // Set Frequency in Parameters
  SetParam();                                                   // Write Parameters
  Mode(CurrentMode);                                            // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte E220Radio::TxPower() {
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return (E220Param[E220REG1] & 0x03);                          //byte[5]bits[01]
  // Decoded by MenuValue (s) SetNumberName() see RadioTxPower
}
//-----------------------------------------------------------------------------------------------------RETURN TO FIX
void E220Radio::FECWakeIOFixed(byte _BitCombo) {
  if ( _BitCombo > 255 ) return;                                // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);    // Enter Sleep-Mode
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  E220Param[E220REG3] = _BitCombo;                              // Set Parameter
  SetParam();                                                   // Write Parameters
  Mode(CurrentMode);                                            // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------RETURN TO FIX
byte E220Radio::FECWakeIOFixed() {
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  return E220Param[E220REG3];                                   // return REG3
}
//---------------------------------------------------------------------------------------------------------------------
void E220Radio::GetParam() {                                           //DBENTERL(("E32Radio::GetParam"))
  byte CurrentMode = Mode();
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);
  RFSERIAL.write(0xC1);RFSERIAL.write(0x00);RFSERIAL.write(0x08);
  int ReceivedByte = -1;
  unsigned long StartMillis = millis();
  while ( millis() - StartMillis < 200 ) {
    if ( RFSERIAL.available() ) {
      ReceivedByte++; E220Param[ReceivedByte] = RFSERIAL.read();
      //DBINFOAL(("ReceivedByte : "),(E220Param[ReceivedByte]))
      if (ReceivedByte>=10) break;
    }
  }
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
void E220Radio::SetParam() {                                           DBENTERL(("E32Radio::SetParam"))
  byte CurrentMode = Mode();
  if ( CurrentMode != E220_SLEEPMODE ) Mode(E220_SLEEPMODE);
  RFSERIAL.write(0xC0);RFSERIAL.write(0x00);RFSERIAL.write(0x08);      //#define E220HEAD       0
  RFSERIAL.write(E220Param[E220ADDH]);    //1 00H ADDH
  RFSERIAL.write(E220Param[E220ADDL]);    //2 01H ADDL
  RFSERIAL.write(E220Param[E220REG0]);    //3 02H REG0 UART.765,PARITY.43,AIR.210
  RFSERIAL.write(E220Param[E220REG1]);    //4 03H REG1 SUB.76,RSSI.5,NA.432,TX.01
  RFSERIAL.write(E220Param[E220REG2]);    //5 04H REG2 FREQ.76543210
  RFSERIAL.write(E220Param[E220REG3]);    //6 05H REG3 RSSI.7,FIX.6,NA.5,LBT.4,NA.3,WOR.210
  RFSERIAL.write(E220Param[E220CRYPTH]);  //7 06H CRYPT_H Key-byte
  RFSERIAL.write(E220Param[E220CRYPTL]);  //8 07H CRYPT_L Key-byte
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//---------------------------------------------------------------------------------------------------------------------
void E220Radio::Mode(byte _Mode) {
  if ( _Mode == E220_NORMMODE ) { digitalWrite(PinM0, LOW); digitalWrite(PinM1, LOW); }
  else if ( _Mode == E220_WAKEMODE ) { digitalWrite(PinM0, LOW); digitalWrite(PinM1, HIGH); }
  else if ( _Mode == E220_PWRSAVE ) { digitalWrite(PinM0, HIGH); digitalWrite(PinM1, LOW); }
  else if ( _Mode == E220_SLEEPMODE ) { digitalWrite(PinM0, HIGH); digitalWrite(PinM1, HIGH); }
  delay(20); // Radio will not respond faster than this.
}
//-----------------------------------------------------------------------------------------------------
byte E220Radio::Mode() {
  if ( digitalRead(PinM0) == LOW ) {
    if ( digitalRead(PinM1) == LOW ) { return E220_NORMMODE; } else { return E220_WAKEMODE; }
  } else {
    // PinM0=HIGH
    if ( digitalRead(PinM1) == LOW ) { return E220_PWRSAVE; } else { return E220_SLEEPMODE; }
  }
}
//---------------------------------------------------------------------------------------------------------------------
bool E220Radio::PacketAvailable() {                                        // Check for incoming communications

  // Check that Radio is in Normal Mode
  if ( Mode() != E220_NORMMODE ) {
    DBINFOL(("E220Radio::PacketAvailable() Mode() != E220_NORMMODE"))
    return false;
  }

  // Check if data exists
  if ( RFSERIAL.available() == 0 ) return false;
  DBINFOAL(("E220Radio::PacketAvailable - RFSERIAL.available() = "),(RFSERIAL.available()));
  
  if ( Packet != 0 ) {                                                    // Expire Packet build after 1-Second
    if ( millis() - Packet->PacketStartTimeMS > 1000 ) { 
      DBINFOL(("E220Radio::PacketAvailable PACKET EXPIRED > 1s"))
      delete Packet; Packet = 0; 
    }
  }
  if ( Packet == 0 ) {                                                    // Create new packet if needed
    Packet = new RxPacket();                   
    Packet->PacketStartTimeMS = millis();
  }

  while ( RFSERIAL.available() ) {                                         // Read RF Data
    if ( Packet->RxByte(RFSERIAL.read()) ) {                               // Returns True when packet size reached
      DBINFOL(("E220Radio::PacketAvailable FULL Packet Received"))
      if ( !Packet->IsValid() ) {
        DBINFOL(("E220Radio::PacketAvailable NOT VALID")) 
        delete(Packet); Packet=0; return false;                           // Return 'false' for InValid Packet
      }
      DBINFOL(("E220Radio::PacketAvailable Packet->IsValid"))
      return true;                                                        // Return 'true' for Valid Packet
    }
  }
  return false;
}
//-----------------------------------------------------------------------------------------------------
void E220Radio::Send(TxPacket* Tx) {
  DBENTERAL(("****************E220Radio::Send(Tx)*********************"),(int(Tx),HEX))
  int i=0;
  
  // Prep & Check
  if ( Tx == 0 ) return;
  if ( E220Param[E220HEAD0] != 0xC1 ) GetParam();               // Get Parameters
  if ( Mode() != E220_NORMMODE ) { DBERRORAL(("E220Radio::Send(Tx) Radio in Mode : "),(Mode())) return; }

  // Wait for not-busy
  RFSERIAL.flush();
  while ( digitalRead(PinAUX) == LOW ) { };
  
  // Set FromRF
  unsigned int FromRF = Address();
  if ( FromRF == 0xFFFF ) { DBERRORL(("E220Radio::Send(Tx) FromRF=0xFFFF")) return; }
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
      if ( Tx->ExtraBytes == 0 ) { DBERRORAL(("E220Radio::Send Tx->ExtraBytes == 0 @i: "),(i)) break; }
      else {
        if ( Tx->ExtraBytes->isEmpty() ) { DBINFOL(("E220Radio::Send Tx-ExtraBytes->isEmpty()")) break; }
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
