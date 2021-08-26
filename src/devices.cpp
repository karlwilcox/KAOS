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

unsigned int setupDevice(unsigned int block, unsigned int deviceAction) {
    byte value = HIGH;
    unsigned int ttr = 0;
    switch (deviceAction) {
        case ACTION_DIGITAL_INPUT:
        case ACTION_ANALOG_INPUT:
            pinMode(eepromRead(block,EEPROM_PIN1),INPUT);
            break;
        case ACTION_DIGITAL_OUTPUT:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updateDigital(block,eepromRead(block,EEPROM_VALUE));
            break;
        case ACTION_PWM_OUTPUT:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updatePWM(block,eepromRead(block,EEPROM_VALUE));
            break;
        case ACTION_FLASH_LED: 
            stateFromEEPROM(block,STATE_PARAM1, EEPROM_PARAM1); // High time to run
            stateFromEEPROM(block,STATE_PARAM2, EEPROM_PARAM2); // Low time to run
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            // stateCopy(block, STATE_TTR, STATE_PARAM1);
            ttr = eepromRead(block, STATE_PARAM1);
            updateDigital(block,HIGH);
            break;
        case ACTION_PULSE_UP:
        case ACTION_PULSE_DOWN:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            stateFromEEPROM(block,STATE_RUNTIME,EEPROM_RUNTIME); // update time
            stateFromEEPROM(block,STATE_PARAM1,EEPROM_PARAM1); // high value
            stateFromEEPROM(block,STATE_PARAM2,EEPROM_PARAM2); // low value
            if (deviceAction == ACTION_PULSE_UP) {
                updatePWM(block,stateRead(block, STATE_PARAM2));
            } else {
                updatePWM(block,stateRead(block, STATE_PARAM1));
            }
            // stateCopy(block, STATE_TTR, STATE_RUNTIME);
            ttr = eepromRead(block, STATE_RUNTIME);
            break;
        case ACTION_SEQ_HEAD:
            stateFromEEPROM(block,STATE_RUNTIME,EEPROM_RUNTIME); // update time
            stateFromEEPROM(block,STATE_PARAM1,EEPROM_PARAM1); // Next device in sequence
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            // stateCopy(block, STATE_TTR, STATE_RUNTIME);
            ttr = eepromRead(block, STATE_RUNTIME);
            updateDigital(block,HIGH);
            break;
        case ACTION_SEQ:
            stateFromEEPROM(block,STATE_RUNTIME,EEPROM_RUNTIME); // update time
            stateFromEEPROM(block,STATE_PARAM1,EEPROM_PARAM1); // Next device in sequence
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updateDigital(block,LOW);
            break;
        case ACTION_SHIFT_REG:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            pinMode(eepromRead(block,EEPROM_PIN2), OUTPUT);
            pinMode(eepromRead(block,EEPROM_PIN3), OUTPUT);
            stateWrite(block,STATE_PARAM1,0);
            stateWrite(block,STATE_PARAM2,0);
            stateWrite(block,STATE_PARAM3,0);
            writeDSR(block);
            stateWrite(DEVICE_BOARD, STATE_1ST_SR, block);  // TODO Support multiple SRs
            break;
        case ACTION_RTC_VIRTUAL: // only virtual for now
            stateWrite(DEVICE_BOARD, STATE_RTC_BLOCK,block);
            stateWrite(block,STATE_PARAM1,18); // Hours
            stateWrite(block,STATE_PARAM2,0); // Minutes
            stateWrite(block,STATE_PARAM3,0); // Seconds
            stateWrite(block, STATE_RUNTIME, 1 << TTR_UNIT_1s);
            // stateWrite(block, STATE_TTR, 1 << TTR_UNIT_1s);
            ttr = 1 << TTR_UNIT_1s;
            break;
        case ACTION_DHT11:
            dht11 = new SimpleDHT11();
            stateWrite(DEVICE_BOARD, STATE_DHT_BLOCK,block);
            stateWrite(block,STATE_PARAM1,20); // Temperature
            stateWrite(block,STATE_PARAM2,0); // Humidity
            // stateWrite(block, STATE_TTR, 10 << TTR_UNIT_1s);
            ttr = 10 << TTR_UNIT_10s;
            break;
            




/*

            // Flow Through 
        case DEVICE_OUTPUT:
        case DEVICE_LED:        // generic output or led are treated the same
            pinMode(eepromRead(block,OFFSET_PIN1),OUTPUT);
            deviceAction = EEPROM.read((block * EEPROM_BLOCK_SIZE) + OFFSET_ACTION);
            switch (deviceAction) {
                // simple cases
                case DEVICE_ON: 
                    case DEVICE_OFF: 
                        updateValue(block,deviceAction == DEVICE_ON? onValue : 0);
                        deviceTTR = 0;
                        break;
                    // these all are off to start with, no timer
                    case ACTION_SEQ:
                        updateValue(block,  0);
                        deviceTTR = 0;
                        break;
                    // switch sequence heads on, and set their timers
                    case ACTION_SEQ_HEAD:
                        updateValue(block,  onValue);
                        deviceTTR = eepromRead(block, OFFSET_TTR);
                        break;
                    case ACTION_FADE_OUT:
                        updateValue(block, 255);
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

            case DEVICE_RTC:
                if (eepromRead(block, OFFSET_ACTION) == ACTION_RTC_VIRTUAL) {
                    deviceTTR = 1 | (TTR_UNIT_1s << 6);
                    stateWrite(block, STATE_RTC_HOURS, 18);
                    stateWrite(block, STATE_RTC_MINS, 0);
                    stateWrite(block, STATE_RTC_SECS, 0);
                } // fake a real time clock by counting seconds
                break;
            case DEVICE_CONTROLLER:
                stateWrite(block,STATE_VALUE, LOW);
                stateWrite(block, STATE_TTR, (random(eepromRead(block,OFFSET_CTRL_MIN_TIME),
                                            eepromRead(block,OFFSET_CTRL_MAX_TIME)) | (TTR_UNIT_10s << 6)));
                break; */
            default:
                break;

        }
        return ttr;

}

void setupDevices() {
    unsigned int block = 1;   // skip past device 0, that is the board itself
    unsigned int deviceType;
    while ((deviceType = eepromRead(block, EEPROM_ACTION)) != DEVICE_END) {
        if (deviceType == DEVICE_CONT || deviceType == DEVICE_DELETED) {      // extra data, should already have been used
            continue;                                                        // or deleted device, but more to come
        }
        stateWrite(block, STATE_TTR, setupDevice(block, eepromRead(block, EEPROM_ACTION)));
        stateCopy(block, STATE_ACTION, EEPROM_ACTION);
        block += 1;
    }
}

unsigned int findDevice(const char *tag) { // return number of device block
    unsigned int devBlock = 1;   // skip past device 0, that is the board itself
    byte deviceType, addr;
    while ((deviceType = stateRead(devBlock, STATE_ACTION)) != DEVICE_END) {
        addr = ((devBlock * EEPROM_BLOCK_SIZE) + EEPROM_TAG);
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
    switch(eepromRead(block,STATE_ACTION)) {
        case ACTION_DIGITAL_INPUT: // generic digital input
            sprintf(monitorBuffer, f_03d, digitalRead(eepromRead(block,EEPROM_PIN1)));
            break;
        case ACTION_ANALOG_INPUT: // generic analog input
            sprintf(monitorBuffer, f_03d, analogRead(eepromRead(block,EEPROM_PIN2)));
            break;
        case ACTION_SHIFT_REG:
            sprintf(monitorBuffer, "%03u %03u %03u", stateRead(block, STATE_PARAM1),
                            stateRead(block, STATE_PARAM2),
                            stateRead(block, STATE_PARAM3));
            break;
        case ACTION_DIGITAL_OUTPUT:
        case ACTION_PWM_OUTPUT:
        case ACTION_FLASH_LED:
        case ACTION_PULSE_UP:
        case ACTION_PULSE_DOWN:
            sprintf(monitorBuffer, f_03d, stateRead(block,STATE_VALUE));
            break;
        default:
            break;
    }
}



void setDevice(unsigned int block, unsigned int value) {
    switch(stateRead(block,STATE_ACTION)) {
        case ACTION_SHIFT_REG:
            break;
        case ACTION_DIGITAL_OUTPUT:
        case ACTION_FLASH_LED:
        case ACTION_PULSE_UP:
        case ACTION_PULSE_DOWN:
            updateDigital(block, (byte)value);
            break;
        case ACTION_PWM_OUTPUT:
            updatePWM(block, (byte)value);
        default:
            break;
    }
}

