#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "actions.h"
#include "devices.h"
#include "memory_map.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"
#include "devices.h"

extern byte deviceActions[];
// deviceActions is initialised to all OFF
// Then setupDevices populates ONLY those entries that have an action set
// Hence we can just traverse this array to find which devices
// are set to use each action

extern byte deviceStates[];

extern SimpleDHT11 *dht11;
extern char lcdLine0[], lcdLine1[];
extern LiquidCrystal *lcd;
/*
void doSample(unsigned int block) {
    switch (eepromRead(block,EEPROM_TYPE)) {
        case DEVICE_INPUT:
            stateWrite(block, STATE_VALUE, digitalRead(eepromRead(block, OFFSET_PIN1)));
            break;
        case DEVICE_ANALOG:
            stateWrite(block, STATE_VALUE, analogRead(eepromRead(block, OFFSET_PIN1)));
            break;
        case DEVICE_DHT11:
            dht11->read(eepromRead(block,OFFSET_PIN1), statePtr(block, STATE_DHT_TMPT),
                        statePtr(block, STATE_DHT_HMDT), NULL);
            break;
        default:
            break;
    }
}
*/
void updateDigital(unsigned int block, byte value) {
    unsigned int pin, SR_block;
    pin = eepromRead(block,EEPROM_PIN1);
    if (pin < 100) {
        digitalWrite(pin, value);
    } else { // this is a pin on a shift register
        SR_block = stateRead(DEVICE_BOARD, STATE_1ST_SR);
        pin -=101; // convert pins 1 to 8 to bits 0-7
        if (value >= 1) {
            if (pin < 8) { // 8 pins on first SR
                stateBitSet(SR_block, STATE_PARAM1,1 << pin);
            } else if (pin < 16) {
                stateBitSet(SR_block, STATE_PARAM2,1 << (pin - 8));
            } else if (pin < 24) {
                stateBitSet(SR_block, STATE_PARAM3,1 << (pin - 16));
            }
        } else {
            if (pin < 8) { // 8 pins on first SR
                stateBitClear(SR_block, STATE_PARAM1,1 << pin);
            } else if (pin < 16) {
                stateBitClear(SR_block, STATE_PARAM2,1 << (pin - 8));
            } else if (pin < 24) {
                stateBitClear(SR_block, STATE_PARAM3,1 << (pin - 16));
            }
        }
        writeDSR(SR_block);
    }
    stateWrite(block, STATE_VALUE, value);
}

void writeDSR(unsigned int block) {
    // how many shift registers are cascaded here?
    unsigned int numSRs = eepromRead(block, EEPROM_VALUE);
    if (numSRs > 3) numSRs = 3;
    // enable loading data
    digitalWrite(eepromRead(block,EEPROM_PIN3), LOW);
    // for each register, write out the state value
    do {
        shiftOut(eepromRead(block, EEPROM_PIN1), eepromRead(block, EEPROM_PIN2),
                 MSBFIRST, stateRead(block,STATE_PARAM1 + numSRs - 1));
    } while (--numSRs > 0);
    // disable loading data
    digitalWrite(eepromRead(block,EEPROM_PIN3), HIGH);
}

void updatePWM(unsigned int block, byte value) {
    unsigned int pin;
    pin = eepromRead(block,EEPROM_PIN1);
    if (value == 1) value = 255;
    analogWrite(pin, value);
    stateWrite(block, STATE_VALUE, value);
}


const char lyrics[] PROGMEM = "I am unwritten, can't read my mind * \
I'm undefined * \
I'm just beginning, the pen's in my hand * \
Ending unplanned * \
 * \
Staring at the blank page before you * \
Open up the dirty window * \
Let the sun illuminate the words that you could not find * \
 * \
Reaching for something in the distance * \
So close you can almost taste it * \
Release your inhibitions * \
 * \
Feel the rain on your skin * \
No one else can feel it for you * \
Only you can let it in * \
No one else, no one else * \
Can speak the words on your lips * \
Drench yourself in words unspoken * \
Live your life with arms wide open * \
Today is where your book begins * \
 * \
The rest is still unwritten * \
 * \
I break tradition, sometimes my tries * \
Are outside the line * \
We've been conditioned to not make mistakes * \
But I can't live that way * \
 * \
Staring at the blank page before you * \
Open up the dirty window * \
Let the sun illuminate the words that you could not find * \
 * \
Reaching for something in the distance * \
So close you can almost taste it * \
Release your inhibitions * \
 * \
Feel the rain on your skin * \
No one else can feel it for you * \
Only you can let it in * \
No one else, no one else * \
Can speak the words on your lips * \
Drench yourself in words unspoken * \
Live your life with arms wide open * \
Today is where your book begins * \
 * \
Feel the rain on your skin * \
No one else can feel it for you * \
Only you can let it in * \
No one else, no one else * \
Can speak the words on your lips * \
Drench yourself in words unspoken * \
Live your life with arms wide open * \
Today is where your book begins * \
 * \
The rest is still unwritten * \
 * \
Staring at the blank page before you * \
Open up the dirty window * \
Let the sun illuminate the words that you could not find * \
 * \
Reaching for something in the distance * \
So close you can almost taste it * \
Release your inhibitions * \
 * \
Feel the rain on your skin * \
No one else can feel it for you * \
Only you can let it in * \
No one else, no one else * \
Can speak the words on your lips * \
Drench yourself in words unspoken * \
Live your life with arms wide open * \
Today is where your book begins * \
 * \
Feel the rain on your skin * \
No one else can feel it for you * \
Only you can let it in * \
No one else, no one else * \
Can speak the words on your lips * \
Drench yourself in words unspoken * \
Live your life with arms wide open * \
Today is where your book begins * \
 * \
The rest is still unwritten * \
The rest is still unwritten * \
The rest is still unwritten * "; 


void updateLCD() { // we are called every 1/4 second, 
    static int count = 0;
    static unsigned int curPos = 0;
    char temp[10];
    unsigned int tmpt = 0, hour = 0, mins = 0, hmdt = 0, block;
                        
    if ((block = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK) != 0 ) && block != 255) {
        tmpt = stateRead(block,STATE_PARAM1);
        hmdt = stateRead(block,STATE_PARAM2);
    }

    if (count == 12) {
        // After 3 seconds display centigrade
        sprintf(temp, "%4d", (int)tmpt);
        lcdLine0[0] = temp[0];
        lcdLine0[1] = temp[1];
        lcdLine0[2] = temp[2];
        lcdLine0[3] = temp[3];
        lcdLine0[4] = 'C';    
        // after another 3 seconds display fahrenheit
        // update the clock and humidity
    } else if (count == 24) {
        if ((block = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK) != 0 ) && block != 255) {
            hour = stateRead(block,STATE_PARAM1);
            mins = stateRead(block, STATE_PARAM2);
        }
        sprintf(temp, "%4d", (((int)tmpt * 9) / 5) + 32);
        lcdLine0[0] = temp[0];
        lcdLine0[1] = temp[1];
        lcdLine0[2] = temp[2];
        lcdLine0[3] = temp[3];
        lcdLine0[4] = 'F';  
        sprintf(temp, "%02u", hour);
        lcdLine0[6] = temp[0];
        lcdLine0[7] = temp[1];
        sprintf(temp, "%02u", mins);
        lcdLine0[9] = temp[0];
        lcdLine0[10] = temp[1];
        sprintf(temp, "%3d", (int)hmdt);
        lcdLine0[12] = temp[0];
        lcdLine0[13] = temp[1];
        lcdLine0[14] = temp[2];
    }
    if (count++ > 24) count = 0;
    // but on every occaision we update the crawl
    for (int i = 0; i <= 15; i++) {
      lcdLine1[i] = lcdLine1[i+1];
    }
    lcdLine1[15] = pgm_read_byte_near(lyrics + curPos);
    if (++curPos >= strlen_P(lyrics)) {
      curPos = 0;
    }
    lcd->setCursor(0,0);
    lcd->print(lcdLine0);
    lcd->setCursor(0,1);
    lcd->print(lcdLine1); 
}

// A timer has expired, so do the required action and return a new time value
void doAction(unsigned int block) {
    unsigned int t1, t2; // temporary store
    byte b; // temporary store
    // Almost always need the current value, so get it
    byte value = stateRead(block, STATE_VALUE);

    // special handling for block 0
    if (block == 0) { // time has expired on system board, resume command processing
        stateBitClear(block, STATE_FLAG, FLAG_SLEEP);
        stateWrite(block, STATE_TTR, 0);
        return;
    }

    // What action do we need to do?
    unsigned int action = stateRead(block, STATE_ACTION);
    switch(action) {
        case ACTION_RTC_VIRTUAL:
            if (++stateRead(block,STATE_PARAM3) >= 60) {
                stateWrite(block,STATE_PARAM3,0);
                if (++stateRead(block,STATE_PARAM2) >= 60) {
                    stateWrite(block,STATE_PARAM2,0);
                    if (++stateRead(block,STATE_PARAM1) >= 24) {
                        stateWrite(block,STATE_PARAM1,0);
                    }
                }
            } 
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_1s);
            break;
        case ACTION_FLASH_LED: 
            if (value >= 1) { // device is currently ON
                // turn device off
                value = LOW;
                // set off TTR
                stateWrite(block, STATE_TTR, stateRead(block, STATE_PARAM2));
            } else { // device is currently OFF
                // turn device ON
                value = HIGH;
                // return on TTR
                stateWrite(block, STATE_TTR, stateRead(block, STATE_PARAM1));
            }
            updateDigital(block, value);
            break;
        case ACTION_FLICKER:
            // Get a new random value
            t1 = stateRead(block,STATE_PARAM1) & 0b11110000; // high value
            t2 = (stateRead(block,STATE_PARAM1) & 0b00001111) << 4; // low value
            // t3 = (t1 - t2) / 4; // range
                        // safety checks
            if (t1 == t2 || abs(t2 - t1) < 50) { // nonsense values
                t2 = 0; t1 = 255;
            } else if (t2 > t1) { // wrong way around
                b = (byte)t1; t1 = t2; t2 = b;
            }
            do {
                b = (byte)random(t2,t1);
                // don't allow jumps bigger than half
            } while (abs(stateRead(block, STATE_VALUE) - b) > (abs(t2 - t1)/ 2));
            updatePWM(block, b);
            // get a new random TTR
            t1 = (stateRead(block, STATE_PARAM2) & 0b11000000); // units
            t2 = (stateRead(block, STATE_PARAM2) & 0b00111111); // value
            if (t2 == 0) t2 = 1;
            if (t2 > 1) t2 = random(1,t2);
            t2 |= t1; // put the units back
            stateWrite(block, STATE_TTR, t2);
            break;
        case ACTION_FADE_ONCE:
            t1 = stateRead(block,STATE_PARAM1) & 0b11110000; // start value
            t2 = (stateRead(block,STATE_PARAM1) & 0b00001111) << 4; // end value
            if (t2 > t1) { // going up
                if ((t2 - value) < stateRead(block, STATE_PARAM2)) { // reached end value
                    value = t2; // so set end value
                    stateWrite(block, STATE_TTR, 0); // and stop
                } else {
                    value += stateRead(block, STATE_PARAM2);
                    stateWrite(block, STATE_TTR, stateRead(block, STATE_RUNTIME));
                }
            } else { // going down
                if ((value - t2) < stateRead(block, STATE_PARAM2)) { // reached end value
                    value = t2; // so set end value
                    stateWrite(block, STATE_TTR, 0); // and stop
                } else {
                    value -= stateRead(block, STATE_PARAM2);
                    stateWrite(block, STATE_TTR, stateRead(block, STATE_RUNTIME));
                }
            }
            updatePWM(block, value);
            break;
        case ACTION_FADE_CYCLE:
            t1 = stateRead(block,STATE_PARAM1) & 0b11110000; // start value
            t2 = (stateRead(block,STATE_PARAM1) & 0b00001111) << 4; // end value
            if (t2 > t1) { // going up
                if ((t2 - value) < stateRead(block, STATE_PARAM2)) { // reached end value
                    value = t2; // so set end value
                    stateWrite(block, STATE_PARAM1, (t1 >> 4) | t2); // swap start & end
                } else {
                   value += stateRead(block, STATE_PARAM2);
                }
            } else { // going down
                if ((value - t2) < stateRead(block, STATE_PARAM2)) { // reached end value
                    value = t2; // so set end value
                    stateWrite(block, STATE_PARAM1, (t1 >> 4) | t2); // swap start & end
                } else {
                    value -= stateRead(block, STATE_PARAM2);
                }
            }
            stateWrite(block, STATE_TTR, stateRead(block, STATE_RUNTIME));
            updatePWM(block, value);
            break;
        case ACTION_DHT11:
            dht11->read(eepromRead(block,EEPROM_PIN1), statePtr(block, STATE_PARAM1),
                        statePtr(block, STATE_PARAM2), NULL);
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_10s);
            break;
        case ACTION_UPDATE_LCD:
            updateLCD();
            stateWrite(block, STATE_TTR, 12 | TTR_UNIT_20ms); // 240ms (about 1/4 second)
            break;
        case ACTION_SEQ:
        case ACTION_SEQ_HEAD:
            // timer has expired, so move on to next in sequence
            b = stateRead(block,STATE_PARAM1);
            if (b > 0 && (stateRead(b, STATE_ACTION) == ACTION_SEQ || stateRead(b, STATE_ACTION) == ACTION_SEQ_HEAD)) { // zero marks ends of chain, so just stop
                // otherwise turn on new device
                updateDigital(b, (byte)HIGH);
                // and set its new timer value
                stateWrite(b, STATE_TTR, eepromRead(b, STATE_RUNTIME));
            }
            // turn off current device
            updateDigital(block, (byte)LOW);
            // timer for this device is zero
            stateWrite(block, STATE_TTR, 0);
            break;
        case ACTION_SEQ_PWM:
        case ACTION_SEQ_PWM_HEAD:
            // timer has expired, so move on to next in sequence
            b = stateRead(block,STATE_PARAM1);
            if (b > 0 && (stateRead(b, STATE_ACTION) == ACTION_SEQ_PWM || stateRead(b, STATE_ACTION) == ACTION_SEQ_PWM_HEAD)) { // zero marks ends of chain, so just stop
                // otherwise turn on new device
                updatePWM(b, stateRead(block, STATE_PARAM2));
                // and set its new timer value
                stateWrite(b, STATE_TTR, eepromRead(b, STATE_RUNTIME));
            }
            // turn off current device
            updatePWM(block, (byte)LOW);
            // timer for this device is zero
            stateWrite(block, STATE_TTR, 0);
            break;        
        default:
            break;
    }
}