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
    unsigned int action = stateRead(block, STATE_ACTION);
    switch (action) {
        case ACTION_DIGITAL_INPUT:
        case ACTION_ANALOG_INPUT:
            pinMode(eepromRead(block,EEPROM_PIN1),INPUT);
            stateWrite(block, STATE_TTR, 0);
            break;
        case ACTION_DIGITAL_OUTPUT:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updateDigital(block,stateRead(block,STATE_VALUE));
            stateWrite(block, STATE_TTR, 0);
            break;
        case ACTION_PWM_OUTPUT:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updatePWM(block,stateRead(block,STATE_VALUE));
            stateWrite(block, STATE_TTR, 0);
            break;
        case ACTION_FLASH_LED: 
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            stateWrite(block, STATE_TTR, stateRead(block, STATE_PARAM1)); // on flash time
            updateDigital(block,HIGH);
            break;
        case ACTION_FLICKER:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            // start in the middle of the range
            stateWrite(block, STATE_PARAM3, 126); // start by going up
            updatePWM(block, (((stateRead(block,STATE_PARAM1) & 0b11110000) - ((stateRead(block,STATE_PARAM1) & 0b00001111) << 4)) / 2));            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_20ms); // start as soon as possible
            break;
        case ACTION_FADE_ONCE:
        case ACTION_FADE_TWICE:
        case ACTION_FADE_CYCLE:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            updatePWM(block,stateRead(block, STATE_PARAM1) & 0b11110000); // set start value
            stateWrite(block, STATE_TTR, STATE_RUNTIME);
            break;
        case ACTION_SEQ_PWM_HEAD:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            stateWrite(block, STATE_TTR, stateRead(block, STATE_RUNTIME)); // on flash time
            updatePWM(block,stateRead(block,STATE_PARAM2));
            break;
        case ACTION_SEQ_PWM:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            stateWrite(block, STATE_TTR, 0);
            updatePWM(block,LOW);
            break;        
        case ACTION_SEQ_HEAD:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            stateWrite(block, STATE_TTR, stateRead(block, STATE_RUNTIME)); // on flash time
            updateDigital(block,HIGH);
            break;
        case ACTION_SEQ:
            pinMode(eepromRead(block,EEPROM_PIN1), OUTPUT);
            stateWrite(block, STATE_TTR, 0);
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
            stateWrite(block, STATE_TTR, 0);
            break;
        case ACTION_RTC_VIRTUAL: // only virtual for now
            stateWrite(DEVICE_BOARD, STATE_RTC_BLOCK,block);
            stateWrite(block,STATE_PARAM1,18); // Hours
            stateWrite(block,STATE_PARAM2,0); // Minutes
            stateWrite(block,STATE_PARAM3,0); // Seconds
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_1s); // update 10th/sec
            break;
        case ACTION_DHT11:
            dht11 = new SimpleDHT11();
            stateWrite(DEVICE_BOARD, STATE_DHT_BLOCK,block);
            stateWrite(block,STATE_PARAM1,20); // Temperature
            stateWrite(block,STATE_PARAM2,0); // Humidity
            stateWrite(block, STATE_TTR, 1 | TTR_UNIT_10s); // update 10th/sec
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
  /*      case ACTION_CTRL_OFF:
        case ACTION_CTRL_ON:
        case ACTION_CTRL_RND:
            stateWrite(block, STATE_TTR, ((eepromRead(block,EEPROM_VALUE) & 0b11110000) >> 4) | TTR_UNIT_10s); */





/*


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
 */
            default:
                break;

        }
}

void setupDevices() {
    unsigned int block = 1;   // skip past device 0, that is the board itself
    unsigned int deviceType;
    for (block = 1; block < MAX_DEVICES; block++) {
        deviceType = eepromRead(block, EEPROM_ACTION);
        for( unsigned int i = 0; i < STATE_BLOCK_SIZE; i++) {
            stateFromEEPROM(block,i,i+4); // copy everything into the state block
        }
        if (deviceType == DEVICE_END) break;
        setupDevice(block);
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



bool getDevice(unsigned int block) {
    switch(stateRead(block,STATE_ACTION)) {
        case ACTION_DIGITAL_INPUT: // generic digital input
            sprintf(val, f_03d, digitalRead(eepromRead(block,EEPROM_PIN1)));
            break;
        case ACTION_ANALOG_INPUT: // generic analog input
            sprintf(val, f_03d, analogRead(eepromRead(block,EEPROM_PIN2)));
            break;
        case ACTION_SHIFT_REG:
        case ACTION_RTC_VIRTUAL:
            sprintf(val, "%03u %03u %03u", stateRead(block, STATE_PARAM1),
                            stateRead(block, STATE_PARAM2),
                            stateRead(block, STATE_PARAM3));
            break;
        case ACTION_DHT11:
            sprintf(val, "%03u %03u", stateRead(block, STATE_PARAM1),
                            stateRead(block, STATE_PARAM2));
            break;
        case ACTION_SEQ_HEAD:
        case ACTION_SEQ:
        case ACTION_SEQ_PWM_HEAD:
        case ACTION_SEQ_PWM:
        case ACTION_DIGITAL_OUTPUT:
        case ACTION_PWM_OUTPUT:
        case ACTION_FLASH_LED:
        case ACTION_FADE_CYCLE:
        case ACTION_FADE_ONCE:
        case ACTION_FADE_TWICE:
            sprintf(val, f_03d, stateRead(block,STATE_VALUE));
            break;
        default:
            return false;
            break;
    }
    return true;
}



void setDevice(unsigned int block, unsigned int value) {
    char *ptr;
    switch(stateRead(block,STATE_ACTION)) {
        case ACTION_SHIFT_REG:
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
        case ACTION_DIGITAL_OUTPUT:
        case ACTION_FLASH_LED:
        case ACTION_SEQ_HEAD:
        case ACTION_SEQ:
            updateDigital(block, (byte)value);
            break;
        case ACTION_SEQ_PWM_HEAD:
        case ACTION_SEQ_PWM:
        case ACTION_FADE_CYCLE:
        case ACTION_FADE_ONCE:
        case ACTION_FADE_TWICE:
        case ACTION_PWM_OUTPUT:
            updatePWM(block, (byte)value);
            break;
        case ACTION_WAIT: // pause for a given time
            stateBitSet(DEVICE_BOARD, STATE_FLAG, FLAG_SLEEP);
            stateWrite(block, STATE_TTR, value);
            break;
        default:
            break;
    }
}

