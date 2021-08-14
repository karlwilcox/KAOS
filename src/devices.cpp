#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "eeprom_layout.h"
#include "monitor.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"
#include "devices.h"
#include "actions.h"

extern char monitorBuffer[];
extern char com[], arg[], val[];
extern byte deviceActions[];
extern SimpleDHT11 *dht11;
extern LiquidCrystal *lcd;
extern shiftRegister sr1;
extern char lcdLine0[], lcdLine1[];
const char line0Default[] PROGMEM = "**.*C hh:mm ***%";
const char line1Default[] PROGMEM = "Hello World...  ";

void setupDevices() {
    unsigned int block = 1;   // skip past device 0, that is the board itself
    byte deviceType, deviceAction, devicePin;
    while ((deviceType = EEPROM.read(block * BLOCK_SIZE)) != DEVICE_END) {
        deviceAction = ACTION_NONE;
        switch (deviceType) {
            case DEVICE_CONT:       // extra data, should already have been used
            case DEVICE_DELETED:    // deleted device, but more to come
                break;
            case DEVICE_OUTPUT:
            case DEVICE_LED:        // generic output or led are treated the same
                devicePin = EEPROM.read((block * BLOCK_SIZE) + OFFSET_PIN1);
                pinMode(devicePin,OUTPUT);
                deviceAction = EEPROM.read((block * BLOCK_SIZE) + OFFSET_ACTION);
                if (deviceAction == DEVICE_ON) {
                    digitalWrite(devicePin,HIGH);
                } else {
                    digitalWrite(devicePin,LOW);
                }
                break;
            case DEVICE_INPUT:
                devicePin = EEPROM.read((block * BLOCK_SIZE) + OFFSET_PIN1);
                deviceAction = EEPROM.read((block * BLOCK_SIZE) + OFFSET_ACTION);
                pinMode(devicePin,INPUT);
                break;
            case DEVICE_TMPHMD:
                dht11 = new SimpleDHT11();
                deviceAction = EEPROM.read((block * BLOCK_SIZE) + OFFSET_ACTION);
                break;
            case DEVICE_LCD:
                lcd = new LiquidCrystal(EEPROM_ADDRESS(block,OFFSET_LCDRS),
                                    EEPROM_ADDRESS(block,OFFSET_LCDEN),
                                    EEPROM_ADDRESS(block,OFFSET_LCDD4),
                                    EEPROM_ADDRESS(block,OFFSET_LCDD5),
                                    EEPROM_ADDRESS(block,OFFSET_LCDD6),
                                    EEPROM_ADDRESS(block,OFFSET_LCDD7)
                                    );
                lcd->begin(16,2);
                lcd->clear();
                lcd->setCursor(0,0);
                strcpy_P(lcdLine0, line0Default);
                strcpy_P(lcdLine1, line1Default);
                deviceAction = EEPROM.read((block * BLOCK_SIZE) + OFFSET_ACTION);
                break;
            case DEVICE_DIGITAL_SR:
                pinMode(EEPROM.read(EEPROM_ADDRESS(block, OFFSET_SR_DATA_PIN)), OUTPUT);
                pinMode(EEPROM.read(EEPROM_ADDRESS(block, OFFSET_SR_CLOCK_PIN)), OUTPUT);
                pinMode(EEPROM.read(EEPROM_ADDRESS(block, OFFSET_SR_LATCH_PIN)), OUTPUT);
                sr1.block = block;
                sr1.state.bits = 0;
                sr1.numRegisters = EEPROM.read(EEPROM_ADDRESS(block, OFFSET_SUBTYPE));
                deviceAction = EEPROM.read((block * BLOCK_SIZE) + OFFSET_ACTION);
                break;
            default:
                break;

        }
        deviceActions[block] = (byte)deviceAction;
        block += 1;
    }
}

void allDevices(int state) {
    unsigned int devAddr = BLOCK_SIZE;   // skip past device 0, that is the board itself
    byte deviceType, devicePin;
    while ((deviceType = EEPROM.read(devAddr)) != DEVICE_END) {
        switch (deviceType) {
            case DEVICE_CONT:       // extra data, should already have been used
            case DEVICE_DELETED:    // deleted device, but more to come
                break;
            case DEVICE_OUTPUT:
            case DEVICE_LED:        // generic output or led are treated the same
                devicePin = EEPROM.read(devAddr + OFFSET_PIN1);
                digitalWrite(devicePin,state);
                break;
            default:
                break;

        }
        devAddr += BLOCK_SIZE;
    }
}

unsigned int findDevice(char *tag) { // return number of device block
    unsigned int devBlock = 1;   // skip past device 0, that is the board itself
    byte deviceType, addr;
    while ((deviceType = EEPROM.read((devBlock * BLOCK_SIZE))) != DEVICE_END) {
        addr = ((devBlock * BLOCK_SIZE) + OFFSET_TAG);
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
    switch(EEPROM.read((block * BLOCK_SIZE) + OFFSET_TYPE)) {
        case DEVICE_INPUT: // generic digital input
            sprintf(monitorBuffer, "%u", digitalRead(EEPROM.read(pin)));
            break;
        case DEVICE_ANALOG: // generic analog input
            sprintf(monitorBuffer, "%u", analogRead(EEPROM.read(pin)));
            break;
        case DEVICE_DIGITAL_SR:
            sprintf(monitorBuffer, "%08lx", sr1.state.bits);
            break;
        default:
            break;
    }
}

void setDevice(unsigned int block, int value) {
    unsigned int pin;
    unsigned int writeVal;
    if (value == -1) {
        writeVal = atoi(val);
    } else {
        writeVal = (unsigned int)value;
    }
    switch(EEPROM.read((block * BLOCK_SIZE) + OFFSET_TYPE)) {
        case DEVICE_LED:
        case DEVICE_OUTPUT:
            pin = EEPROM.read((block * BLOCK_SIZE) + OFFSET_PIN1);
            if (writeVal > 0) {
                digitalWrite(pin, HIGH);
            } else {
                digitalWrite(pin,LOW);
            };
            break;
        case DEVICE_DIGITAL_SR:
            sscanf(val, "%lx", &sr1.state.bits);
            writeDSR(&sr1);
            break;
        default:
            break;
    }
}

