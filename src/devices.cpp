#include <Arduino.h>
#include <EEPROM.h>
#include "eeprom_layout.h"
#include "monitor.h"

extern char monitorBuffer[];
extern char com[], arg[], val[];

void setupDevices() {
    unsigned int devAddr = DEVICE_SIZE;   // skip past device 0, that is the board itself
    unsigned int deviceType, deviceAction, devicePin;
    while ((deviceType = EEPROM.read(devAddr)) != DEVICE_END) {
        switch (deviceType) {
            case DEVICE_CONT:       // extra data, should already have been used
            case DEVICE_DELETED:    // deleted device, but more to come
                break;
            case DEVICE_OUTPUT:
            case DEVICE_LED:        // generic output or led are treated the same
                devicePin = EEPROM.read(devAddr + DEVICE_PIN);
                pinMode(devicePin,OUTPUT);
                deviceAction = EEPROM.read(devAddr + DEVICE_ACTION);
                if (deviceAction == DEVICE_ON) {
                    digitalWrite(devicePin,HIGH);
                } else {
                    digitalWrite(devicePin,LOW);
                }
                break;
            case DEVICE_INPUT:
                devicePin = EEPROM.read(devAddr + DEVICE_PIN);
                pinMode(devicePin,INPUT);
                break;
            default:
                break;

        }
        devAddr += DEVICE_SIZE;
    }
}

void allDevices(int state) {
    unsigned int devAddr = DEVICE_SIZE;   // skip past device 0, that is the board itself
    unsigned int deviceType, devicePin;
    while ((deviceType = EEPROM.read(devAddr)) != DEVICE_END) {
        switch (deviceType) {
            case DEVICE_CONT:       // extra data, should already have been used
            case DEVICE_DELETED:    // deleted device, but more to come
                break;
            case DEVICE_OUTPUT:
            case DEVICE_LED:        // generic output or led are treated the same
                devicePin = EEPROM.read(devAddr + DEVICE_PIN);
                digitalWrite(devicePin,state);
                break;
            default:
                break;

        }
        devAddr += DEVICE_SIZE;
    }
}

unsigned int findDevice(char *tag) { // return EEPROM address of device block
    unsigned int devAddr = DEVICE_SIZE;   // skip past device 0, that is the board itself
    unsigned int deviceType;
    while ((deviceType = EEPROM.read(devAddr)) != DEVICE_END) {
        if (tag[0] == EEPROM.read(devAddr + DEVICE_TAG) &&
               tag[1] == EEPROM.read(devAddr + DEVICE_TAG + 1) &&
               tag[2] == EEPROM.read(devAddr + DEVICE_TAG + 2) &&
               tag[3] == EEPROM.read(devAddr + DEVICE_TAG + 3)) {
            return devAddr;
        }
        devAddr += DEVICE_SIZE;
    }
    return 0;
}


void getDevice(unsigned int addr) {
    unsigned int pin;
    switch(EEPROM.read(addr + DEVICE_TYPE)) {
        case DEVICE_INPUT: // generic digital input
            sprintf(monitorBuffer, "%u", digitalRead(EEPROM.read(pin)));
            break;
        case DEVICE_ANALOG: // generic analog input
            sprintf(monitorBuffer, "%u", analogRead(EEPROM.read(pin)));
            break;
        default:
            break;
    }
}

void setDevice(unsigned int addr) {
    unsigned int pin;
    switch(EEPROM.read(addr + DEVICE_TYPE)) {
        case DEVICE_LED:
        case DEVICE_OUTPUT:
            pin = EEPROM.read(addr + DEVICE_PIN);
            if (atoi(val) > 0) {
                digitalWrite(pin, HIGH);
            } else {
                digitalWrite(pin,LOW);
            };
            break;
        default:
            break;
    }
}
