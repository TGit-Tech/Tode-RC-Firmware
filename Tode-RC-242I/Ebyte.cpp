/******************************************************************************************************************//**
 * @file    E32.cpp
 * @brief   Ref.h
 *********************************************************************************************************************/
#ifndef _EBYTE_CPP
#define _EBYTE_CPP
#include "Ebyte.h"

//#####################################################################################################################
Ebyte::Ebyte(byte _PinM0, byte _PinM1, byte _PinTX, byte _PinRX, byte _PinAUX):
  RadioI() {
    DBINITL(("Ebyte::Ebyte"))
    PinM0 = _PinM0; PinM1 = _PinM1; PinTX = _PinTX; PinRX = _PinRX; PinAUX = _PinAUX;
    if ( PinM0 != 0 ) pinMode(PinM0, OUTPUT); 
    if ( PinM1 != 0 ) pinMode(PinM1, OUTPUT);
    if ( PinTX != 0 ) pinMode(PinTX, OUTPUT);
    if ( PinRX != 0 ) pinMode(PinRX, INPUT_PULLUP);
    if ( PinAUX != 0 ) pinMode(PinAUX, INPUT_PULLUP);
    
    CheckRadio();
}
//---------------------------------------------------------------------------------------------------------------------
byte Ebyte::RadioID() { return EM; }
//-----------------------------------------------------------------------------------------------------
void Ebyte::Address(unsigned int _RFAddress) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramADDH = highByte(_RFAddress);                     // Set bytes[12]
  paramADDL = lowByte(_RFAddress);                      // Set bytes[12]
  EEPROM.put(0, _RFAddress);                                    // Save new address in EEPROM too.
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
unsigned int Ebyte::Address() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return word(paramADDH, paramADDL);            // bytes[12]
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::NetID(byte _NetID) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) { // Only supported on E22 not E220/32
    byte CurrentMode=Mode();                                        // Check Radio is Identified
    if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
    if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed. 
    paramNETID = _NetID;
    SetParam();                                                     // Write Parameters
    Mode(CurrentMode);                                              // Return to Current Mode
  }
}
//-----------------------------------------------------------------------------------------------------
byte Ebyte::NetID() {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) { // Only supported on E22 not E220/32
    if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
    return paramNETID;
  }
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::AirSpeed(byte _Speed) {
  if ( _Speed > 7 ) return;                                     // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramREG0SPED &= 0xF8;                                          // 0 - previous settings ADR.210
  paramREG0SPED += _Speed;                                        // Set AirSpeed in Parameter

  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte Ebyte::AirSpeed() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return (paramREG0SPED&0x07);                                    // ADR.210
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::UARTSpeed(byte _SpeedParity) {
  if ( _SpeedParity > 31 ) return;                              // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramREG0SPED &= 0x07;                 // 0 - previous setting ( bits[76543] )
  paramREG0SPED += (_SpeedParity << 3);  //E32 Parity.76,UARTSPD.543 - E22 UARTSPD.765,Parity.43
  //REG0.765 = UART bps, b000(0)=1200,b001(1)=2400,b010(2)=4800,b011(3)=9600
  //REG0.43 = b00(0)=8N1, b01(1)=8O1, b10(2)=8E1, b11=8N1(3)
  // So for E22 011(UARTSPD.765),00(PAR.43) = 12,0x0C,b1100
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte Ebyte::UARTSpeed() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return ((paramREG0SPED&0xF8)>>3);       //E32 Parity.76,UARTSPD.543 - E22 UARTSPD.765,Parity.43
}
//-----------------------------------------------------------------------------------------------------
// E32 - 'Channel' corresponds to 00H to 1FH and assigns 410 to 441MHz 
// Each (1)Channel is (1)MHz adjust [1F=31].  Default is 17H(23DEC) + 410 = 433.
//-----------------------------------------------------------------------------------------------------
void Ebyte::Frequency(int _RFFrequency) {           DBENTERAL(("Ebyte::Frequency(SET)"),(_RFFrequency))
  if (EM==EM_E22_900T30D&&(850>_RFFrequency||_RFFrequency>930)) return;
  if (!EM==EM_E22_900T30D&&(410>_RFFrequency||_RFFrequency>441)) return;  // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EM==EM_E22_900T30D) {   paramREG2CHAN=_RFFrequency-850;}        // Set Frequency
  else if (EMS==EMS_E220E22) {paramREG2CHAN=_RFFrequency-410;}
  else if (EMS==EMS_E32) {    paramREG2CHAN=_RFFrequency-410;}

  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//---------------------------------------------------------------------------------------------------------------------
int Ebyte::Frequency() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters

  if (EM==EM_E22_900T30D) { return Channel()+850;}
  else {                    return Channel()+410;}
}
//-----------------------------------------------------------------------------------------------------
byte Ebyte::Channel() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return paramREG2CHAN;
  //if (EMS==EMS_E32) return (paramCHAN&0x1F);  // Return CHAN.43210
}
//---------------------------------------------------------------------------------------------------------------------
void Ebyte::TxPower(byte _TxPower) {
  if ( _TxPower > 3 ) return;                                     // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramREG1OPT &= 0xFC;                           // 0 - previous setting TX.10
  paramREG1OPT += _TxPower;                       // Set TxPower

  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte Ebyte::TxPower() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  return (paramREG1OPT&0x03);                                       // TX.10
  // Decoded by MenuValue (s) SetNumberName() see RadioTxPower
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::FixedTransmission(bool _Fixed) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E220E22) {                                         
    if(_Fixed) {paramREG3 += (1<<6);} else {paramREG3 += (0<<6);} // E220&E22 REG3.6
  } else if (EMS==EMS_E32) {
    if(_Fixed) {paramREG1OPT+=(1<<7);} else {paramREG1OPT+=(0<<7);} // E32 OPT.7
  }

  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool Ebyte::FixedTransmission() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E220E22) return ((paramREG3&0x40)>>6);     //E220&E22 REG3.6
  if (EMS==EMS_E32)     return ((paramREG1OPT&0x80)>>7);  //E32 OPT.7
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::E32IODrive(bool _Drive) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  if (EMS==EMS_E220E22) return;                                   // NOT SUPPORTED on E220 or E22
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E32) {
    if (_Drive) {paramREG1OPT+=(1<<6);} else {paramREG1OPT+=(0<<6);}
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool Ebyte::E32IODrive() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E32)     return ((paramREG1OPT&0x40)>>6);            //E32 OPT.6
  // Not supported on E220 & E22
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::RSSIOn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  if (EMS==EMS_E32) return;                                       // NOT SUPPORTED on E32
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E220E22) {
    if (_On) {paramREG3 += (1<<7);} else {paramREG3 += (0<<7);}   //E220&E22 REG3.7
  } 
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool Ebyte::RSSIOn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E220E22) return ((paramREG3&0x80)>>7);    //E220&E22 REG3.7
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::LBTOn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  if (EMS==EMS_E32) return;                                       // NOT SUPPORTED on E32
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E220E22) {
    if (_On) {paramREG3 += (1<<4);} else {paramREG3 += (0<<4);}   //E220&E22 REG3.4
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool Ebyte::LBTOn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E220E22) return ((paramREG3&0x10)>>4);    //E220&E22 REG3.4
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::WUORTime(byte _Time) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E32) {
    paramREG1OPT &= 0xC7;                           // 0 - previous setting E32 WU OPT.543
    paramREG1OPT += (_Time<<3);                     // Set TxPower
  }
  if (EMS==EMS_E220E22) {
    paramREG3 &=0xF8;                                // 0 - previous setting E22 WOR REG3.210
    paramREG3 += _Time;
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte Ebyte::WUORTime() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E32) return ((paramREG1OPT&0x38)>>3);                // E32 WU OPT.543
  if (EMS==EMS_E220E22) return paramREG3&0x03;                      // E22 WOR REG3.210
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::E32FECOn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  if (EMS==EMS_E220E22) return;                                   // NOT SUPPORTED on E220&E22
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E32) {
    if (_On) {paramREG1OPT += (1<<2);} else {paramREG3 += (0<<2);}  //E32 OPT.2
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool Ebyte::E32FECOn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E32) return ((paramREG1OPT&0x04)>>2);                // E32 OPT.2
  // NOT SUPPORTED on E220&E22
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::E22WOROn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) { // Only supported on E22 not E220/32
    byte CurrentMode=Mode();                                        // Check Radio is Identified
    if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
    if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed. 
    if(_On){paramREG3+=(1<<3);}else{paramREG3+=(0<<3);}             // E22 REG3.3
    SetParam();                                                     // Write Parameters
    Mode(CurrentMode);                                              // Return to Current Mode
  }
}
//-----------------------------------------------------------------------------------------------------
bool Ebyte::E22WOROn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) return ((paramREG3&0x08)>>3);
}
//---------------------------------------------------------------------------------------------------------------------
void Ebyte::SetParam() {                                           DBENTERL(("Ebyte::SetParam"))
  // Check Radio is Known and Radio is in SETMODE
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  byte CurrentMode=Mode(); if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);

  // SET Parameters
  if (EMS==EMS_E220E22) {
    RFSERIAL.write(0xC0); //Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
    RFSERIAL.write(0x00); //StartReg ( 0x00(00H) to 0x07(07H) )
    if (EM==EM_E220_400T30D) {RFSERIAL.write(0x08);} else {RFSERIAL.write(0x09);} //Length
    RFSERIAL.write(paramADDH);      //00H ADDH
    RFSERIAL.write(paramADDL);      //01H ADDL
    if (EM!=EM_E220_400T30D) RFSERIAL.write(paramNETID); //02H NETID
    RFSERIAL.write(paramREG0SPED);  //03H/02H REG0 UART.765,PARITY.43,AIR.210
    RFSERIAL.write(paramREG1OPT);   //04H/03H REG1 SUB.76,RSSI.5,NA.432,TX.01
    RFSERIAL.write(paramREG2CHAN);  //05H/04H REG2 FREQ.76543210
    RFSERIAL.write(paramREG3);      //06H/05H REG3 RSSI.7,FIX.6,NA.5,LBT.4,NA.3,WOR.210
    RFSERIAL.write(paramCRYPTH);    //07H/06H CRYPT_H Key-byte
    RFSERIAL.write(paramCRYPTL);    //08H/07H CRYPT_L Key-byte
    
  } else if (EMS==EMS_E32) {
    RFSERIAL.write(0xC0);           //b[0]=C0(SAVE) + 5 bytes parameters
    RFSERIAL.write(paramADDH);      //b[1]
    RFSERIAL.write(paramADDL);      //b[2]
    RFSERIAL.write(paramREG0SPED);  //b[3]=SPED   Parity.76,UARTSPD.543,AIRDR.210
    RFSERIAL.write(paramREG2CHAN);  //b[4]=CHAN   NA.765,CHAN.43210
    RFSERIAL.write(paramREG1OPT);   //b[5]=OPTION FIX.7,IODR.6,WAKE.543,FEC.2,TX.10   
  }
  RFSERIAL.flush();
  GetParam(true);       // Reload Parameters after set which replies with new parameters so skip request.
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//---------------------------------------------------------------------------------------------------------------------
void Ebyte::GetParam(bool _SkipReq) {                                         DBENTERAL(("Ebyte::GetParam"),(_SkipReq))
  
  // Check Radio is Known and Radio is in SETMODE
  byte CurrentMode=Mode();if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::GetParam() EM<=EM_UNKNOWN")) return; }
  
  if (!_SkipReq) { // Request Parameters
    if (EMS==EMS_E220E22) {     RFSERIAL.write(0xC1); //Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
                                RFSERIAL.write(0x00); //Start-Reg
      if (EM==EM_E220_400T30D) {RFSERIAL.write(0x08);DBINFOL(("Ebyte::GetParam Sending C1 00 08"))} 
      else {                    RFSERIAL.write(0x09);DBINFOL(("Ebyte::GetParam Sending C1 00 09"))} //Length

    } else if (EM==EM_E32_433T20D||EM==EM_E32_433T30D) {
                                RFSERIAL.write(0xC1);RFSERIAL.write(0xC1);RFSERIAL.write(0xC1); 
                                DBINFOL(("Ebyte::GetParam Sending C1 C1 C1"))
    }
  }
  // Parse Reply
  EParamGotBytes=0;bool ReplyOk=true;
  unsigned long StartMillis = millis();
  while ( millis() - StartMillis < 200 ) {
    while ( RFSERIAL.available() > 0 ) {
      byte b=RFSERIAL.read();DBINFOAAL(("Ebyte::GetParam [?]=?"),(EParamGotBytes),(b,HEX))
      if (ReplyOk) {
        if (EMS==EMS_E220E22) {
          if (EParamGotBytes<=4) {
            switch(EParamGotBytes) {      //b[0]=Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
              case 0: if (b!=0xC1) {ReplyOk=false;DBERRORAL(("Ebyte::GetParam No 0xC1 reply[0]="),(b,HEX))} break;
              case 1: break;              //b[1]=StartReg ( 0x00(00H) to 0x07(07H) )
              case 2: break;              //b[2]=Length ( Number of Parameter-bytes (0-8) )
              case 3: paramADDH=b; break; //b[3]=00H ADDH
              case 4: paramADDL=b; break; //b[4]=01H ADDL
            }
          } else {
            byte idx=0;
            if (EM==EM_E220_400T30D) {idx=EParamGotBytes+1;} else {idx=EParamGotBytes;}
            switch(idx) {
              case 5: paramNETID=b; break;    //b[5]=02H NETID (NOT on E220)
              case 6: paramREG0SPED=b; break; //b[5/6]=03H REG0 UART.765,PARITY.43,AIR.210 (same as E220)
              case 7: paramREG1OPT=b; break;  //b[6/7]=04H REG1 SUB.76,RSSI.5,NA.432,TX.10 (same as E220)
              case 8: paramREG2CHAN=b; break; //b[7/8]=05H REG2 FREQ.76543210 (same as E220)
              case 9: paramREG3=b; break;     //b[8/9]=06H REG3 RSSI.7,FIX.6,REPEATER.5,LBT.4,WOR.3,WOR.210 (Repeater bit)
              case 10:paramCRYPTH=b; break;   //b[9/10]=07H CRYPT_H Key-byte
              case 11:paramCRYPTL=b; break;   //b[10/11]=08H CRYPT_L Key-byte
            }
          }

        } else if (EMS==EMS_E32) {
          switch(EParamGotBytes) {          //b[0]=0xC0
            case 0: if (b!=0xC0) {ReplyOk=false;DBERRORAL(("Ebyte::GetParam No 0xC0 reply[0]="),(b,HEX))} break;
            case 1: paramADDH=b; break;     //b[1]=ADDH
            case 2: paramADDL=b; break;     //b[2]=ADDL
            case 3: paramREG0SPED=b; break; //b[3]=SPED   Parity.76,UARTSPD.543,AIRDR.210
            case 4: paramREG2CHAN=b; break; //b[4]=CHAN   NA.765,CHAN.43210
            case 5: paramREG1OPT; break;    //b[5]=OPTION FIX.7,IODR.6,WAKE.543,FEC.2,TX.10
          }
        }
        if (ReplyOk) EParamGotBytes++;
      } else {DBINFOAAL(("Ebyte::GetParam !ReplyOk[?]=?"),(EParamGotBytes),(b,HEX))}
    }
  }
  DBINFOAL(("Ebyte::GetParam EParamGotBytes="),(EParamGotBytes))
  if (!_SkipReq) Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//---------------------------------------------------------------------------------------------------------------------
void Ebyte::Mode(byte _Mode) {
  if (EM<=EM_UNKNOWN) {DBERRORL(("Ebyte::Mode(SET) EM<=EM_UNKNOWN")) return; }
  if (EM==EM_E22_400T33D || EM==EM_E22_400T30D || EM==EM_E22_900T30D ) {
    if (      _Mode == ERFNORMMODE) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 0);}
    else if ( _Mode == ERFWAKEMODE) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 1);}
    else if ( _Mode == ERFPWRSAVE ) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 1);}
    else if ( _Mode == ERFSETMODE ) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 0);}
  }
  else if (EM==EM_E32_433T20D || EM==EM_E32_433T30D || EM_E220_400T30D ) {
    if (      _Mode == ERFNORMMODE) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 0);}
    else if ( _Mode == ERFWAKEMODE) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 0);}
    else if ( _Mode == ERFPWRSAVE ) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 1);}
    else if ( _Mode == ERFSETMODE ) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 1);}
  }
  delay(40); // Radio will not respond faster than this.
}
//-----------------------------------------------------------------------------------------------------
byte Ebyte::Mode() {
  if (EM<=EM_UNKNOWN) {/*DBERRORL(("Ebyte::Mode(GET) EM<=EM_UNKNOWN"))*/ return ERFNORMMODE; }
  int M1=digitalRead(PinM1);int M0=digitalRead(PinM0);
  if (EM==EM_E22_400T33D || EM==EM_E22_400T30D ) {
    if (M1==0 && M0==0) return ERFNORMMODE;
    if (M1==0 && M0==1) return ERFWAKEMODE;
    if (M1==1 && M0==1) return ERFPWRSAVE;
    if (M1==1 && M0==0) return ERFSETMODE;
  }
  else if (EM==EM_E32_433T20D || EM==EM_E32_433T30D || EM_E220_400T30D ) {
    if (M1==0 && M0==0) return ERFNORMMODE;
    if (M1==1 && M0==0) return ERFWAKEMODE;
    if (M1==0 && M0==1) return ERFPWRSAVE;
    if (M1==1 && M0==1) return ERFSETMODE;
  }
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::CheckRadio() {                                              DBENTERL(("Ebyte::CheckRadio"))
  if ( EM==EM_NOTGOT ) {
    // Radio Auto-Discovery ForE22
    // REPLY Should have 3HEAD(C1 80 07) +7PINFO[80H-86H] = 10 total reply bytes
    // We use last PInfo-byte as extra-reply garbage bytes
    byte Pinfo[11]={0,0,0,0,0,0,0,0,0,0,0};byte byteIdx=0; 
    DBINFOL(("Ebyte::CheckRadio ForE22 SENDING C1 80 07 @ M1=1,M0=0"))
    digitalWrite(PinM1, HIGH);digitalWrite(PinM0, LOW);delay(50);
    RFSERIAL.write(0xC1);RFSERIAL.write(0x80);RFSERIAL.write(0x07);RFSERIAL.flush();
    unsigned long StartMillis = millis();
    while ( millis() - StartMillis < 200 ) { 
      while ( RFSERIAL.available() > 0 ) {
        Pinfo[byteIdx]=byte(RFSERIAL.read());
        DBINFOAA(("(idx)=?"),(byteIdx),(Pinfo[byteIdx],HEX))
        if (byteIdx<10) byteIdx++;
      } if (byteIdx==10) break; 
    } DBINFOAL(("Ebyte::CheckRadio ForE22 END-REPLY Duration"),(millis() - StartMillis))

    // Parse Reply
    if ( Pinfo[0]==0xC1 && Pinfo[1]==0x80 && Pinfo[2]==0x07 ) { // E22 3-byte HEAD
      if ( Pinfo[3]==0x00 && Pinfo[4]==0x22 && Pinfo[5]==0x10 && Pinfo[6]==0x1E && Pinfo[7]==0x0B ) {
        EM=EM_E22_900T30D;EMS=EMS_E220E22;
        DBINFOL(("Ebyte::CheckRadio ForE22 Discovered EM=EM_E22_900T30D"))
      }
      if ( Pinfo[3]==0x00 && Pinfo[4]==0x22 && Pinfo[5]==0x40 && Pinfo[6]==0x21 && Pinfo[7]==0x0A ) {
        EM=EM_E22_400T33D;EMS=EMS_E220E22;
        DBINFOL(("Ebyte::CheckRadio ForE22 Discovered EM=EM_E22_400T33D"))
      }
    }
  }
  //------------------------------------------------------------------------------------------------------
  if ( EM==EM_NOTGOT ) {
    // Radio Auto-Discovery ForE32
    DBINFOL(("Ebyte::CheckRadio ForE32 SENDING C3 C3 C3 @ M1=1,M0=1"))
    digitalWrite(PinM1, HIGH);digitalWrite(PinM0, HIGH);delay(50);
    RFSERIAL.write(0xC3);RFSERIAL.write(0xC3);RFSERIAL.write(0xC3);RFSERIAL.flush();
    byte Pinfo[5]={0,0,0,0,0};byte byteIdx=0; // PI[0-9]=Product Information bytes
    unsigned long StartMillis = millis();
    while ( millis() - StartMillis < 200 ) {
      while ( RFSERIAL.available() > 0 ) {
        Pinfo[byteIdx]=byte(RFSERIAL.read());
        DBINFOAA((" (idx)=?"),(byteIdx),(Pinfo[byteIdx],HEX))
        if (byteIdx<4) byteIdx++;
      } if (byteIdx==4) break; 
    }
    DBINFOAL(("Ebyte::CheckRadio ForE32 END-REPLY Duration"),(millis() - StartMillis))

    // Parse Reply
    if ( Pinfo[0]==0xC3 ) { // E32 1-byte HEAD
      if        ( Pinfo[1]==0x32 ) {  // Is 433MHz
        // 1E is probably equal to a 30D
        if (Pinfo[2]==0x48&&Pinfo[3]==0x14){EM=EM_E32_433T20D;EMS=EMS_E32;DBINFOL(("Ebyte::CheckRadio FOUND EM=EM_E32_433T20D"))}
        if (Pinfo[2]==0x44&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBINFOL(("Ebyte::CheckRadio FOUND EM=EM_E32_433T30D [E32-TTL-1W]"))} //410-441Mhz,YBT17093321
        if (Pinfo[2]==0x49&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBINFOL(("Ebyte::CheckRadio FOUND EM=EM_E32_433T30D [Pre1B](TTL-1W)"))} //SN:W020987S00846 Pre-1B
        if (Pinfo[2]==0x51&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBINFOL(("Ebyte::CheckRadio FOUND EM=EM_E32_433T30D [1B]"))} //SN:W020583S02336
        if (Pinfo[2]==0x10&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBINFOL(("Ebyte::CheckRadio FOUND EM=EM_E32_433T30D [V8]"))}
      } else if ( Pinfo[1]==0x38 ) {  // Is 470MHz
      } else if ( Pinfo[1]==0x45 ) {  // Is 868MHz
      } else if ( Pinfo[1]==0x44 ) {  // Is 915MHz
      } else if ( Pinfo[1]==0x46 ) {  // Is 170MHz
      } //close[1]
    } //close[0]
  }
  //------------------------------------------------------------------------------------------------------
  if ( EM==EM_NOTGOT ) {
    // Radio Auto-Discovery ForE220
    // Send C1 08 03
    // REPLY Should have 3HEAD(C1 08 03) +3PINFO[08H-0AH] = 6 total reply bytes
    // We use last PInfo-byte as extra-reply garbage bytes
    byte Pinfo[7]={0,0,0,0,0,0,0};byte byteIdx=0; 
    DBINFOL(("Ebyte::CheckRadio ForE220 SENDING C1 08 03 @ M1=1,M0=1"))
    digitalWrite(PinM1, HIGH);digitalWrite(PinM0, HIGH);delay(50);
    RFSERIAL.write(0xC1);RFSERIAL.write(0x08);RFSERIAL.write(0x03);RFSERIAL.flush();
    unsigned long StartMillis = millis();
    while ( millis() - StartMillis < 200 ) { 
      while ( RFSERIAL.available() > 0 ) {
        Pinfo[byteIdx]=byte(RFSERIAL.read());
        DBINFOAA(("(idx)=?"),(byteIdx),(Pinfo[byteIdx],HEX))
        if (byteIdx<6) byteIdx++;
      } if (byteIdx==6) break; 
    } DBINFOAL(("Ebyte::CheckRadio ForE220 END-REPLY Duration"),(millis() - StartMillis))

    // Parse Reply
    if ( Pinfo[0]==0xC1 && Pinfo[1]==0x08 && Pinfo[2]==0x03 ) { // E220 3-byte HEAD + 3-byte Prod Info
      if ( Pinfo[3]==0x20 && Pinfo[4]==0x0A && Pinfo[5]==0x1E ) {EM=EM_E220_400T30D;EMS=EMS_E220E22;DBINFOL(("Ebyte::CheckRadio FOUND EM=EM_E220_400T30D [V1.2]"))}
    }
  }
  //------------------------------------------------------------------------------------------------------
  #if defined(FORCERADIOMODEL)
    EM=FORCERADIOMODEL;
  #endif
  if (EM==EM_NOTGOT) {
    EM=EM_UNKNOWN; DBERRORL(("EM=EM_UNKNOWN"))
    digitalWrite(PinM1, LOW);digitalWrite(PinM0, LOW);delay(40);
    return; // Exit: No Radio Found.
  } else {
    GetParam(); // Check Found Radio Register Settings
    DBINFOAL(("Ebyte::CheckRadio Radio Freq[mhz]=?"),(Frequency()))
    DBINFOAL(("Ebyte::CheckRadio AirSpeed=?"),(AirSpeed()))
    DBINFOAL(("Ebyte::CheckRadio Radio Power[db]=?"),(TxPower()))

    unsigned int RFAddress=Address();
    unsigned int EETode0RFAddress=EEPROM.get(0, EETode0RFAddress); //TodeIndex*AEB_TODEALLOC
    DBINFOAL(("Ebyte::CheckRadio Radio RFAddress=?"),(RFAddress,HEX))
    DBINFOAL(("Ebyte::CheckRadio EEPROM RFAddress=?"),(EETode0RFAddress,HEX))
    if (RFAddress!=EETode0RFAddress) {
      EEPROM.put(0,RFAddress);
      DBINFOL(("Ebyte::CheckRadio RFAddress!=EETode0RFAddress SET-EEPROM.put(0, RFAddress)"))
    }

    byte uartspd = UARTSpeed(); // E220&E22 Speed then Parity, E32 Parity then Speed
    DBINFOAL(("Ebyte::CheckRadio UARTSpeed=?"),(uartspd))
    if(EMS==EMS_E220E22&&uartspd!=12) {
      UARTSpeed(12);
      DBINFOL(("Ebyte::CheckRadio EMS==EMS_E220E22&&uartspd!=12 SET UARTSpeed(12)"))
    }

    bool fixedtrans = FixedTransmission();
    if (fixedtrans) {;DBINFOL(("Ebyte::CheckRadio FixedTransmission=true"))}
    else {FixedTransmission(true);DBINFOL(("Ebyte::CheckRadio FixedTransmission=false SET FixedTransmission(true)"))}
  }
  Mode(ERFNORMMODE);
}
//---------------------------------------------------------------------------------------------------------------------
bool Ebyte::PacketAvailable() {                                        // Check for incoming communications
  if ( EM==EM_UNKNOWN ) return;   // No Radio, No Packet to Check.
  
  // Check that Radio is in Normal Mode
  if ( Mode() != ERFNORMMODE ) {
    DBINFOL(("Ebyte::PacketAvailable() Mode() != ERFNORMMODE"))
    return false;
  }

  // Check if data exists
  if ( RFSERIAL.available() == 0 ) return false;
  DBINFOAL(("Ebyte::PacketAvailable - RFSERIAL.available() = "),(RFSERIAL.available()));
  
  if ( Packet != 0 ) {                                                    // Expire Packet build after 1-Second
    if ( millis() - Packet->PacketStartTimeMS > 5000 ) { 
      DBINFOL(("Ebyte::PacketAvailable PACKET EXPIRED > 1s"))
      delete Packet; Packet = 0; 
    }
  }
  if ( Packet == 0 ) {                                                    // Create new packet if needed
    Packet = new RxPacket();                   
    Packet->PacketStartTimeMS = millis();
  }

  while ( RFSERIAL.available() ) {                                         // Read RF Data
    if ( Packet->RxByte(RFSERIAL.read()) ) {                               // Returns True when packet size reached
      DBINFOL(("Ebyte::PacketAvailable FULL Packet Received"))
      if ( !Packet->IsValid() ) {
        DBINFOL(("Ebyte::PacketAvailable NOT VALID")) 
        delete(Packet); Packet=0; return false;                           // Return 'false' for InValid Packet
      }
      DBINFOL(("Ebyte::PacketAvailable Packet->IsValid"))
      return true;                                                        // Return 'true' for Valid Packet
    }
  }
  return false;
}
//-----------------------------------------------------------------------------------------------------
void Ebyte::Send(TxPacket* Tx) {
  DBENTERAL(("****************Ebyte::Send(Tx)*********************"),(int(Tx),HEX))
  int i=0;
  
  // Prep & Check
  if ( Tx == 0 ) return;
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if ( Mode() != ERFNORMMODE ) { DBERRORAL(("Ebyte::Send(Tx) Radio in Mode : "),(Mode())) return; }

  // Wait for not-busy
  RFSERIAL.flush();
  while ( digitalRead(PinAUX) == LOW ) { };
  
  // Set FromRF
  unsigned int FromRF = Address();
  if ( FromRF == 0xFFFF ) { DBERRORL(("Ebyte::Send(Tx) FromRF=0xFFFF")) return; }
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
      if ( Tx->ExtraBytes == 0 ) { DBERRORAL(("Ebyte::Send Tx->ExtraBytes == 0 @i: "),(i)) break; }
      else {
        if ( Tx->ExtraBytes->isEmpty() ) { DBINFOL(("Ebyte::Send Tx-ExtraBytes->isEmpty()")) break; }
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