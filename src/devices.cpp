#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "memory_map.h"
#include "monitor.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"
#include "devices.h"
#include "actions.h"

extern char com[], arg[], val[];
extern byte deviceStates[];
extern char f_03d[];
extern SimpleDHT11 *dht11;
extern LiquidCrystal *lcd;
extern char lcdLine0[], lcdLine1[];
const char line0Default[] PROGMEM = "**.*C hh:mm ***%";
const char line1Default[] PROGMEM = "Hello World...  ";


void setupDevice(unsigned int block) {
    switch (eepromRead(block, EEPROM_BLOCK_TYPE)) {
        case ACTION_DIGITAL_INPUT:
        case ACTION_ANALOG_INPUT:
            pinMode(eepromRead(block,EEPROM_PIN1),INPUT);
            stateWrite(block, STATE_VALUE, 0);
            stateWrite(block, STATE_TTR, eepromRead(block, EEPROM_RUNTIME));
            break;
        case DEVICE_DIGITAL_OUTPUT:
            if (eepromRead(block,EEPROM_PIN1) < 100) // real pin
                pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updateDigital(block,0); // value will be set by signal generator
            // everything is taken from RAM so can be remote override
            stateFromEEPROM(block,STATE_SIGGEN,EEPROM_SIGGEN);
            stateFromEEPROM(block,STATE_PARAM1,EEPROM_PARAM1);
            stateFromEEPROM(block,STATE_PARAM2,EEPROM_PARAM2);
            stateFromEEPROM(block,STATE_RUNTIME,EEPROM_RUNTIME);
            stateWrite(block, STATE_TTR, eepromRead(block, EEPROM_RUNTIME));
            break;
        case DEVICE_PWM_OUTPUT:
            if (eepromRead(block,EEPROM_PIN1) < 100) // real pin
                pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updatePWM(block,0); // value will be set by signal generator
            // everything is taken from RAM so can be remote override
            stateFromEEPROM(block,STATE_SIGGEN,EEPROM_SIGGEN);
            stateFromEEPROM(block,STATE_PARAM1,EEPROM_PARAM1);
            stateFromEEPROM(block,STATE_PARAM2,EEPROM_PARAM2);
            stateFromEEPROM(block,STATE_RUNTIME,EEPROM_RUNTIME);
            stateWrite(block, STATE_TTR, eepromRead(block, EEPROM_RUNTIME));
            break;
 /*       case CONTROLLER:
            t = eepromRead(block, EEPROM_PARAM0); // target device
            if (t > 0 && t < MAX_BLOCKS && (eepromRead(t, EEPROM_BLOCK_TYPE) == DEVICE_DIGITAL_OUTPUT ||
                                            eepromRead(t, EEPROM_BLOCK_TYPE) == DEVICE_PWM_OUTPUT)) {
                stateWrite(block, STATE_PARAM1, eepromRead(t, STATE_SIGGEN)); // get default SG
                stateWrite(block, STATE_TTR, eepromRead(block, EEPROM_RUNTIME));
            }
            break;        */
        case ACTION_ON_OFF_TIMER: 
            stateWrite(block, STATE_TTR, eepromRead(block, EEPROM_PARAM1)); // on flash time
            stateWrite(block,STATE_VALUE, HIGH);
            break;
        case ACTION_COUNT:
            stateFromEEPROM(block, STATE_VALUE, EEPROM_PARAM1); // initial value is low
            stateFromEEPROM(block, STATE_TTR, EEPROM_RUNTIME);
            stateWrite(block, STATE_PARAM1, DIRECTION_UP); // direction
            break;
        case ACTION_WAVEFORM:
            stateWrite(block, STATE_PARAM1, 0); // start tick count at zero
            stateWrite(block, STATE_VALUE, 0); // initial value is low
            stateFromEEPROM(block, STATE_TTR, EEPROM_RUNTIME);
            stateWrite(block, STATE_PARAM2, DIRECTION_UP); // direction
            break;
        case ACTION_FLICKER:
            // start in the middle of the range
            updatePWM(block, (((eepromRead(block,EEPROM_PARAM1) & 0b11110000) -
                         ((eepromRead(block,EEPROM_PARAM1) & 0b00001111) << 4)) / 2));            
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_20ms); // start as soon as possible
            break;
        case DEVICE_SHIFT_REG:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            pinMode(eepromRead(block,EEPROM_PIN2), OUTPUT);
            pinMode(eepromRead(block,EEPROM_PIN3), OUTPUT);
            digitalWrite(eepromRead(block,EEPROM_PIN3), HIGH);
            stateWrite(block,STATE_PARAM1,0);
            stateWrite(block,STATE_PARAM2,0);
            stateWrite(block,STATE_PARAM3,0);
            writeDSR(block);
            stateWrite(DEVICE_BOARD, STATE_1ST_SR, block);  // TODO Support multiple SRs
            stateWrite(block, STATE_TTR, 0);
            break;
        case DEVICE_RTC_VIRTUAL: // only virtual for now
            stateWrite(DEVICE_BOARD, STATE_RTC_BLOCK,block);
            stateWrite(block,STATE_PARAM1,18); // Hours
            stateWrite(block,STATE_PARAM2,0); // Minutes
            stateWrite(block,STATE_PARAM3,0); // Seconds
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_1s); // update 10th/sec
            break;
        case ACTION_SWITCH:
            stateFromEEPROM(block, STATE_VALUE, EEPROM_VALUE);
            break;
        case ACTION_COMBINE:
            stateFromEEPROM(block, STATE_VALUE, EEPROM_VALUE);
            stateFromEEPROM(block, STATE_TTR, EEPROM_RUNTIME);
            break;
        case DEVICE_DHT11:
            dht11 = new SimpleDHT11();
            stateWrite(DEVICE_BOARD, STATE_DHT_BLOCK,block);
            stateWrite(block,STATE_PARAM1,20); // Temperature
            stateWrite(block,STATE_PARAM2,0); // Humidity
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_10s); // update 10th/sec
            break;
        case ACTION_RND_DIGITAL:
            // initial value is random too
            stateWrite(block, STATE_VALUE, random(1,11) % 2);
            stateWrite(block, STATE_TTR, randomTTR(eepromRead(block, EEPROM_RUNTIME)));
            break;
        case ACTION_RND_ANALOG:
            // initial value is random too
            stateWrite(block, STATE_VALUE, random(0,256));
            stateWrite(block, STATE_TTR, randomTTR(eepromRead(block, EEPROM_RUNTIME)));
            break;
        case ACTION_UPDATE_LCD:
            lcd = new LiquidCrystal(
                eepromRead(block, EEPROM_PIN1),
                eepromRead(block, EEPROM_PIN2),
                eepromRead(block, EEPROM_PIN3),
                eepromRead(block, EEPROM_PIN4),
                eepromRead(block + 1, EEPROM_PIN1),
                eepromRead(block + 1, EEPROM_PIN2)
            );
            strcpy_P(lcdLine0,line0Default);
            strcpy_P(lcdLine1,line1Default);
            stateWrite(block, STATE_TTR, 12 | TTR_UNIT_20ms); 
            break;

/*
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
 */
            default:
                break;

        }
}

void setupDevices() {
    unsigned int block = 1;   // skip past device 0, that is the board itself
    unsigned int deviceType;
    for (block = 1; block < MAX_BLOCKS; block++) {
        deviceType = eepromRead(block, EEPROM_BLOCK_TYPE);
     /*   for( unsigned int i = 0; i < STATE_BLOCK_SIZE; i++) {
            stateFromEEPROM(block,i,i+4); // copy everything into the state block
        } */
        if (deviceType == BLOCK_END_MARKER) break;
        setupDevice(block);
    }

}

unsigned int findDevice(const char *tag) { // return number of device block
    unsigned int devBlock = 1;   // skip past device 0, that is the board itself
    byte deviceType, addr;
    while ((deviceType = eepromRead(devBlock, EEPROM_BLOCK_TYPE)) != BLOCK_END_MARKER) {
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



bool getDevice(unsigned int block) {
    // return false if not meaningful value
    switch(eepromRead(block, EEPROM_BLOCK_TYPE)) {
        case ACTION_DIGITAL_INPUT: // generic digital input
            sprintf(val, f_03d, digitalRead(eepromRead(block,EEPROM_PIN1)));
            break;
        case ACTION_ANALOG_INPUT: // generic analog input
            sprintf(val, f_03d, analogRead(eepromRead(block,EEPROM_PIN2)));
            break;
        case DEVICE_SHIFT_REG:
        case DEVICE_RTC_VIRTUAL:
            sprintf(val, "%03u %03u %03u", stateRead(block, STATE_PARAM1),
                            stateRead(block, STATE_PARAM2),
                            stateRead(block, STATE_PARAM3));
            break;
        case DEVICE_DHT11:
            sprintf(val, "%03u %03u", stateRead(block, STATE_PARAM1),
                            stateRead(block, STATE_PARAM2));
            break;
        default:
            sprintf(val, f_03d, stateRead(block,STATE_VALUE));
            break;
    }
    return true;
}



void setDevice(unsigned int block, unsigned int value) {
    char *ptr;
    switch(eepromRead(block, EEPROM_BLOCK_TYPE)) {
        case DEVICE_SHIFT_REG:
            stateWrite(block,STATE_PARAM1,(byte)value);
            ptr = val;
            while (*ptr != '\0' && *ptr != '/') ptr++;
            if (*ptr == '/') {
                ptr++;
                stateWrite(block, STATE_PARAM2, char2int(ptr));
            }
            while (*ptr != '\0' && *ptr != '/') ptr++;
            if (*ptr == '/') {
                ptr++;
                stateWrite(block, STATE_PARAM3, char2int(ptr));
            }
            writeDSR(block);
            break;
        case DEVICE_DIGITAL_OUTPUT:
            updateDigital(block, (byte)value);
            break;
        case DEVICE_PWM_OUTPUT:
            updatePWM(block, (byte)value);
            break;
        case ACTION_WAIT: // pause for a given time
            stateBitSet(DEVICE_BOARD, STATE_FLAG, FLAG_SLEEP);
            stateWrite(block, STATE_TTR, value);
            break;
        case ACTION_SWITCH:
            stateWrite(block, STATE_VALUE, value);
            break;
        default:
            break;
    }
}

