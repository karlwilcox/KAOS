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


byte evaluate(byte value, byte optr, byte opnd) {
    int ret;
    switch(optr) {
        case COPY_VALUE:
            ret = value;
            break;
        case LESS_THAN:
            ret = (value < opnd);
            break;
        case GREATER_THAN:
            ret = (value > opnd);
            break;
        case EQUAL_TO:
            ret = (value == opnd);
            break;
        case NOT_EQUAL:
            ret = (value != opnd);
            break;
        case GREATER_EQUAL:
            ret = (value >= opnd);
            break;
        case LESS_EQUAL:
            ret = (value <= opnd);
            break;
        case MOD_EQUAL:
            ret = ((value % opnd) == 0);
            break;
        case NOT_MOD_EQUAL:
            ret = ((value % opnd) != 0);
            break;
        case MULTIPLY:
            ret = value * opnd;
            break;
        case DIVIDE: // don't runtime error
            ret =  (opnd == 0) ? 0 : value / opnd;
            break;
        case PLUS: // don't overflow
            ret = (255 - value >= opnd) ? value + opnd : 255;
            break;
        case MINUS: // don't underflow
            ret = (value >= opnd) ? value - opnd : 0;
            break;
        case VALUES_ABOVE:
            ret = value > opnd ? value : 0;
            break;
        case VALUES_BELOW:
            ret = value < opnd ? value : 0;
            break;
        case INVERSE:
            ret = 255 - value;
            break;
        default:
            ret = 1;
            break;
    }
    return (byte)ret;
}

byte combine(byte operation, byte value1, byte value2) {
    int ret;
    
    switch (operation) {
        case LOGICAL_AND:
            ret = value1 && value2;
            break;
        case LOGICAL_OR:
            ret = value1 || value2;
            break;
        case LOGICAL_XOR:
            ret = value1 ^ value2;
            break;
        case LOGICAL_IF:
            ret = value1 ? value2 : 0;
            break;
        case LOGICAL_IF_NOT:
            ret = !value1 ? value2 : 0;
            break;
        case ARITHMETIC_MAX:
            ret = max(value1, value2);
            break;
        case ARITHMETIC_MIN:
            ret = min(value1, value2);
            break;
        case ARITHMETIC_MINUS:
            ret = value1 - value2;
            break;
        case ARITHMETIC_PLUS:
            ret = value1 + value2;
            break;
    }
    
    return (byte)ret;
}

/*
unsigned int ttr2ms(byte ttr) {
    unsigned int mult;

    switch (ttr & 0b11000000) {
        case TTR_UNIT_20ms:
            mult = 20;
            break;
        case TTR_UNIT_100ms:
            mult = 100;
            break;
        case TTR_UNIT_1s:
            mult = 1000;
            break;
        case TTR_UNIT_10s:
            mult = 10000;
            break;
    }
    return mult * (ttr & 0b00111111);
}

byte ms2ttr(unsigned int ms) {
    unsigned int ttr;

    if (ms < 1260) {
        ttr = ms/TICKRATE;
        if (ttr == 0) ttr = 1;
        ttr &= TTR_UNIT_20ms;
    } else if (ms < 63000) {
        ttr = (ms/100) | TTR_UNIT_100ms;
    } else if (ms < 630000) {
        ttr = (ms/1000) | TTR_UNIT_1s;
    } else if (ttr > 630000) {
        ttr = 63 | TTR_UNIT_10s;
    } else {
        ttr = (ms/10000) | TTR_UNIT_10s;
    }
    return (byte)ttr;
}
*/

byte randomTTR(byte ttr) {
    unsigned int t1, t2;

    t1 = ttr & 0b11000000; // units
    t2 = ttr & 0b00111111; // value
    if (t2 == 0) t2 = 1;
    if (t2 > 1) t2 = random(1,t2);
    t2 |= t1; // put the units back 
    return (byte)t2;
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
    switch(eepromRead(block,EEPROM_BLOCK_TYPE)) {
        case DEVICE_DIGITAL_OUTPUT:
        case DEVICE_PWM_OUTPUT:
            // get signal generator block
            t1 = stateRead(block, STATE_SIGGEN);
            // if valid, set value
            if (t1 > 0 && t1 < MAX_BLOCKS &&  eepromRead(t1, EEPROM_BLOCK_TYPE) < DEVICE_UPPER) {
                b = evaluate(stateRead(t1, STATE_VALUE), stateRead(block, STATE_PARAM1), stateRead(block, STATE_PARAM2));
                if (eepromRead(block,EEPROM_BLOCK_TYPE) == DEVICE_PWM_OUTPUT) {
                    updatePWM(block, b);
                } else {
                    updateDigital(block, b);
                }
            }
            stateWrite(block,STATE_TTR,stateRead(block,STATE_RUNTIME));
            break;
        case DEVICE_RTC_VIRTUAL:
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
        case ACTION_COMBINE:
            t1 = stateRead(eepromRead(block, EEPROM_PARAM1), STATE_VALUE);
            t2 = stateRead(eepromRead(block, EEPROM_PARAM2), STATE_VALUE);
            stateWrite(block, STATE_VALUE, combine(eepromRead(block, EEPROM_PARAM0), t1, t2));
            stateWrite(block,STATE_TTR,eepromRead(block, EEPROM_RUNTIME));
            break;
 /*       case CONTROLLER:
            // get signal generator block
            t1 = eepromRead(block, EEPROM_SIGGEN); // our signal generator
            t2 = eepromRead(block, EEPROM_PARAM0); // target device
            // if valid, set value
            if (t1 > 0 && t1 < MAX_BLOCKS &&  eepromRead(t1, EEPROM_BLOCK_TYPE) < DEVICE_UPPER) {
                b = evaluate(stateRead(t1, STATE_VALUE), eepromRead(block, EEPROM_PARAM1), eepromRead(block, EEPROM_PARAM2));
                if (b) { // enable device / siggen
                    if (stateRead(t2, STATE_SIGGEN) == 0) { // suspended
                        stateWrite(t2, STATE_SIGGEN, stateRead(block, STATE_PARAM1)); // switch back on
                        stateWrite(t2,STATE_TTR,stateRead(t2,STATE_RUNTIME));
                    }
                } else {
                    if (stateRead(t2, STATE_SIGGEN) != 0) { // not already suspended
                        stateWrite(t2, STATE_SIGGEN, 0);
                        stateWrite(t2,STATE_TTR,0);
                    }
                }
            }
            stateWrite(block,STATE_TTR,stateRead(block,STATE_RUNTIME));
            break;         */
        case ACTION_ON_OFF_TIMER: 
            if (value >= 1) { // device is currently ON
                // turn device off
                value = LOW;
                // set off TTR
                stateWrite(block, STATE_TTR, eepromRead(block, EEPROM_PARAM2));
            } else { // device is currently OFF
                // turn device ON
                value = HIGH;
                // return on TTR
                stateWrite(block, STATE_TTR, eepromRead(block, EEPROM_PARAM1));
            }
            stateWrite(block, STATE_VALUE, value);
            break;        
        case ACTION_COUNT:
            switch(eepromRead(block, EEPROM_PARAM0)) {
                case COUNT_UP:
                    if (value < eepromRead(block, EEPROM_PARAM2)) {
                        stateWrite(block, STATE_VALUE, value + eepromRead(block, EEPROM_PARAM4));
                    } else {
                        stateFromEEPROM(block, STATE_VALUE, EEPROM_PARAM1);
                    }
                    break;
                case COUNT_DN:
                    if (value > eepromRead(block, EEPROM_PARAM1)) {
                        stateWrite(block, STATE_VALUE, value - eepromRead(block, EEPROM_PARAM4));
                    } else {
                        stateFromEEPROM(block, STATE_VALUE, EEPROM_PARAM2);
                    }
                    break;
                case COUNT_BOTH:
                    if (stateRead(block,STATE_PARAM1) > 0) { // going up
                        if (value < eepromRead(block, EEPROM_PARAM2)) {
                            stateWrite(block, STATE_VALUE, value + eepromRead(block, EEPROM_PARAM4));
                        } else {
                            stateFromEEPROM(block, STATE_VALUE, EEPROM_PARAM1);
                            stateWrite(block, STATE_PARAM1,0); // change direction
                        }
                    } else { // going down
                        if (value > eepromRead(block, EEPROM_PARAM1)) {
                            stateWrite(block, STATE_VALUE, value - eepromRead(block, EEPROM_PARAM4));
                        } else {
                            stateFromEEPROM(block, STATE_VALUE, EEPROM_PARAM2);
                            stateWrite(block, STATE_PARAM1,1); // change direction
                        }
                    }
                    break;
            }
            stateFromEEPROM(block, STATE_TTR, EEPROM_RUNTIME);
            break;
        case ACTION_WAVEFORM:
            t1 = eepromRead(block, EEPROM_PARAM1); // step value
            switch(eepromRead(block, EEPROM_PARAM0)) {
                case WAVE_SAWTOOTH_UP:
                    if ((unsigned int)(255 - value) > t1) {
                        stateWrite(block, STATE_VALUE, value + t1);
                    } else {
                        stateWrite(block, STATE_VALUE, 0);
                    }
                    break;
                case WAVE_SAWTOOTH_DN:
                    if (value > t1) {
                        stateWrite(block, STATE_VALUE, value - t1);
                    } else {
                        stateWrite(block, STATE_VALUE, 255);
                    }
                case WAVE_TRIANGULAR:
                    if (stateRead(block,STATE_PARAM2) == DIRECTION_UP) { // going up
                        if (value < eepromRead(block, EEPROM_PARAM2)) {
                            stateWrite(block, STATE_VALUE, value + t1);
                        } else {
                            stateWrite(block, STATE_VALUE, 0);
                            stateWrite(block, STATE_PARAM2,DIRECTION_DN); // change direction
                        }
                    } else { // going down
                        if (value > t1) {
                            stateWrite(block, STATE_VALUE, value - t1);
                        } else {
                            stateWrite(block, STATE_VALUE, 255);
                            stateWrite(block, STATE_PARAM2, DIRECTION_UP); // change direction
                        }
                    }
                case WAVE_SINE:
                    t2 = stateRead(block, STATE_PARAM3); // tick count
                    if (255 - t2 > t1) {
                        t2 += t1;
                    } else {
                        t2 = 0;
                    }
                    value = (byte)(127 + (127 * sin(2 * PI * (t2/255))));
                    stateWrite(block, STATE_VALUE, value);
                    stateWrite(block, STATE_PARAM3, t2);
                    break;
                default:
                    break;
            }
            stateFromEEPROM(block, STATE_TTR, EEPROM_RUNTIME);
            break;
        case ACTION_RND_ANALOG:
            stateWrite(block, STATE_VALUE, random(0,256));
            stateWrite(block, STATE_TTR, randomTTR(eepromRead(block, EEPROM_RUNTIME)));
            break;
        case ACTION_RND_DIGITAL:
            value = value == 0 ? 1 : 0;
            stateWrite(block, STATE_VALUE, value);
            stateWrite(block, STATE_TTR, randomTTR(eepromRead(block, EEPROM_RUNTIME)));
            break;
        case ACTION_FLICKER:
            // Get a new random value
            t1 = eepromRead(block,EEPROM_PARAM0); // high value
            t2 = eepromRead(block,EEPROM_PARAM1); // low value
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
            stateWrite(block, STATE_TTR, randomTTR(eepromRead(block, EEPROM_PARAM2)));
            break;
        case DEVICE_DHT11:
            dht11->read(eepromRead(block,EEPROM_PIN1), statePtr(block, STATE_PARAM1),
                        statePtr(block, STATE_PARAM2), NULL);
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_10s);
            break;
        case ACTION_UPDATE_LCD:
            updateLCD();
            stateWrite(block, STATE_TTR, 12 | TTR_UNIT_20ms); // 240ms (about 1/4 second)
            break;
        case ACTION_DIGITAL_INPUT:
            t1 = digitalRead(eepromRead(block, EEPROM_PIN1));
            t2 = eepromRead(block, EEPROM_PARAM1);
            if (t2 == INPUT_RAW) {
                stateWrite(block, STATE_VALUE, t1);
            } else if 
            stateWrite(block, STATE_TTR, randomTTR(eepromRead(block, EEPROM_RUNTIME)));
        default:
            break;
    }
}