#include <Arduino.h>
#include <EEPROM.h>
#include "actions.h"
#include "devices.h"
#include "eeprom_layout.h"


void blink() { // alternate every 1/2 second
    static char phase = 'a';
    unsigned int devAddr = DEVICE_SIZE;   // skip past device 0, that is the board itself
    unsigned int deviceType, pin;
    while ((deviceType = EEPROM.read(devAddr)) != DEVICE_END) {
        pin = EEPROM.read(devAddr + DEVICE_PIN);
        if (phase == 'a') {
            if (EEPROM.read(devAddr + DEVICE_ACTION) == DEVICE_BLNKA) {
                digitalWrite(pin,HIGH);
            } else if (EEPROM.read(devAddr + DEVICE_ACTION) == DEVICE_BLNKB) {
                digitalWrite(pin, LOW);
            }
        } else { // phase = 'b'
            if (EEPROM.read(devAddr + DEVICE_ACTION) == DEVICE_BLNKB) {
                digitalWrite(pin,HIGH);
            } else if (EEPROM.read(devAddr + DEVICE_ACTION) == DEVICE_BLNKA) {
                digitalWrite(pin, LOW);
            }
        }
        devAddr += DEVICE_SIZE;
    }
    if (phase == 'a') {
        phase = 'b';
    } else {
        phase = 'a';
    }
}

void flash1s() { // flash on a 1 second cycle
    static unsigned int state = LOW;
    unsigned int devAddr = DEVICE_SIZE;   // skip past device 0, that is the board itself
    unsigned int deviceType, pin;
    if (state == LOW) {
        state = HIGH;
    } else {
        state = LOW;
    }
    while ((deviceType = EEPROM.read(devAddr)) != DEVICE_END) {
        pin = EEPROM.read(devAddr + DEVICE_PIN);
        if (EEPROM.read(devAddr + DEVICE_ACTION) == DEVICE_FLSH1) {
            digitalWrite(pin,state);
        }
        devAddr += DEVICE_SIZE;
    }
}

void flash0_5s() { // flash on a 1 second cycle
    static unsigned int state = LOW;
    unsigned int devAddr = DEVICE_SIZE;   // skip past device 0, that is the board itself
    unsigned int deviceType, pin;
    if (state == LOW) {
        state = HIGH;
    } else {
        state = LOW;
    }
    while ((deviceType = EEPROM.read(devAddr)) != DEVICE_END) {
        pin = EEPROM.read(devAddr + DEVICE_PIN);
        if (EEPROM.read(devAddr + DEVICE_ACTION) == DEVICE_FLSH2) {
            digitalWrite(pin,state);
        }
        devAddr += DEVICE_SIZE;
    }
}