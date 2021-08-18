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

void doSample(unsigned int block) {
    switch (eepromRead(block,OFFSET_TYPE)) {
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

void writeDSR(unsigned int block) {
    // how many shift registers are cascaded here?
    unsigned int numSRs = eepromRead(block, OFFSET_SR_NUM);
    // enable loading data
    digitalWrite(eepromRead(block,OFFSET_SR_LATCH_PIN), LOW);
    // for each register, write out the state value
    do {
        shiftOut(eepromRead(block, OFFSET_SR_DATA_PIN), eepromRead(block, OFFSET_SR_CLOCK_PIN),
                 MSBFIRST, stateRead(block,STATE_SR1_MAP + numSRs - 1));
    } while (--numSRs > 0);
    // disable loading data
    digitalWrite(eepromRead(block,OFFSET_SR_LATCH_PIN), HIGH);
}

void updateValue(unsigned int block, byte value) {
    unsigned int pin, SR_block;
    switch (eepromRead(block,OFFSET_TYPE)) {
        case DEVICE_PWM_LED:
            pin = eepromRead(block,OFFSET_PIN1);
            if (value == 1) value = 255;
            analogWrite(pin, value);
            stateWrite(block, STATE_VALUE, value);
            break;
        case DEVICE_LED:
        case DEVICE_OUTPUT:
            pin = eepromRead(block,OFFSET_PIN1);
            if (pin < 100) {
                digitalWrite(pin, value);
            } else { // this is a pin on a shift register
                SR_block = eepromRead(block, OFFSET_SR_BLOCK);
                pin -=101; // convert pins 1 to 8 to bits 0-7
                if (value >= 1) {
                    if (pin < 8) { // 8 pins on first SR
                        stateBitSet(SR_block, STATE_SR1_MAP,1 << pin);
                    } else if (pin < 16) {
                        stateBitSet(SR_block, STATE_SR2_MAP,1 << (pin - 8));
                    } else if (pin < 24) {
                        stateBitSet(SR_block, STATE_SR3_MAP,1 << (pin - 16));
                    } else {
                        stateBitSet(SR_block, STATE_SR4_MAP,1 << (pin - 24));
                    }
                } else {
                    if (pin < 8) { // 8 pins on first SR
                        stateBitClear(SR_block, STATE_SR1_MAP,1 << pin);
                    } else if (pin < 16) {
                        stateBitClear(SR_block, STATE_SR2_MAP,1 << (pin - 8));
                    } else if (pin < 24) {
                        stateBitClear(SR_block, STATE_SR3_MAP,1 << (pin - 16));
                    } else {
                        stateBitClear(SR_block, STATE_SR4_MAP,1 << (pin - 24));
                    }
                }
                writeDSR(SR_block);
            }
            stateWrite(block, STATE_VALUE, value);
            break;
        default:
            break;
    }
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
    unsigned int t1, t2; // temporary store
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
            if (++stateRead(block,STATE_RTC_SECS) >= 60) {
                stateWrite(block,STATE_RTC_SECS,0);
                if (++stateRead(block,STATE_RTC_MINS) >= 60) {
                    stateWrite(block,STATE_RTC_MINS,0);
                    if (++stateRead(block,STATE_RTC_HOURS) >= 24) {
                        stateWrite(block,STATE_RTC_HOURS,0);
                    }
                }
            } 
            retval = stateRead(block, STATE_TTR);
            break;
        case ACTION_CTRL_SET_ACTION:
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
            break;
        case ACTION_FLASH: 
            b = eepromRead(block, OFFSET_FLASH_TTR);
            // upper 4 bits are on time (x 300ms)
            // lower 4 bits are off time (x 300ms)
            if (value >= 1) { // device is currently ON
                // turn device off
                value = LOW;
                // return off TTR
                retval = ((b & 0b00001111) *3) | (TTR_UNIT_100ms << 6);
            } else { // device is currently OFF
                // turn device ON
                value = HIGH;
                // return on TTR
                retval = ((b >> 4) *3) | (TTR_UNIT_100ms << 6);
            }
            updateValue(block, value);
            break;
        case ACTION_FLICKER:
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
            break;
        case ACTION_PULSE:
            t1 = stateRead(block, STATE_VALUE);
            b = eepromRead(block, OFFSET_DELTA);
            if (b == 0) b = 1; // slow pulse
            if (stateRead(block, STATE_PULSE_UP)) { // going up: Lady's wear, garden furniture...
                t1 += b;
                if (t1 > 255) {
                    t1 = 255;
                    stateWrite(block, STATE_PULSE_UP, 0);
                }
            } else { // going down: menswear, pet food...
                if (t1 < b) {
                    t1 = 0;
                    stateWrite(block, STATE_PULSE_UP, 1);
                } else {
                    t1 -= b;
                }
            }
            updateValue(block,(byte)t1);
            retval = stateRead(block, STATE_TTR);
            break;        
        case ACTION_FADE_IN:
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
            break;
        case ACTION_SEQ:
        case ACTION_SEQ_HEAD:
            // timer has expired, so move on to next in sequence
            b = eepromRead(block,OFFSET_NEXT_DEVICE);
            if (b > 0) { // zero marks ends of chain, so just stop
                // otherwise turn on new device
                updateValue(b, HIGH);
                // and set its new timer value
                stateWrite(b, STATE_TTR, eepromRead(b, OFFSET_TTR));
            }
            // turn off current device
            updateValue(block, LOW);
            // timer for this device is zero
            // which is the default return value
            break;
        case ACTION_SAMPLE:
            doSample(block);
            retval = eepromRead(block, OFFSET_TTR);
            break;
        default:
            break;
    }
    return retval;
}