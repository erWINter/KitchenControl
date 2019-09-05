#ifndef _KitchenEffects_h
#define _KitchenEffects_h

//----------------------------------------------------------------------
// define subroutines enabling different RGB strip effects:

// Walker effect: fast back- and forth-running single LED at open drawer 
int FastWalkerIndex( int cabNr, char dpOMU)
{
    int ledFrom = LedFromN[cabNr].fromPos;
    int ledNums = LedFromN[cabNr].nLEDs;

    // walker (white pixels from left to right and vice versa)
    int nWalk = -1;

static struct {  
    int32_t  Cnt;
    int      Dir;
    int      CDold;    // eg. 1118
} fw = { 0,0,0 };

    int currCD = 1000*cabNr + (int)dpOMU;   // drawer-specific!
    if (fw.CDold != currCD) {
        // drawer changed !
        fw.CDold = currCD;
        fw.Cnt = int( (ledFrom+ledNums-1)/2);    // start in the middle
        fw.Dir = 1;          // to right
    }

    if (fw.Dir == 1) {
        nWalk = fw.Cnt;
        fw.Cnt++;           // next right
        if (fw.Cnt - ledFrom > ledNums) {
            fw.Cnt--;
            fw.Dir = -1;    // back to left
        }
        return nWalk;
    }
    
    if (fw.Dir == -1) {
        nWalk = fw.Cnt;
        fw.Cnt--;           // next left
        if (fw.Cnt < ledFrom) {
            fw.Cnt++;
            fw.Dir = 1;     // back to right
        }
        return nWalk;
    }
    return -1;
}

// Laola_Welle rundherum ueber alle RGB-LEDs
void run_Laola_Wave()
{
    // current center postion
    static int currLaolaCenter = NEOPIXEL_NUM/2;
    static int dirLaola = 1;    // zuerst nach rechts
    static uint8_t CntLaolas = 0;

    CntLaolas++;        // 0..255
/*
        0b000,  // black
        0b001,  // red
        0b010,  // green
        0b100,  // blue
        0b101,  // RB
        0b
 */    
    const int pixWidth = CONVERT_cm2pixels( LOALA_BREITE_CM) / 2;     // eg. 60/2 = 30
    uint8_t mask4RGB = CntLaolas & 7;   // 0..7

    int n = -1;
    while (++n < pixWidth) {
        int x = 255 - (int)(256*n/pixWidth);        
        int pwmSine = _NeoPixelSineTable[x];
        if (mask4RGB==3 || mask4RGB>=5) {
            pwmSine /= mask4RGB < 7 ? 2 : 3;
        }
        uint32_t RGB_Laola = 0;
        for (int b = 0; b < 2; b++) {
            if (mask4RGB & (1<<b)) {
                RGB_Laola |= pwmSine << (b<<2);
            }
        }
        if (n+1 >= pixWidth) {
            RGB_Laola = 0;          // force last to be black!      
        }
        strip.setPixelColor( (currLaolaCenter-n)%NEOPIXEL_NUM, RGB_Laola);   // left
        strip.setPixelColor( (currLaolaCenter+n)%NEOPIXEL_NUM, RGB_Laola);   // right
    }
    
    currLaolaCenter += dirLaola + NEOPIXEL_NUM;
    currLaolaCenter %= NEOPIXEL_NUM;        // wrap around

    // change direction every 1st,2nd,4th,8th or 16th cycle
    int MCx = LAOLA_CYCLE;    // = 0-4 every 4th cyle
                        //    0, 1, 2, 3, 4
const uint8_t MaskCycle[] = { 0, 1, 3, 7, 15 };

    uint8_t maskDirChg = 7 | (MaskCycle[MCx]<<3);

    if ((CntLaolas & maskDirChg) == (maskDirChg^7)) {
        // change Laola-Direction when Laola is gone
        dirLaola *= -1;
    }
}

// this function must always be at the end of source code
int getLOC2() { return __LINE__; }
#endif
