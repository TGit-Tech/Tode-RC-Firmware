/******************************************************************************************************************//**
 * @file E32.cpp
 * @brief Ref.h
 *********************************************************************************************************************/
#ifndef _E32_CPP
#define _E32_CPP

#include "E32.h"
//#####################################################################################################################
E32Radio::E32Radio(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX):
  RadioI() {
    DBINITL(("E32Radio::E32Radio"))
    PinM0 = _PinM0; PinM1 = _PinM1; PinTX = _PinTX; PinRX = _PinRX; PinAUX = _PinAUX;
    if ( PinM0 != 0 ) pinMode(PinM0, OUTPUT); 
    if ( PinM1 != 0 ) pinMode(PinM1, OUTPUT);
    if ( PinTX != 0 ) pinMode(PinTX, OUTPUT);
    if ( PinRX != 0 ) pinMode(PinRX, INPUT_PULLUP);
    if ( PinAUX != 0 ) pinMode(PinAUX, INPUT_PULLUP);
    
    // Check E32 RF-Settings
    Mode(E32_SLEEPMODE);
    unsigned int RFAddress=0;RFAddress=Address();
    DBINFOAL(("E32Radio::E32Radio Radio Address: "),    (RFAddress,HEX))
    DBINFOAL(("E32Radio::E32Radio Radio Freq[mhz]: "),  (Frequency()))
    DBINFOAL(("E32Radio::E32Radio AirSpeed"),           (AirSpeed()))
    DBINFOAL(("E32Radio::E32Radio UARTSpeed"),          (UARTSpeed()))
    DBINFOAL(("E32Radio::E32Radio FECWakeIOFixed"),     (FECWakeIOFixed()))
    DBINFOAL(("E32Radio::E32Radio Radio Power[db]: "),  (TxPower()))
    unsigned int EETode0RFAddress=0; EEPROM.get(0, EETode0RFAddress); //TodeIndex*AEB_TODEALLOC
    if ( RFAddress != EETode0RFAddress ) {      // Check EEPROM Saved Address == RF->Address()
      DBINFOAL( ("E32Radio::E32Radio RFAddress!=?EETode0RFAddress? UPDATING EEPROM"), (EETode0RFAddress,HEX) )
      EEPROM.put(0, RFAddress);
    }    
    if ( UARTSpeed() != 3 ) {
      DBINFOL(("E32Radio::E32Radio UPDATING RFPARAM RF->UARTSpeed != 3"))
      UARTSpeed(3);
    }
    if ( FECWakeIOFixed() != 49 ) {
      DBINFOL(("E32Radio::E32Radio UPDATING RFPARAM RF->FECWakeIOFixed != 49"))
      FECWakeIOFixed(49);
    }
    Mode(E32_NORMMODE);
}
//---------------------------------------------------------------------------------------------------------------------
void E32Radio::Address(unsigned int _RFAddress) {
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);      // Enter Sleep-Mode
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                  // Get Parameters
  E32Param[E32ADDH] = highByte(_RFAddress);                     // Set bytes[12]
  E32Param[E32ADDL] = lowByte(_RFAddress);                      // Set bytes[12]
  EEPROM.put(0, _RFAddress);                                    // Save new address in EEPROM too.
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
unsigned int E32Radio::Address() {
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();
  return word(E32Param[E32ADDH], E32Param[E32ADDL]);            // bytes[12]
}
//-----------------------------------------------------------------------------------------------------
void E32Radio::AirSpeed(byte _Speed) {
  if ( _Speed > 7 ) return;                                     // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);      // Enter Sleep-Mode
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                  // Get Parameters
  E32Param[E32SPEED] &= 0xF8;                                   // 0 - previous setting ( bits[012] )
  E32Param[E32SPEED] += _Speed;                                 // Set Frequency in Parameters
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte E32Radio::AirSpeed() {
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                  // Get Parameters
  return (E32Param[E32SPEED] & 0x07);                           // Return byte[3]bit[012]
}
//-----------------------------------------------------------------------------------------------------
void E32Radio::UARTSpeed(byte _SpeedParity) {
  if ( _SpeedParity > 31 ) return;                              // Check Boundaries
  byte CurrentMode = Mode();                                    // Set & Return to Current Mode
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);      // Enter Sleep-Mode
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                  // Get Parameters
  E32Param[E32SPEED] &= 0x07;                                   // 0 - previous setting ( bits[34567] )
  E32Param[E32SPEED] += (_SpeedParity << 3);                    // Set UART Speed in Parameters
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte E32Radio::UARTSpeed() {
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                  // Get Parameters
  return ((E32Param[E32SPEED] & 0xF8) >> 3);
}
//-----------------------------------------------------------------------------------------------------
// E32 - 'Channel' corresponds to 00H to 1FH and assigns 410 to 441MHz 
// Each (1)Channel is (1)MHz adjust [1F=31].  Default is 17H(23DEC) + 410 = 433.
//-----------------------------------------------------------------------------------------------------
void E32Radio::Frequency(int _RFFrequency) {
  if ( 410 > _RFFrequency || _RFFrequency > 441 ) return;         // Check Boundaries
  byte CurrentMode = Mode();                                      // Set & Return to Current Mode
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);        // Enter Sleep-Mode
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                    // Get Parameters
  E32Param[E32CHAN] = _RFFrequency-410;                           // Set Frequency in Parameters[4]
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//---------------------------------------------------------------------------------------------------------------------
int E32Radio::Frequency() {
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                    // Get Parameters
  return 410 + (E32Param[E32CHAN] & 0x1F);                        // Return Channel in MHz form[4]
}
//-----------------------------------------------------------------------------------------------------
byte E32Radio::Channel() {
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                    // Get Parameters
  return (E32Param[E32CHAN] & 0x1F);                              // Return Channel in Channel form[4]
}
//---------------------------------------------------------------------------------------------------------------------
void E32Radio::TxPower(byte _TxPower) {
  if ( _TxPower > 3 ) return;                                     // Check Boundaries
  byte CurrentMode = Mode();                                      // Set & Return to Current Mode
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);        // Enter Sleep-Mode
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                    // Get Parameters
  E32Param[E32OPTION] &= 0xFC;                                    // 0 - previous setting ( bits 0 & 1 )
  E32Param[E32OPTION] += _TxPower;                                // Set Frequency in Parameters
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte E32Radio::TxPower() {
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                    // Get Parameters
  return (E32Param[E32OPTION] & 0x03);                            //byte[5]bits[01]
  // Decoded by MenuValue (s) SetNumberName() see RadioTxPower
}
//-----------------------------------------------------------------------------------------------------
void E32Radio::FECWakeIOFixed(byte _BitCombo) {
  if ( _BitCombo > 252 ) return;                                  // Check Boundaries
  byte CurrentMode = Mode();                                      // Set & Return to Current Mode
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);        // Enter Sleep-Mode
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                    // Get Parameters
  E32Param[E32OPTION] &= 0x03;                                    // 0 - previous setting ( bits 0 & 1 )
  E32Param[E32OPTION] += (_BitCombo << 2);                        // Set Frequency in Parameters
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte E32Radio::FECWakeIOFixed() {
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();                    // Get Parameters
  return ((E32Param[E32OPTION] & 0xFC) >> 2);                     // byte[5]bit[234567]
}
//---------------------------------------------------------------------------------------------------------------------
void E32Radio::GetParam() {                                           //DBENTERL(("E32Radio::GetParam"))
  byte CurrentMode = Mode();
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);
  RFSERIAL.write(0xC1);RFSERIAL.write(0xC1);RFSERIAL.write(0xC1);
  int ReceivedByte = -1;
  unsigned long StartMillis = millis();
  while ( millis() - StartMillis < 200 ) {
    if ( RFSERIAL.available() ) {
      ReceivedByte++; E32Param[ReceivedByte] = RFSERIAL.read();
      //DBINFOAL(("ReceivedByte : "),(E32Param[ReceivedByte]))
      if (ReceivedByte>=5) break;
    }
  }
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
void E32Radio::SetParam() {                                           DBENTERL(("E32Radio::SetParam"))
  byte CurrentMode = Mode();
  if ( CurrentMode != E32_SLEEPMODE ) Mode(E32_SLEEPMODE);
  RFSERIAL.write(0xC0);      //#define E32HEAD       0
  RFSERIAL.write(E32Param[E32ADDH]);
  RFSERIAL.write(E32Param[E32ADDL]);
  RFSERIAL.write(E32Param[E32SPEED]);
  RFSERIAL.write(E32Param[E32CHAN]);
  RFSERIAL.write(E32Param[E32OPTION]);
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//---------------------------------------------------------------------------------------------------------------------
void E32Radio::Mode(byte _Mode) {
  if ( _Mode == E32_NORMMODE ) { digitalWrite(PinM0, LOW); digitalWrite(PinM1, LOW); }
  else if ( _Mode == E32_WAKEMODE ) { digitalWrite(PinM0, LOW); digitalWrite(PinM1, HIGH); }
  else if ( _Mode == E32_PWRSAVE ) { digitalWrite(PinM0, HIGH); digitalWrite(PinM1, LOW); }
  else if ( _Mode == E32_SLEEPMODE ) { digitalWrite(PinM0, HIGH); digitalWrite(PinM1, HIGH); }
  delay(20); // Radio will not respond faster than this.
}
//-----------------------------------------------------------------------------------------------------
byte E32Radio::Mode() {
  if ( digitalRead(PinM0) == LOW ) {
    if ( digitalRead(PinM1) == LOW ) { return E32_NORMMODE; } else { return E32_WAKEMODE; }
  } else {
    // PinM0=HIGH
    if ( digitalRead(PinM1) == LOW ) { return E32_PWRSAVE; } else { return E32_SLEEPMODE; }
  }
}
//---------------------------------------------------------------------------------------------------------------------
bool E32Radio::PacketAvailable() {                                        // Check for incoming communications
  
  // Check that Radio is in Normal Mode
  if ( Mode() != E32_NORMMODE ) {
    DBINFOL(("E32Radio::PacketAvailable() Mode() != E32_NORMMODE"))
    return false;
  }

  // Check if data exists
  if ( RFSERIAL.available() == 0 ) return false;
  DBINFOAL(("E32Radio::PacketAvailable - RFSERIAL.available() = "),(RFSERIAL.available()));
  
  if ( Packet != 0 ) {                                                    // Expire Packet build after 1-Second
    if ( millis() - Packet->PacketStartTimeMS > 5000 ) { 
      DBINFOL(("E32Radio::PacketAvailable PACKET EXPIRED > 1s"))
      delete Packet; Packet = 0; 
    }
  }
  if ( Packet == 0 ) {                                                    // Create new packet if needed
    Packet = new RxPacket();                   
    Packet->PacketStartTimeMS = millis();
  }

  while ( RFSERIAL.available() ) {                                         // Read RF Data
    if ( Packet->RxByte(RFSERIAL.read()) ) {                               // Returns True when packet size reached
      DBINFOL(("E32Radio::PacketAvailable FULL Packet Received"))
      if ( !Packet->IsValid() ) {
        DBINFOL(("E32Radio::PacketAvailable NOT VALID")) 
        delete(Packet); Packet=0; return false;                           // Return 'false' for InValid Packet
      }
      DBINFOL(("E32Radio::PacketAvailable Packet->IsValid"))
      return true;                                                        // Return 'true' for Valid Packet
    }
  }
  return false;
}
//-----------------------------------------------------------------------------------------------------
void E32Radio::Send(TxPacket* Tx) {
  DBENTERAL(("****************E32Radio::Send(Tx)*********************"),(int(Tx),HEX))
  int i=0;
  
  // Prep & Check
  if ( Tx == 0 ) return;
  if ( E32Param[E32HEAD] != 0xC0 ) GetParam();
  if ( Mode() != E32_NORMMODE ) { DBERRORAL(("E32Radio::Send(Tx) Radio in Mode : "),(Mode())) return; }

  // Wait for not-busy
  RFSERIAL.flush();
  while ( digitalRead(PinAUX) == LOW ) { };
  
  // Set FromRF
  unsigned int FromRF = Address();
  if ( FromRF == 0xFFFF ) { DBERRORL(("E32Radio::Send(Tx) FromRF=0xFFFF")) return; }
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
      if ( Tx->ExtraBytes == 0 ) { DBERRORAL(("E32Radio::Send Tx->ExtraBytes == 0 @i: "),(i)) break; }
      else {
        if ( Tx->ExtraBytes->isEmpty() ) { DBINFOL(("E32Radio::Send Tx-ExtraBytes->isEmpty()")) break; }
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
