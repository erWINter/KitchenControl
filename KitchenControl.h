#ifndef _KitchenControl_h
#define _KitchenControl_h

#include <Arduino.h>

//              Manuels Kitchen Control S/W             August 2019
//              ===========================

// Hier stehen im ersten Abschnitt einige Konstanten zum Anpassen

const int PRESENZ_DAUER = 120;  // Sekunden nachleuchten
// d.h. nach 120 Sekunden ohne PIR Detection ist alles wieder dunkel

// definiere maximale PWM fuer WW-LEDs (max. 255!)
const int PWM_Tag   = 255;      // volle Helligkeit am Tag
const int PWM_Nacht = 127;      // reduzierte Helligkeit nachts

// definiere Breite der Laola Welle in cm
#define LOALA_BREITE_CM 100

// define time to change direction every 
//  1st(0),2nd(1),4th(2),8th(3) or 16th(4) cycle
#define LAOLA_CYCLE 2

//--------------------------------------------------------------------
// Adafruit Feather nRF52 Bluefruit (512 KB flash, 64 KB SRAM)
// available Feather 3V3 pins:
//      TX SCK MOSI MISO SDA SCL
//-------------------------------------------------------------------------------

// define all used GPIO Pins (3.3V):
// some are using isolating Opto-Couplers: OC

// analog pins
const int PIN_IRMOTION[3] = { A0, A1, A2 }; // for IR-Motion reading (via OC)
const int PIN_PWRVOLTS[2] = { A4, A5 };     // measure PowerSupply Voltages 5V & 12V
const int PIN_SOLARCELL   = A3;             // measure light brightness

// PWM outputs for dimming the WW-LED strips (via OC to MOSFET)
const int PIN_WWLED_PWM[2] = { 11, 7 };     // must be PWM possible pins

// digital outputs
const int PIN_PWRSUP_ON[2] = { 27, 31 };    // switch 5V-/12V-PowerSupplies ON (via OC)

const int PIN_NEOPIXEL = 30;   // output steering the NeoPixel stripe
// the NeoPixel output must be converted via Level-Shifter from 3.3V to 5V !!

const int PIN_RED_LED  = 13;   // onboard red LED

// the internal AD-convertor has 3.3V reference
#define CONVERT_mV2adc(mV) (int)(((mV)*1023UL)/3300)

//--------------------------------------------------------------------
// define NeoPixel information

// define macro to convert cm to number of pixels (60 LEDs per meter)
#define CONVERT_cm2pixels(cm) (((cm)*60)/100)

const int NEOPIXEL_NUM = 300;   // 60*5m number of total NEO-pixels:

// adapt to used NeoPixel type
#define NEO_TYPE  (NEO_GRB + NEO_KHZ800)

// setup NEO Pixel strip
Adafruit_NeoPixel strip = 
            Adafruit_NeoPixel(NEOPIXEL_NUM, PIN_NEOPIXEL, NEO_TYPE);

// ATTENTION: strip pixel indexing is from Right to Left (see LedFromN[5])
// runing 

// define some often used RGB-strip colors
const uint32_t RGB_BLACK = 0x000000;
const uint32_t RGB_WHITE = 0xFFFFFF;     // full bright !
const uint32_t RGB_GRAY  = 0x777777;

// define maximal brightness for day and night
#define MAX_RGB_BRIGHT_DAY      255
#define MAX_RGB_BRIGHT_NIGHT    127

// this structure is used to hold cabinet LED position
struct sCABINET { 
    int fromPos;        // Pixel-index 0..(NEOPIXEL_NUM-1)
    int nLEDs;          // nr.of Pixels in this cabinet/section 
};

const sCABINET LedFromN[8] = {
//                      #=> Cabinet-Nr (or index) 1-7
//fromPos,nLEDs
//      V,V
    {   0,NEOPIXEL_NUM },   // non-existing Cabinet-Nr 0 means ALL LEDs
                    // Cabinet-Nr
                    // # Breite
    { 150, 36 },    // 1 60cm Ofen Auszieh-Schublade
    { 102, 48 },    // 2 80cm
    {  54, 48 },    // 3 80cm
    {  30, 24 },    // 4 40cm
    {   0, 30 },    // 5 50cm (Einspeisung Kuehlschrank)

    { 234, 24 },    // 6 40cm
    { 186, 48 }     // 7 80cm Spuele
};

// need to know the cabinet order
const int CabinetOrderR2L[] = { 5,4,3,2,1,7,6 };
const int CabinetOrderL2R[] = { 6,7,1,2,3,4,5 };

// include KitchenEffects.h (library with RGB effects)
#include "KitchenEffects.h"

int getLOC0();      // defined in MAIN
int getLOC1();      // defined here at end
//######################################################################################
String infoLOC( String text)
{
    text = "\n";
    text += "FILE: " __FILE__ "\n";
    text += "DATE: " __DATE__ "  TIME: " __TIME__ "\n";
    text += "LOC= ";
    int loc1 = getLOC1();           // KitchenControl.h LOC
    int loc2 = getLOC2();           // KitchenEffects.h LOC
    int loc0 = getLOC0();           // Main LOC
    int LOC = loc1 + loc2 + loc0;
    
    text += LOC;
    text += "\t; total (main & both includes)";
    return text;
}

// this function must always be at the end of source code
int getLOC1() { return __LINE__; }
#endif
