// Board: Adafruit Bluefruit nRF52 Feather
//              Manuels Kitchen Control S/W             August 2019
//              ===========================
// List of external H/W connections:
//
// 3 Digital Inputs for 3 IR Motion Detectors Type HC-SR501
//          the connector has 3 pins: [Gnd|OutDetect|+5V]
//  (use 3 opto-couplers)
//
// 2 Digital Outputs to switch-on the 12V- and 5V-power supplies
// 2 Analog Inputs to measure the 12V and 5V
//
// 2 PWM Outputs to control the 2 WW LED stripe dimming via 2 MOS-FETs
// (use 2 fast opto-couplers, which control 2 Power MOS-FETs to dim up to 6A/12V)
//
// 1 RX Serial Input Line signalling open drawers from 3 Trinket PROs
//  (use 1 opto-coupler)
//      Baudrate:   19200
//      Protocol: "CxPO<NL>"  (PO means Percentage Open: 00-99)
//                  x could be 'o', 'm' or 'u' for oben, mitte or unten
//                 C is the Cabinet number 1-7
//            eg. "7o99<NL>"  (one drawer in cabinet 7 oben if fully open)
//            or  "1u20 3m30<NL>"  (2 open: cabinet 1 unten, 3 mitte)
//
// 1 Digital Output to control the NeoPixel RGB-stripe
//  (use 1 level-converter from 3V3 to 5V)
//
// 1 Analog Input to a Solar Cell for measuring the ambient kitchen brightness
//          This allows to use lower light during night time
//
//######################################################################################
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678
//       1         2         3         4         5         6         7         8        

#include <Adafruit_NeoPixel.h>

#define DEBUG

// put all definitions and fix (const) variable declarations into include file
#include "KitchenControl.h"

//--------------------------------------------------------------------
// macro definition
#define DEBUG_PRINT( var) Serial.print( #var "= "); Serial.println( var);

// if BLINKING is defined, then onboard LED could signal some events
// #define BLINKING // TBD!!

//####################################################################

//--------------------------------------------------------------------
// global used variables:

const uint32_t REDBLK_FREQ[8] = {
    0x00000000,     // 0 no blinking
    0x000F0000,     // 1 slowest 1on/7off blinking
    0x00FFFF00,     // 2 slowest 50on/50off blinking
    0x00FF00FF,     // 3 slow 50on/50off blinking
    0x0F0F0F0F,     // 4 fast 50on/50off blinking
    0x33333333,     // 5 faster 50on/50off blinking
    0x11111111,     // 6 faster 25on/75off blinking
    0xFFFFFFFF      // 7 permanent 100on    
};

uint32_t MaskRedBlink = REDBLK_FREQ[2];

int delayXmsCounter = 0;
int PowerOnCounter = 0;
int PWMforWWstrip = 0;

// Licht-Effekte
const uint32_t ME4Walker    = 1<<0;    // W Walker running on open drawer
const uint32_t ME4LaolaWave = 1<<1;    // Laola-Wave if no open drawer

static uint32_t Mask4Effects = 0;

#define RUN_EFFEKT(m) ((Mask4Effects & (m)) != 0)
//------------------------------------------------------------------

// define 500mV day/night ADC-level
const int ADC_NIGHT = CONVERT_mV2adc( 500);
int     MaxWW_PWM = PWM_Tag;
bool    IsDay = true;

String drawerMessage = "";              // gather serial input

//###################################################################
void setup()        // put your setup code here, to run once at begin
{
    setRedLED( true);
    /////////

    // start RGB NeoPixels named 'strip'
    strip.begin();
    strip.show();               // start with all cleared NeoPixels
    // set maximal brightness for day
    strip.setBrightness( MAX_RGB_BRIGHT_DAY);

    // disable PWM for Warm White stripes
    analogWrite( PIN_WWLED_PWM[0], 0);      // set to 0 Volt
    analogWrite( PIN_WWLED_PWM[1], 0);      // set to 0 Volt

    // init Serial RX to get any open drawer messages
    Serial.begin( 19200);       // use same baudrate as DrawerDetect.ino

    // 2 outputs to control the 5V- and 12V-power supplies
    for (int i=0; i < 2; i++) {
        pinMode( PIN_PWRSUP_ON[i], OUTPUT);
        digitalWrite( PIN_PWRSUP_ON[i], LOW);   // Power off at startup
    }

    // 3 IR detectors HC-SR501 inputs
    for (int i=0; i < 3; i++) {
        pinMode( PIN_IRMOTION[i], INPUT);      // IR human presence detection pin
        digitalWrite( PIN_IRMOTION[i], LOW);   // HIGH is active, so pull down to LOW
    }

    while (!Serial) {        // idle, until Serial is available
        delay( 1);
    }

    // print out information to identify program/code
    Serial.println( infoLOC( "Manuels KitchenControl") );

    setRedLED( false);
    /////////

    // switch on the two effects Walker and Laola-Wave
    Mask4Effects |= ME4Walker;

    Mask4Effects |= ME4LaolaWave;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void loop()             // put your main code here, to run repeatedly
{
const int      EveryXms = 20;                // main-loop repetition 50Hz
    loopDelay( EveryXms);
//  ########

    // with help of the 3 IR Motion Detectors check for any present humans
    bool humanDetected = detectIR_Motion( &delayXmsCounter, PRESENZ_DAUER *100);
//                       ###############
    if (humanDetected) {
        // somebody is present: activate Kitchen

        // first powerup both power supplies
        digitalWrite( PIN_PWRSUP_ON[0], HIGH);
        digitalWrite( PIN_PWRSUP_ON[1], HIGH);

        // the power supplies need some millisecs to provide full power
        PowerOnCounter++;
        if (PowerOnCounter < 12345) {    // dummy TBD
            // check if voltages are available
            int adc__5V = analogRead( PIN_PWRVOLTS[0]);
            int adc_12V = analogRead( PIN_PWRVOLTS[1]);
        
            if ((adc__5V > 512)
             && (adc_12V > 512)) {
                // now both power-supplies are ready
                PowerOnCounter = 12345;    // dummy TBD
            }
        }
        else {
            PWMforWWstrip++;               // dim up WW LED stripes
        }
        
        // get (gather) any sent serial information from Drawer-Detection
        if (gotSerialInput( &drawerMessage)) {

            MaskRedBlink = REDBLK_FREQ[5];
            // we got complete message from DrawerControl S/W
            decodeMessage( drawerMessage);
        }
        else {
            MaskRedBlink = REDBLK_FREQ[4];
            // no drawer out
            if (RUN_EFFEKT( ME4LaolaWave)) {
                run_Laola_Wave();
                //////////////
            }
        }
    }
    else {
        // nobody here since PRESENZ_TIME seconds
        MaskRedBlink = REDBLK_FREQ[2];
        
        PWMforWWstrip -= 3;         // dim down WW LED stripes 3 times faster
        if (PWMforWWstrip < 0) {

            MaskRedBlink = REDBLK_FREQ[1];
            
            // switch both power supplies off
            digitalWrite( PIN_PWRSUP_ON[0], LOW);
            digitalWrite( PIN_PWRSUP_ON[1], LOW);
            PowerOnCounter = 0;

            // kitchen is now without power and light
            
            // only during the power-off period:
            //      measure permanently the brightness via Solar Cell
            IsDay = analogRead( PIN_SOLARCELL) > ADC_NIGHT;

            // set maximal PWM-value for Warm White stripes
            MaxWW_PWM = IsDay ? PWM_Tag : PWM_Nacht;

            strip.setBrightness( IsDay ? MAX_RGB_BRIGHT_DAY : MAX_RGB_BRIGHT_NIGHT);
        }
    }

    // set the PWM for both Warm-White 12V LED stripes:
    int pwmX = constrain( PWMforWWstrip, 0,255);
    
    // use gamma corrected brightness
    uint8_t pwmCorr = _NeoPixelGammaTable[pwmX];
    
    pwmCorr = map( pwmCorr, 0,255, 0,MaxWW_PWM);
    
    analogWrite( PIN_WWLED_PWM[0], pwmCorr);
    analogWrite( PIN_WWLED_PWM[1], pwmCorr);

    if (pwmX == 0) {
        strip.clear();          // Set all pixel colors to 'off'
    }
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

////////////////////////////////////////////////////////////////////////////////////////
// used special subroutines & functions (except setup and loop)
////////////////////////////////////////////////////////////////////////////////////////

//######################################################################################
// set onboard red LED on or off
void setRedLED( bool setOn)
{
    digitalWrite( PIN_RED_LED, setOn ? HIGH : LOW);
}

//######################################################################################
void loopDelay( int msDelay)
{
static uint8_t loopCounter = 0;

static uint32_t nextMillis = millis() + msDelay;
    int brake = msDelay;            // emergency brake
    while (millis() < nextMillis) {
        delay(1);
        if (brake-- < 0)
            break;
    }
    nextMillis = millis() + msDelay;

    // insert code for automated blinking
    loopCounter++;

#ifdef BLINKING
    if (MaskRedBlink == 0) {
        setRedLED( false);
        /////////
        return;
    }
    if (~MaskRedBlink == 0) {
        setRedLED( true);
        /////////
        return;
    }
    bool isOn = (MaskRedBlink & (1<<(31&(loopCounter>>3)))) != 0;
    setRedLED( isOn);
    /////////
#endif
}

//######################################################################################
bool detectIR_Motion( int *Delay, const int maxDelay)
{
    String irDetected = "";     // preset none detected
    
    for (int i=1; i <= 3; i++) {
        if (digitalRead( PIN_IRMOTION[i-1]) == HIGH) {
            irDetected += i;
        }
    }
    
    if (irDetected != "") {
        // now one or more IR detectors signal detection
        // irDetected eg. = "1" or = "123" if all 3 detected humans (or hungry cats)

        DEBUG_PRINT( irDetected)
        // BTW: showing which IR Detectors are responding
        // could be helpful to find the best adjusted position for the 3 IR Detectors

        // reset the hold-on timer to the predefined customized time.
        *Delay = maxDelay;
        return true;
    }

    // no human jumping around
    (*Delay)--;                 // count down for final shut-down
    
    if (*Delay < maxDelay/2) {

        // after half time, start reducing the brightness
        int mod = (maxDelay/2) / 256;
        if ((*Delay % mod) == 1) {
            PWMforWWstrip--;
        }
        return true;        // delay is active
    }
    
    // maxDelay is over
    *Delay = 0;
    return false;           // no humans present! (maybe cats ?)
}

//######################################################################################
bool gotSerialInput( String *msg)
{
static String drawerMessage = String( "");

    while (Serial.available()) {
        // append all available characters
        drawerMessage += String( Serial.read());
    }

    *msg = "";                      // wipe out returned message

    int len = drawerMessage.length(); 
    if (len < 5)
        return false;               // too short

    if ((len % 5) > 0)
        return false;               // not a multiple of 5

    *msg = drawerMessage;           // return the complete message
    drawerMessage = "";             // reset internal saving buffer
    return true;                    // an msg was set
}

//######################################################################################
void decodeMessage( String msg)
{
    // incoming message eg. "4u20<LF>"
    // or more eg. "1o09 2o50 4u10<LF>"

#ifdef DEBUG
    Serial.println( msg);
#endif
    char Z = msg.charAt( msg.length()-1);
    if (Z != '\n')
        return;         // last character is NOT newline

    // as we have 7 cabinets, all of them could have one open drawer
    int cabNr,percOpen;
    char drawerPos;                 // 'o', 'm', 'u'
    while (msg.length() >= 5) {
        String NxPO = msg.substring( 0, 4);
        if (decodeNxPO( NxPO, &cabNr,&drawerPos,&percOpen)) {
            //////////
            createRGBeffect( cabNr, drawerPos, percOpen);
            ///////////////
        }
        else {
            Serial.println( "-E- decodNxPO error:");
            DEBUG_PRINT( NxPO);
        }
        // finally remove already parsed info from msg string
        msg.remove( 5);
    }
}

//######################################################################################
// decodes one message "NxPO" sent from DrawerControl, 
// eg. "3m45" ( 3 is cabNr, drawerPos is 'm', percOpen is 45)
bool decodeNxPO( String NxPO, int *cabNr, char *drawerPos, int *percOpen)
{
const String DRAWER_POS = "omu";   // drawer-position-coding: oben/ mitte/ unten

    if (NxPO.length() != 4)
        return false;               // wrong length: must be 4!

    // NxPO eg.= "7o99" or "1m09" or "2u50"
    char N = NxPO.charAt( 0);
    if (! isDigit( N))
        return false;               // 1st character is not a digit 0-9

    char x = NxPO.charAt( 1);
    if ( DRAWER_POS.indexOf( x) < 0)
        return false;               // 2nd character not o m u

    *cabNr = (int)(N - '0');        // cabNr = 0-9
    *drawerPos = x;                 // = o,m or u

    int PO = 0;
    for (int i = 2; i <= 3; i++) {
        char d = NxPO.charAt( i);
        if (! isDigit( d))
            return false;           // 3rd/4th character is not a digit 0-9

        PO = 10*PO + (int)(d - '0');
    }
    
    *percOpen = PO;                 // finally set percentage open to 0-99
    return true;        // decoding was successful, set values are valid
}

//######################################################################################
// this routine gets the cabinet with the open drawer information
// and creates RGB effects with this info 
void createRGBeffect( int cabNr, char drawerPos, int percOpen)
{

    char line[80];
    sprintf( line, "cabNr= %d, drawerPos= %c, percOpen= %02d\n",
                    cabNr,     drawerPos,     percOpen);
    Serial.print( line);

    // get the section of the cabinet, where the LEDs are placed
    int ledFrom = LedFromN[cabNr].fromPos;
    int ledNums = LedFromN[cabNr].nLEDs;
    if (ledNums <= 0)
        return;
 
    // for demonstration and as a very first test
    // use the dedicated colors blue,green,red for each drawer
    // set the brightness according to length drawn-out

    // re-map percent to full PWM brightness
    int brightness = map( percOpen, 0,99, 0,255);

    int sRGB = 0;                       // unten red
    if (drawerPos == 'm') sRGB = 1;     // mitte green
    if (drawerPos == 'o') sRGB = 2;     // oben blue

    // pass shifted brightness to R(0), G(1) or B(2) position
    strip.fill( brightness << (sRGB<<2), ledFrom, ledNums);

    // post add overlayed effects
    if (RUN_EFFEKT( ME4Walker)) {
        // run white LED back and forth on open drawer
        int n = FastWalkerIndex( cabNr ,drawerPos);
                ///////////////
        if (n >= 0)
            strip.setPixelColor( n, RGB_WHITE);
    }
    
    strip.show();       // it's show time 
}

//######################################################################################

// this function must always be at the end of source code
int getLOC0() { return __LINE__; }  // main

//eoc   E_Winter@web.de started 2019-08-25
//.
