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

extern int temperature, humidity, hours, minutes;
extern SimpleDHT11 *dht11;
extern char lcdLine0[], lcdLine1[];
extern LiquidCrystal *lcd;

void sampleInputs(unsigned int rate) {
    unsigned int devBlock;   // skip past device 0, that is the board itself
    unsigned int pin;

    for (devBlock = 1; devBlock < MAX_DEVICES; devBlock++) {
        if (!(deviceActions[devBlock] == (byte)rate)) {
            continue;
        }
        switch (EEPROM.read((devBlock * EEPROM_BLOCK_SIZE) + OFFSET_TYPE)) {
            case DEVICE_DHT11:    // temperature & humidity
                // Serial.println("Read DHT");
                pin = EEPROM.read((devBlock * EEPROM_BLOCK_SIZE) + OFFSET_PIN1);
                if (dht11->read(pin, (byte*)&temperature, (byte *)&humidity, NULL)) {
                    ; //lags |= FLAG_DHT_FAIL;
                } else {
                    ; //flags &= ~FLAG_DHT_FAIL;
                }
                break;
            default:
                break;
        }
    }
}

void writeDSR(unsigned int block) {
    // how many shift registers are cascaded here?
    unsigned int numSRs = eepromRead(block, OFFSET_SR_NUM);
    // enable loading data
    digitalWrite(EEPROM.read(eepromRead(block,OFFSET_SR_LATCH_PIN)), LOW);
    // for each register, write out the state value
    do {
        shiftOut(eepromRead(block, OFFSET_SR_DATA_PIN), eepromRead(block, OFFSET_SR_CLOCK_PIN),
                 MSBFIRST, stateRead(block,STATE_SR1_MAP + numSRs - 1));
    } while (--numSRs > 0);
    // disable loading data
    digitalWrite(EEPROM.read(eepromRead(block,OFFSET_SR_LATCH_PIN)), HIGH);
}

void updateValue(unsigned int block, byte value) {
    unsigned int pin, SR_block;
    switch (eepromRead(block,OFFSET_TYPE)) {
        case DEVICE_LED:
        case DEVICE_OUTPUT:
            pin = eepromRead(block,OFFSET_PIN1);
            if (pin < 100) {
                digitalWrite(pin, value);
            } else { // this is a pin on a shift register
                SR_block = eepromRead(block, OFFSET_SR_BLOCK);
                pin -=101;
                if (pin < 9) { // 8 pins on first SR
                    stateBitSet(SR_block, STATE_SR1_MAP,1 << (pin - 1));
                } else if (pin < 17) {
                    stateBitSet(SR_block, STATE_SR2_MAP,1 << (pin - 9));
                } else if (pin < 25) {
                    stateBitSet(SR_block, STATE_SR3_MAP,1 << (pin - 17));
                } else {
                    stateBitSet(SR_block, STATE_SR4_MAP,1 << (pin - 25));
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
    static int count = 0;
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
    lcd->print(lcdLine1);
}

// A timer has expired, so do the required action and return a new time value
byte doAction(unsigned int block) {
    // unsigned int t; // temporary store
    byte b; // temporary store
    // default new value is just 0, no further action
    byte retval = 0;
    // Almost always need the current value, so get it
    byte value = stateRead(block, STATE_VALUE);

    // some devices are always handled the same way
    if (eepromRead(block,OFFSET_TYPE) == DEVICE_RTC) {
        if (++stateRead(block,STATE_RTC_SECS) >= 60) {
            stateWrite(block,STATE_RTC_SECS,0);
            if (++stateRead(block,STATE_RTC_MINS) >= 60) {
                stateWrite(block,STATE_RTC_MINS,0);
                if (++stateRead(block,STATE_RTC_HOURS) >= 24) {
                    stateWrite(block,STATE_RTC_HOURS,0);
                }
            }
        } 
        return (1 | (TTR_UNIT_1s << 6));
    }

    // What action do we need to do?
    switch(stateRead(block, STATE_ACTION)) {
        case ACTION_FLSH1: 
            value = value >= 1? 0 : 1;
            updateValue(block, value);
            retval = 1 | (TTR_UNIT_1s << 6);
            break;
        case ACTION_FLSH2: 
            value = value >= 1? 0 : 1;
            updateValue(block, value);
            retval = 5 | (TTR_UNIT_100ms << 6);
            break;
        case ACTION_FLSH3:
            if (value >= 1) {
                value = 0;
                retval = 1 | (TTR_UNIT_1s < 6);
            } else {
                value = 1;
                retval = 5 | (TTR_UNIT_100ms << 6);
            }
            updateValue(block, value);
            break;
        case ACTION_FLCK1:
            break;
        case ACTION_RND2S:
            if (value >= 1) { // on, so long time off
                value = 0;
                retval = random(2,12) | (TTR_UNIT_1s << 6);
            } else { // off, so on for short time 
                value = 1;
                retval = random(5,30) | (TTR_UNIT_100ms << 6);
            }
            updateValue(block, value);
            break;
        case ACTION_SEQ_FAST:
        case ACTION_SEQ_FAST_HEAD:
            // timer has expired, so move on to next in sequence
            b = eepromRead(block,OFFSET_NEXT_DEVICE);
            if (b > 0) { // zero marks ends of chain, so just stop
                // otherwise turn on new device
                updateValue(b, value);
                // and set its new timer value
                stateWrite(b, STATE_TTR, 5 | (TTR_UNIT_100ms << 6));
            }
            // turn off current device
            updateValue(block, value);
            // timer for this device is zero
        case ACTION_SEQ_MED:
        case ACTION_SEQ_MED_HEAD:
            b = eepromRead(block,OFFSET_NEXT_DEVICE);
            if (b > 0) { 
                updateValue(b, value);
                stateWrite(b, STATE_TTR, 1 | (TTR_UNIT_1s << 6));
            }
            updateValue(block, value);
            break;
        case ACTION_SEQ_SLOW:
        case ACTION_SEQ_SLOW_HEAD:
            b = eepromRead(block,OFFSET_NEXT_DEVICE);
            if (b > 0) { 
                updateValue(b, value);
                stateWrite(b, STATE_TTR, 2 | (TTR_UNIT_1s << 6));
            }
            updateValue(block, value);
            break;
        default:
            break;
    }
    return retval;
}