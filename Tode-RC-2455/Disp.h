/****************************************************************************************************************//**
 * @file    Disp.h
 * @brief   Itemize various Display differences
 *          Custom Fonts - https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
 *********************************************************************************************************************/
#ifndef _DISP_H
#define _DISP_H

#include "config.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <OneSlot9pt7b.h>             
#include <DroidSansMono9pt7b.h>

#define DSPCLASS Adafruit_ST7735
//#include <Fonts/FreeSansBold9pt7b.h>

//---[ OBJECT ]---------------------------------------------------------
static DSPCLASS oDisplay = DSPCLASS(DISPPINS);     //AnaKeyPad  TFT(cs, dc(rs)(cd), rst)

/******************************************************************************//**
 * @defgroup DSPCOLORS Display Colors
 *           Defined in Disp.h
 * @{
 *********************************************************************************/
#define BLACK 0x0000
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define CPINK 0xF81F
///@}
/******************************************************************************//**
 * @defgroup DSPSETTINGS Display Settings
 *           Defined in Disp.h
 * @{
 *********************************************************************************/
//---[ ASSIGNMENTS ]----------------------------------------------------
#define VALCOL 80             // Pixel Column In-Row Value Starts at
#define SCREENWIDTH 128
#define SCREENHEIGHT 160
#define ITEMCHARWIDTH 7       // Was 6 ( includes post char spacing )
#define ITEMCHARHEIGHT 14     // Was 13
#define TITLECHARHEIGHT 13
#define TITLECHARWIDTH 11
//---[ COMMANDS ]-------------------------------------------------------
#define DSP_BEGIN initR(INITR_BLACKTAB)
#define DSP_SETROTATION setRotation(2)
#define CLEARSCREEN fillScreen(0x0000)
#define TITLEFONT setFont(&DroidSansMono9pt7b)
#define ITEMFONT setFont(&OneSlot9pt7b)
#define SETCURSOR setCursor
//---[ FORMAT ]---------------------------------------------------------
#define TXTNORM       setTextColor(WHITE)
#define TXTSELT       setTextColor(YELLOW)
#define TXTSET        setTextColor(MAROON)
#define TXTEXPIRED    setTextColor(DARKGREY)
#define TXTRFGETTING  setTextColor(CPINK)
#define TXTRFGETFAIL  setTextColor(RED)
#define TXTRFGOT      setTextColor(GREEN)
#define BGNAMESELT    fillRect(XPos,YPos-ITEMCHARHEIGHT+2,VALCOL,ITEMCHARHEIGHT,NAVY)
#define BGNAMENORM    fillRect(XPos,YPos-ITEMCHARHEIGHT+2,VALCOL,ITEMCHARHEIGHT,BLACK)
#define BGVALSELT     fillRect(XPos+VALCOL,YPos-ITEMCHARHEIGHT+2,SCREENWIDTH-VALCOL,ITEMCHARHEIGHT,NAVY)
#define BGVST         fillRect(XPos+VALCOL,YPos-ITEMCHARHEIGHT+2,SCREENWIDTH-VALCOL,ITEMCHARHEIGHT,YELLOW)
#define BGVALNORM     fillRect(XPos+VALCOL,YPos-ITEMCHARHEIGHT+2,SCREENWIDTH-VALCOL,ITEMCHARHEIGHT,BLACK)
#define BGVALSELSTS   fillRect(XPos+VALCOL,YPos-ITEMCHARHEIGHT+2,SCREENWIDTH-VALCOL,ITEMCHARHEIGHT,NAVY)
//#define BOXVDIGIT 
///@}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



//_____________________________________________________________________________________________________________________
#endif
