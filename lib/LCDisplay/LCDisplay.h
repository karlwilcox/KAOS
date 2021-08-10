#ifndef LCDISPLAY_H
#define LCDISPLAY_H

#define LCDISPLAY_H

#include <LiquidCrystal.h>
#include "../Process/Process.h"
#include "../RTClock/RTClock.h"
#include <Arduino.h>

class LCDisplay : public Process {

    protected:
        int rs, en, d4, d5, d6, d7;
        LiquidCrystal *lcd;
        unsigned int state = LOW;
        static const int numItems = 5;
        char newsItems[LCDisplay::numItems][64];
        unsigned int curItem = 0;
        unsigned int curPos = 0;
        char line0[17];
        char line1[17];
        RTClock *clock;
        char headline[64];
        int clockCounter = 0;
        bool blink = true;

        void scrollNews();
        void updateDisplay();
        char *getHeadline();
        void updateClock();

    public:
        LCDisplay(int rsVal, int enVal, int d4Val, int d5Val, int d6Val, int d7Val );

        void setup();
        void action();

        void useClock(RTClock *clockPtr);

};


#endif // !LCDISPLAY_H
