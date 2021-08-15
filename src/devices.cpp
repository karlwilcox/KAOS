#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "memory_map.h"
#include "monitor.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"
#include "devices.h"
#include "actions.h"

extern char monitorBuffer[];
extern char com[], arg[], val[];
extern byte deviceStates[];
extern char f_03d[];
extern SimpleDHT11 *dht11;
extern LiquidCrystal *lcd;
extern char lcdLine0[], lcdLine1[];
const char line0Default[] PROGMEM = "**.*C hh:mm ***%";
const char line1Default[] PROGMEM = "Hello World...  ";


void setupDevices() {
    unsigned int block = 1;   // skip past device 0, that is the board itself
    byte deviceType, deviceAction, deviceTTR;
    while ((deviceType = EEPROM.read(block * EEPROM_BLOCK_SIZE)) != DEVICE_END) {
        deviceAction = ACTION_NONE;
        deviceTTR = 1; // 10ms, run as soon as possible
        switch (deviceType) {
            case DEVICE_CONT:       // extra data, should already have been used
            case DEVICE_DELETED:    // deleted device, but more to come
                break;
            case DEVICE_OUTPUT:
            case DEVICE_LED:        // generic output or led are treated the same
                pinMode(eepromRead(block,OFFSET_PIN1),OUTPUT);
                deviceAction = EEPROM.read((block * EEPROM_BLOCK_SIZE) + OFFSET_ACTION);
                switch (deviceAction) {
                    // simple cases
                    case DEVICE_ON: 
                    case DEVICE_OFF: 
                        updateValue(block,deviceAction == DEVICE_ON? 1 : 0);
                        deviceTTR = 0;
                        break;
                    // these all are off to start with, no timer
                    case ACTION_SEQ_FAST:
                    case ACTION_SEQ_MED:
                    case ACTION_SEQ_SLOW:
                        updateValue(block,  0);
                        deviceTTR = 0;
                        break;
                    // switch sequence heads on, and set their timers
                    case ACTION_SEQ_FAST_HEAD:
                        updateValue(block,  1);
                        deviceTTR = 5 | (TTR_UNIT_100ms << 6);
                        break;
                    case ACTION_SEQ_MED_HEAD:
                        updateValue(block,  1);
                        deviceTTR = 1 | (TTR_UNIT_1s << 6);
                        break;
                    case ACTION_SEQ_SLOW_HEAD:
                        updateValue(block,  1);
                        deviceTTR = 2 | (TTR_UNIT_1s << 6);
                        break;
                    // other actions we can just leave to run as soon as possible
                    default:
                        break;
                }
                break;
            case DEVICE_INPUT:
                deviceAction = EEPROM.read((block * EEPROM_BLOCK_SIZE) + OFFSET_ACTION);
                pinMode(eepromRead(block,OFFSET_PIN1),INPUT);
                break;
            case DEVICE_DHT11:
                dht11 = new SimpleDHT11();
                deviceAction = EEPROM.read((block * EEPROM_BLOCK_SIZE) + OFFSET_ACTION);
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
                deviceAction = EEPROM.read((block * EEPROM_BLOCK_SIZE) + OFFSET_ACTION);
                break;
            case DEVICE_DIGITAL_SR:
                pinMode(EEPROM.read(EEPROM_ADDRESS(block, OFFSET_SR_DATA_PIN)), OUTPUT);
                pinMode(EEPROM.read(EEPROM_ADDRESS(block, OFFSET_SR_CLOCK_PIN)), OUTPUT);
                pinMode(EEPROM.read(EEPROM_ADDRESS(block, OFFSET_SR_LATCH_PIN)), OUTPUT);
                stateWrite(block,STATE_SR1_MAP,0);
                stateWrite(block,STATE_SR2_MAP,0);
                stateWrite(block,STATE_SR3_MAP,0);
                stateWrite(block,STATE_SR4_MAP,0);
                // everything off to start with
                writeDSR(block);
                deviceTTR = 0; // SRs don't have a state change of their own
                break;
            case DEVICE_RTC:
                if (eepromRead(block, OFFSET_ACTION) == ACTION_RTC_VIRTUAL) {
                    deviceTTR = 1 | (TTR_UNIT_1s << 6);
                } // fake a real time clock by counting seconds
                break;
            default:
                break;

        }
        if (deviceAction != ACTION_NONE) {
            stateWrite(block,STATE_ACTION,(byte)deviceAction);
        }
        if (deviceTTR != 0) {
            stateWrite(block,STATE_TTR,deviceTTR); // trigger action as soon as possible (usually)
        }
        block += 1;
    }
}

unsigned int findDevice(const char *tag) { // return number of device block
    unsigned int devBlock = 1;   // skip past device 0, that is the board itself
    byte deviceType, addr;
    while ((deviceType = eepromRead(devBlock, OFFSET_TYPE)) != DEVICE_END) {
        addr = ((devBlock * EEPROM_BLOCK_SIZE) + OFFSET_TAG);
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
    switch(eepromRead(block,OFFSET_TYPE)) {
        case DEVICE_INPUT: // generic digital input
            sprintf(monitorBuffer, f_03d, digitalRead(eepromRead(block,OFFSET_PIN1)));
            break;
        case DEVICE_ANALOG: // generic analog input
            sprintf(monitorBuffer, f_03d, analogRead(eepromRead(block,OFFSET_PIN1)));
            break;
        case DEVICE_DIGITAL_SR:
            sprintf(monitorBuffer, "%03u %03u %03u %03u", stateRead(block, STATE_SR1_MAP),
                            stateRead(block, STATE_SR2_MAP),
                            stateRead(block, STATE_SR3_MAP),
                            stateRead(block, STATE_SR4_MAP));
            break;
        case DEVICE_LED:
        case DEVICE_OUTPUT:
            sprintf(monitorBuffer, f_03d, stateRead(block,STATE_VALUE));
            break;
        default:
            break;
    }
}

