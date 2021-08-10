#include <Arduino.h>
#include <EEPROM.h>
#include "eeprom_layout.h"
#include "monitor.h"
#include "SimpleDHT.h"

extern char monitorBuffer[];
extern char com[], arg[], val[];
extern byte deviceActions[];
extern SimpleDHT11 *dht11;

void setupDevices() {
    unsigned int block = 1;   // skip past device 0, that is the board itself
    byte deviceType, deviceAction, devicePin;
    while ((deviceType = EEPROM.read(block * DEVICE_SIZE)) != DEVICE_END) {
        switch (deviceType) {
            case DEVICE_CONT:       // extra data, should already have been used
            case DEVICE_DELETED:    // deleted device, but more to come
                break;
            case DEVICE_OUTPUT:
            case DEVICE_LED:        // generic output or led are treated the same
                devicePin = EEPROM.read((block * DEVICE_SIZE) + DEVICE_PIN);
                pinMode(devicePin,OUTPUT);
                deviceAction = EEPROM.read((block * DEVICE_SIZE) + DEVICE_ACTION);
                deviceActions[block] = (byte)deviceAction;
                if (deviceAction == DEVICE_ON) {
                    digitalWrite(devicePin,HIGH);
                } else {
                    digitalWrite(devicePin,LOW);
                }
                break;
            case DEVICE_INPUT:
                devicePin = EEPROM.read((block * DEVICE_SIZE) + DEVICE_PIN);
                pinMode(devicePin,INPUT);
                break;
            case DEVICE_TMPHMD:
                dht11 = new SimpleDHT11();
                break;
            default:
                break;

        }
        block += 1;
    }
}

void allDevices(int state) {
    unsigned int devAddr = DEVICE_SIZE;   // skip past device 0, that is the board itself
    byte deviceType, devicePin;
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

unsigned int findDevice(char *tag) { // return number of device block
    unsigned int devBlock = 1;   // skip past device 0, that is the board itself
    byte deviceType, addr;
    while ((deviceType = EEPROM.read((devBlock * DEVICE_SIZE))) != DEVICE_END) {
        addr = ((devBlock * DEVICE_SIZE) + DEVICE_TAG);
        if (tag[0] == EEPROM.read(addr) &&
               tag[1] == EEPROM.read(addr + 1) &&
               tag[2] == EEPROM.read(addr + 2) &&
               tag[3] == EEPROM.read(addr + 3)) {
            return devBlock;
        }
        devBlock += 1;
    }
    return 0;
}


void getDevice(unsigned int block) {
    unsigned int pin;
    switch(EEPROM.read((block * DEVICE_SIZE) + DEVICE_TYPE)) {
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

void setDevice(unsigned int block, int value = -1) {
    unsigned int pin;
    if (value == -1) {
        value = atoi(val);
    }
    switch(EEPROM.read((block * DEVICE_SIZE) + DEVICE_TYPE)) {
        case DEVICE_LED:
        case DEVICE_OUTPUT:
            pin = EEPROM.read((block * DEVICE_SIZE) + DEVICE_PIN);
            if (value > 0) {
                digitalWrite(pin, HIGH);
            } else {
                digitalWrite(pin,LOW);
            };
            break;
        default:
            break;
    }
}
