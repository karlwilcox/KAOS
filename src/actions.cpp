#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "actions.h"
#include "devices.h"
#include "eeprom_layout.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"
#include "devices.h"

extern byte deviceActions[];
// deviceActions is initialised to all OFF
// Then setupDevices populates ONLY those entries that have an action set
// Hence we can just traverse this array to find which devices
// are set to use each action

extern int temperature, humidity, hours, minutes;
extern SimpleDHT11 *dht11;
extern char lcdLine0[], lcdLine1[];
extern LiquidCrystal *lcd;
extern shiftRegister sr1;
extern uint16_t flags;

void blink() { // alternate every 1/2 second
    static char phase = 'a';
    unsigned int block;   // skip past device 0, that is the board itself
    unsigned int pin;

    for (block = 1; block < MAX_DEVICES; block++) {
        if (!(deviceActions[block] == (byte)ACTION_BLNKA || deviceActions[block] == (byte)ACTION_BLNKB)) {
            continue;
        }
        pin = EEPROM.read((block * BLOCK_SIZE) + OFFSET_PIN1);
        if (phase == 'a') {
            if (deviceActions[block] == (byte)ACTION_BLNKA) {
                digitalWrite(pin,HIGH);
            } else if (deviceActions[block] == (byte)ACTION_BLNKB) {
                digitalWrite(pin, LOW);
            }
        } else { // phase = 'b'
            if (deviceActions[block] == (byte)ACTION_BLNKB) {
                digitalWrite(pin,HIGH);
            } else if (deviceActions[block] == (byte)ACTION_BLNKA) {
                digitalWrite(pin, LOW);
            }
        }
    }
    phase = (phase == 'a') ? 'b' : 'a';
}

void flash(unsigned int action) { // flash as per action
    static unsigned int state1 = LOW;
    static unsigned int state2 = LOW;
    static unsigned int state3 = 0;
    unsigned int state;
    unsigned int devBlock;   // skip past device 0, that is the board itself
    unsigned int pin;
    switch (action) {
        case ACTION_FLSH1:
            state = state1 = (state1 == HIGH) ? LOW : HIGH;
            break;
        case ACTION_FLSH2:
            state = state2 = (state2 == HIGH) ? LOW : HIGH;
            break;
        case ACTION_FLSH3:
            if (state3 == 0 ) {
                state = LOW;
                state3 = 1;
            } else if (state3 == 1) {
                state = LOW;
                state3 = 2;
            } else { // state3 is 3
                state = HIGH;
                state3 = 0;
            }
    }
    for (devBlock = 1; devBlock < MAX_DEVICES; devBlock++) {
        if (!(deviceActions[devBlock] == (byte)action)) {
            continue;
        }
        pin = EEPROM.read((devBlock * BLOCK_SIZE) + OFFSET_PIN1);
        digitalWrite(pin,state);
    }
}

void sampleInputs(unsigned int rate) {
    unsigned int devBlock;   // skip past device 0, that is the board itself
    unsigned int pin;

    for (devBlock = 1; devBlock < MAX_DEVICES; devBlock++) {
        if (!(deviceActions[devBlock] == (byte)rate)) {
            continue;
        }
        switch (EEPROM.read((devBlock * BLOCK_SIZE) + OFFSET_TYPE)) {
            case DEVICE_TMPHMD:    // temperature & humidity
                // Serial.println("Read DHT");
                pin = EEPROM.read((devBlock * BLOCK_SIZE) + OFFSET_PIN1);
                if (dht11->read(pin, (byte*)&temperature, (byte *)&humidity, NULL)) {
                    flags |= FLAG_DHT_FAIL;
                } else {
                    flags &= ~FLAG_DHT_FAIL;
                }
                break;
            default:
                break;
        }
    }
}

void writeDSR(shiftRegister *sr) {
    digitalWrite(EEPROM.read(EEPROM_ADDRESS(sr->block, OFFSET_SR_LATCH_PIN)), LOW);
    for (unsigned int i = 0; i < sr->numRegisters; i++) {
        shiftOut(EEPROM.read(EEPROM_ADDRESS(sr->block, OFFSET_SR_DATA_PIN)), EEPROM.read(EEPROM_ADDRESS(sr->block, OFFSET_SR_DATA_PIN)), MSBFIRST, sr->state.bytes[i]);
    }
    digitalWrite(EEPROM.read(EEPROM_ADDRESS(sr->block, OFFSET_SR_LATCH_PIN)), HIGH);
}

void myDigitalWrite(unsigned int pin, unsigned int value) {
    if (pin < 100) {
        digitalWrite(pin, value);
    } else {
        pin -= 101;
        sr1.state.bits &= 1 << pin;
        writeDSR(&sr1);
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
