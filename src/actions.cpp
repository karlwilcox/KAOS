#include <Arduino.h>
#include <EEPROM.h>
#include "actions.h"
#include "devices.h"
#include "eeprom_layout.h"
#include "config.h"
#include "SimpleDHT.h"

extern byte deviceActions[];
// deviceActions is initialised to all OFF
// Then setupDevices populates ONLY those entries that have an action set
// Hence we can just traverse this array to find which devices
// are set to use each action

extern int temperature, humidity;
extern SimpleDHT11 *dht11;

void blink() { // alternate every 1/2 second
    static char phase = 'a';
    unsigned int block;   // skip past device 0, that is the board itself
    unsigned int pin;

    for (block = 1; block < MAX_DEVICES; block++) {
        if (!(deviceActions[block] == (byte)ACTION_BLNKA || deviceActions[block] == (byte)ACTION_BLNKB)) {
            continue;
        }
        pin = EEPROM.read((block * DEVICE_SIZE) + DEVICE_PIN);
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
        pin = EEPROM.read((devBlock * DEVICE_SIZE) + DEVICE_PIN);
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
        switch (EEPROM.read((devBlock * DEVICE_SIZE) + DEVICE_TYPE)) {
            case DEVICE_TMPHMD:    // temperature & humidity
                pin = EEPROM.read((devBlock * DEVICE_SIZE) + DEVICE_PIN);
                dht11->read(pin, (byte*)&temperature, (byte *)&humidity, NULL);
                break;
            default:
                break;
        }
    }
        
}
