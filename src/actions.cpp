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
/*    static int count = 0;
    static unsigned int curPos = 0;
    char temp[10];

    if (count == 12) {
        // After 3 seconds display centigrade
        sprintf(temp, "%4d", (int)temperature);
        lcdLine0[0] = temp[0];
        lcdLine0[1] = temp[1];
        lcdLine0[2] = temp[2];
        lcdLine0[3] = temp[3];
        lcdLine0[4] = 'C';    
        // after another 3 seconds display fahrenheit
        // update the clock and humidity
    } else if (count == 24) {
        sprintf(temp, "%4d", (((int)temperature * 9) / 5) + 32);
        lcdLine0[0] = temp[0];
        lcdLine0[1] = temp[1];
        lcdLine0[2] = temp[2];
        lcdLine0[3] = temp[3];
        lcdLine0[4] = 'F';  
        sprintf(temp, "%02u", hours);
        lcdLine0[6] = temp[0];
        lcdLine0[7] = temp[1];
        sprintf(temp, "%02u", minutes);
        lcdLine0[9] = temp[0];
        lcdLine0[10] = temp[1];
        sprintf(temp, "%3d", (int)humidity);
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
    lcd->print(lcdLine1); */
}

// A timer has expired, so do the required action and return a new time value
byte doAction(unsigned int block) {
    unsigned int t1; // temporary store
    byte b; // temporary store
    // default new value is just 0, no further action
    byte retval = 0;
    // Almost always need the current value, so get it
    byte value = stateRead(block, STATE_VALUE);

    // What action do we need to do?
    switch(stateRead(block, STATE_ACTION)) {
        case ACTION_UPDATE_LCD:
            break;
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
            retval = stateRead(block, STATE_RUNTIME);
            break;
    /*    case ACTION_CTRL_SET_ACTION:
            b = eepromRead(block,OFFSET_CTRL_DEVICE);
            if (stateRead(block,STATE_VALUE) == LOW) { // target device is off
                stateWrite(b, STATE_ACTION, eepromRead(b,OFFSET_ACTION));
                stateWrite(b, STATE_TTR, eepromRead(b,OFFSET_TTR));
                stateWrite(block,STATE_VALUE,HIGH);
            } else {
                stateWrite(b,STATE_ACTION,0); // turn off target device
                stateWrite(block,STATE_VALUE,LOW);
            }
            retval = (random(eepromRead(block,OFFSET_CTRL_MIN_TIME),
                                        eepromRead(block,OFFSET_CTRL_MAX_TIME)) | (TTR_UNIT_10s << 6));
            break; */
        case ACTION_FLASH_LED: 
            if (value >= 1) { // device is currently ON
                // turn device off
                value = LOW;
                // return off TTR
                retval = stateRead(block, STATE_PARAM2);
            } else { // device is currently OFF
                // turn device ON
                value = HIGH;
                // return on TTR
                retval = stateRead(block, STATE_PARAM1);
            }
            updateDigital(block, value);
            break;
/*        case ACTION_FLICKER:
            b = eepromRead(block, OFFSET_FLICKER_MAXMIN);
            t1 = 255 * (((b >> 4) * 10 ) / 100); // max value
            t2 = 255 * (((b & 0b00001111) * 10) / 100); // min value;
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
            updateValue(block,b);
            retval = random(10,50) | (stateRead(block, STATE_TTR) & 0b11000000);
            break; */
        case ACTION_PULSE_UP:
            // calculate step
            t1 = (stateRead(block, STATE_PARAM1) - stateRead(block, STATE_PARAM2)) / stateRead(block,STATE_RUNTIME);
            // check for limit (careful not to overflow)
            if ((unsigned int)(stateRead(block, STATE_PARAM1) - stateRead(block, STATE_VALUE)) > t1) { // state change
                b = stateRead(block, STATE_PARAM1); // set high value
                stateWrite(block, STATE_ACTION, ACTION_PULSE_DOWN); // go down;
            } else {
                b = stateRead(block, STATE_VALUE) + t1;
            }
            updateDigital(block,b);
            retval = stateRead(block, STATE_RUNTIME);
            break;               
        case ACTION_PULSE_DOWN:
            // calculate step
            t1 = (stateRead(block, STATE_PARAM1) - stateRead(block, STATE_PARAM2)) / stateRead(block,STATE_RUNTIME);
            // check for limit (careful of underflow)
            if (stateRead(block, STATE_VALUE) < t1) { // state change
                b = stateRead(block, STATE_PARAM2); // set low value
                stateWrite(block, STATE_ACTION, ACTION_PULSE_UP); // go up;
            } else {
                b = stateRead(block, STATE_VALUE) - t1;
            }
            updateDigital(block,b);
            retval = stateRead(block, STATE_RUNTIME);
            break;     
 /*       case ACTION_FADE_IN:
            retval = stateRead(block, STATE_TTR);
            t1 = stateRead(block, STATE_VALUE);
            b = eepromRead(block, OFFSET_DELTA);
            if (b == 0) b = 1; // slow rise
            t1 += b;
            if (t1 > 255) {
                t1 = 255;
                stateWrite(block, STATE_ACTION, DEVICE_ON);
                retval = 0;
            }
            updateValue(block,(byte)t1);
            break;    
         case ACTION_FADE_OUT:
            retval = stateRead(block, STATE_TTR);
            t1 = stateRead(block, STATE_VALUE);
            b = eepromRead(block, OFFSET_DELTA);
            if (b == 0) b = 1; // slow fade
            if (t1 < b) {
                t1 = 0;
                stateWrite(block, STATE_ACTION, DEVICE_OFF);
                retval = 0;
            } else {
                t1 -= b;
            }
            updateValue(block,(byte)t1);
            break;
        case ACTION_RANDOM:
            b = eepromRead(block, OFFSET_RANDOM_TTR);
            t1 = ((b >> 4) * 10) % 100; // duty cycle
            t2 = (b & 0b00001111) * 10; // max length, in seconds
            if (value >= 1) { // on
                value = 0;
                retval = random(1,(t1 / 100 * t2)) | (TTR_UNIT_1s << 6);
            } else { // off
                value = 1;
                retval = random(1,(100 - t1)/100 * t2) | (TTR_UNIT_1s << 6);
            }
            updateValue(block, value);
            break; */
        case ACTION_DHT11:
            dht11->read(eepromRead(block,EEPROM_PIN1), statePtr(block, STATE_PARAM1),
                        statePtr(block, STATE_PARAM2), NULL);
            break;
        case ACTION_SEQ:
        case ACTION_SEQ_HEAD:
            // timer has expired, so move on to next in sequence
            b = eepromRead(block,STATE_PARAM1);
            if (b > 0) { // zero marks ends of chain, so just stop
                // otherwise turn on new device
                updateDigital(b, (byte)HIGH);
                // and set its new timer value
                stateWrite(b, STATE_TTR, eepromRead(b, STATE_RUNTIME));
            }
            // turn off current device
            updateDigital(block, (byte)LOW);
            // timer for this device is zero
            // which is the default return value
            break;
        default:
            break;
    }
    return retval;
}