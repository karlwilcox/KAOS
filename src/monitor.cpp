/*
 * Monitor code
 */
#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "monitor.h"
#include "memory_map.h"
#include "devices.h"
#include "actions.h"

int monitorFlags = 0;
const char badAddr[] PROGMEM = "Bad addr";
const char badCmd[] PROGMEM = "Bad cmd";
const char badTag[] PROGMEM = "Bad tag";
const char badDev[] PROGMEM = "Bad dev";
const char tag_dht11[] = "DHT1";
const char tag_rtc[] = "RTC1";
extern char monitorBuffer[];
extern int hours, minutes, seconds, temperature, humidity;
extern char com[], arg[], val[];
extern unsigned int flags;
extern byte deviceStates[];
extern char f_03d[];

unsigned int str2pin(char t, char d1, char d2) {
    const unsigned int aPinValues[] = {
        A0, A1, A2, A3, A4, A5
#if NANO == 1 || MEGA == 1
        , A6, A7
#endif
#if MEGA == 1
        , A8, A9, A10, A11, A12, A13, A14, A15
#endif
    };
    if (t == 'a') { // analog pin
        unsigned int index =  (10 * (d1 - '0') + (d2 - '0'));
        if (index < (sizeof(aPinValues) / sizeof(aPinValues[0]))) {
            return aPinValues[index];
        }
        // else return 255
    } else if (t == 'd' || t == 'p') {
        return (10 * (d1 - '0') + (d2 - '0'));
    }
    return BAD_PIN;
}

void(* reset) (void) = 0;

/*
    Notes - Operating Modes
    =======================

KAOS can operate in 3 modes:

Fully automatic: actions for each device are read from the EEPROM and executed automatically
once the device is powered on.

Semi-automatic: As above, but a remote device can change the actions for each device (e.g.
from DEVICE_OFF to DEVICE_BLNKA) and the newly selected action will be executed automatically.
If required, the original (default) action can be read from the EEPROM and restored. An example
remote command could be:
SET LD99 vvv
where vvv is one of the device action values listed in memory_map.h

Fully remote: A remote device can suspend all automatic processing and operate all devices
with remote commands. It is also possible to turn off automatic processing for inputs and
outputs separetely. The command sequence to take over outputs for example would be:
get FLAG
(clear bit FLAG_AUTO_OUTPUTS, put into vvv)
set FLAG vvv
(on/off device commands as required)
SET tttt vvv
SET tttt www
... ...   ...
(restore flag to original value)
set FLAG vvv

*/

void monitorRun() {
    enum cmds { 
    // NOTE: command formats must be used EXACTLY as shown
    // commands marked with asterisks have not been implemented yet
    CMD_BAD,    // error flag
    CMD_SAY,    // SAY.mmmmmmmmmm - echos input buffer contents
    CMD_ALL,    // Change all devices on or off (may add sub-type arg later) ****
    CMD_RUN,    // Run a named pattern
    CMD_WEB,    // WEB.aaaa.nnn - write value nnn to EEPROM byte aaaa
    CMD_REB,    // REB.aaaa - read value from EEPROM byte aaaa 
    CMD_WHO,    // WHO - report board details (device 0)
    CMD_RST,    // RST reset device (e.g. after entering new device into EEPROM)
    CMD_WET,    // WET.aaaa.tttt - write tag tttt to EEPROM starting at address aaaa
    CMD_RET,    // RET.aaaa - read EEPROM tag starting from address aaaa
    CMD_SET,    // SET.tttt.vvv - set device with tag tttt to value vvv (see below)
    CMD_GET,    // GET.tttt.vvv - get something from device with tag ttt (see below)
                // (possibly vvv will be used to get sub-information at some point)
    CMD_DMP,    // DMP.aaaa dump 64 bytes starting at address aaaa (Serial monitor only)
    CMD_WEP,    // WEP.aaaa.ppp - write value of pin ppp to EEPROM address aaaa
    CMD_ACT,    // ACT.tttt.vvv - Set current action for device tttt to vvv (255 to turn off)
                // ACT.tttt - Return current action for device with tag tttt
    CMD_WES,    // WBS.aaaa - Write byte sequence off following lines to eeprom until space
    CMD_WSS,    // WSB.aaaa write byte sequence off following lines to state until space
   // CMD_TTR,    // TTR.tttt.vvv set time to run of device with tag tttt to value vvv
    };
    const static char commands[] PROGMEM = "BADSAYALLRUNWEBREBWHORSTWETRETSETGETDMPWEPACTWESWSS";
    enum tags { // Note these are effectively RESERVED tag names and should NOT be used
                // for LEDs or other devices. Values are read/write unless shown otherwise
    TAG_NONE,   // No built in value
    TAG_SEED,   // Set the seed for the random number generator (write only)
    TAG_HOUR,   // Set or get the hour
    TAG_MINS,   // Set/get the minutes (and the seconds to 0)
    TAG_FLAG,   // Set/get global control values
    TAG_ALLL,   // Refers to all LEDs (write only)
    TAG_ALLD,   // Refers to all devices (write only)
    TAG_TMPT,   // temperature value
    TAG_HMDT,   // Humidity
//    TAG_ACTS,   // Dump shadow actions (read only)
    // TAG_BORD,   // Board identity (read only)
    };
    const static char taglist[] PROGMEM = "NONESEEDHOURMINSFLAGALLLALLDTMPTHMDT";
    static unsigned int eepromAddr = 0;
    static unsigned int stateAddr = 0;
    enum cmds cmdVal = CMD_BAD;
    enum tags tagVal = TAG_NONE;
    char temp[6], c;
    unsigned int addr, t1;
    int block;

    // First, check if we are in the middle of writing a byte stream to eeprom...
    if (eepromAddr > 0) { // read data as a byte, write to current address
        if (monitorBuffer[0] == ' ') {
            eepromAddr = 0; // read no more bytes
        } else {
            // otherwise,
            addr = atoi(monitorBuffer); // used a temp value, not address
            EEPROM.write(eepromAddr++, (byte)addr);
            monitorBuffer[0] = '\0';
        }
        return;
    }
    // Then, check if we are in the middle of writing a byte stream to state...
    if (stateAddr > 0) { // read data as a byte, write to current address
        if (monitorBuffer[0] == ' ') {
            stateAddr = 0; // read no more bytes
        } else {
            // otherwise,
            addr = atoi(monitorBuffer); // used a temp value, not address
            deviceStates[stateAddr++] = (byte)addr;
            monitorBuffer[0] = '\0';
        }
        return;
    }

    // Ignore lines starting with a space or a #
    // to allow for comments in scripts
    if (monitorBuffer[0] == ' ' || monitorBuffer[0] == '#') {
        monitorBuffer[0] = '\0';
        return;
    }

    // Copy data out of buffer into component parts
    strncpy(com, monitorBuffer,3);
    strncpy(arg, monitorBuffer + 4, 4);
    strncpy(val, monitorBuffer + 9, (MONITOR_BUFFER_SIZE - 10));
    // Mark the buffer unused
    monitorBuffer[0] = '\0';

    // Find which command we have
    for (unsigned int i = 0; i < strlen_P(commands); i++) {
        if (strncasecmp_P(com,&commands[i*3], 3) == 0) {
                cmdVal = (cmds)i;
                break;
            }
    }
    // Some commands might then use a built-in tag
    if (cmdVal == CMD_GET || cmdVal == CMD_SET) {
        for (unsigned int i = 0; i < strlen_P(taglist); i++) {
            if (strncasecmp_P(arg,&taglist[i*4], 4) == 0) {
                    tagVal = (tags)i;
                    break;
                }
        }
    }

    switch (cmdVal) {
        case CMD_BAD:
            strcpy_P(monitorBuffer, badCmd);
            break;
        case CMD_WES:
            eepromAddr = atoi(arg);
            break;
        case CMD_WSS:
            stateAddr = atoi(arg);
            break;
        case CMD_WHO:
            sprintf(monitorBuffer,f_03d,EEPROM.read(ADDRESS_UNIT_ID));
            monitorBuffer[4] = ' ';
            monitorBuffer[5] = EEPROM.read(ADDRESS_UNIT_TAG);
            monitorBuffer[6] = EEPROM.read(ADDRESS_UNIT_TAG+1);
            monitorBuffer[7] = EEPROM.read(ADDRESS_UNIT_TAG+2);
            monitorBuffer[8] = EEPROM.read(ADDRESS_UNIT_TAG+3);
            monitorBuffer[9] = '\0';
            break;
        case CMD_SET:
            // Is this tag a reserved name?
            if (tagVal != TAG_NONE) {
                switch (tagVal) {
                    case TAG_SEED:
                        randomSeed(atoi(val));
                        break;
                    case TAG_FLAG:
                        stateWrite(DEVICE_BOARD,STATE_FLAG,(byte)(atoi(val)));
                        break;
                    case TAG_HOUR:
                        addr = atoi(val);
                        if (addr > 23) addr = 0;
                        if ((block = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK) != 0 ) && block != 255) 
                            stateWrite(block,STATE_RTC_HOURS,addr);
                        break;
                    case TAG_MINS:
                        addr = atoi(val);
                        if (addr > 59) addr = 0;
                        if ((block = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK) != 0 ) && block != 255)  {
                            stateWrite(block,STATE_RTC_HOURS,addr);
                            stateWrite(block,STATE_RTC_SECS,0);
                        }
                        break;
                    case TAG_TMPT:
                        if ((block = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK) != 0 ) && block != 255) 
                            stateWrite(block, STATE_DHT_TMPT, atoi(val));
                        break;
                    case TAG_HMDT:
                        if ((block = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK) != 0 ) && block != 255) 
                            stateWrite(block, STATE_DHT_HMDT, atoi(val));
                        break;
                    default:
                        break;
                }
            } else if ((block = findDevice(arg)) == 0) {
                strcpy_P(monitorBuffer,badTag);
            } else { // should be one of our defined names
                setDevice(block, atoi(val)); 
            }                    
            break;
        case CMD_GET:
            // Is this tag a reserved name?
            if (tagVal != TAG_NONE) {
                switch (tagVal) {
                    case TAG_FLAG:
                        sprintf(monitorBuffer,f_03d,stateRead(DEVICE_BOARD,STATE_FLAG));
                        break;
                    case TAG_HOUR:
                        t1 = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(monitorBuffer,f_03d,stateRead(t1,STATE_PARAM1));
                        break;
                    case TAG_MINS:
                        t1 = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(monitorBuffer,f_03d,stateRead(block,STATE_PARAM2));
                        break;
                    case TAG_TMPT:
                        t1 = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(monitorBuffer,f_03d,stateRead(block,STATE_PARAM1));
                        break;
                    case TAG_HMDT:
                        t1 = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(monitorBuffer,f_03d,stateRead(block,STATE_PARAM2));
                        break;
                    default:
                        break;
                }
            } else if ((block = findDevice(arg)) == 0) {
                    strcpy_P(monitorBuffer,badTag);
            } else { // should be one of our defined names
                getDevice(block); 
            }
            break;
        case CMD_SAY: // echo
            monitorBuffer[0] = ' ';
            monitorBuffer[1] = ' ';
            monitorBuffer[2] = ' ';
            break;
        case CMD_RST:
            reset();
            break;
        case CMD_ACT: // Set device action
            if ((block = findDevice(arg)) == 0) {
                strcpy_P(monitorBuffer,badTag);
            } else { // should be one of our defined names
                setupDevice(block, atoi(val));
            }
            break;
        case CMD_WEB: // Write EEPROM byte
        case CMD_WEP: // write EEPROM pin value
            addr = atoi(arg);
            if (!isdigit(arg[3])) { // write to state
                t1 = atoi(val);
                switch(tolower(arg[3])) {
                    case 'a':
                        stateWrite(addr,0,(byte)t1);
                        break;
                    case 'b':
                        stateWrite(addr,1,(byte)t1);
                        break;
                    case 'c':
                        stateWrite(addr,2,(byte)t1);
                        break;
                    case 'd':
                        stateWrite(addr,3,(byte)t1);
                        break;
                    case 'e':
                        stateWrite(addr,4,(byte)t1);
                        break;
                    case 'f':
                        stateWrite(addr,5,(byte)t1);
                        break;
                }
                break;
            }
            if (cmdVal == CMD_WEB) {
                EEPROM.write(addr,atoi(val));
            } else {
                EEPROM.write(addr,str2pin(tolower(val[0]),val[1],val[2]));
            }
            break;
        case CMD_REB: // Read EEPROM byte
            sprintf(monitorBuffer,f_03d,EEPROM.read(atoi(arg)));
            break;
        case CMD_WET: // Write EEPROM tag
            addr = atoi(arg) * EEPROM_BLOCK_SIZE;
            EEPROM.write(addr,val[0]);
            EEPROM.write(addr+1,val[1]);
            EEPROM.write(addr+2,val[2]);
            EEPROM.write(addr+3,val[3]);
            break;
        case CMD_RET: // Read EEPROM tag
            addr = atoi(arg);
            c = EEPROM.read(addr);
            monitorBuffer[0] = (char)(isalnum(c) ? c : '?');
            c = EEPROM.read(addr + 1);
            monitorBuffer[1] = isalnum(c) ? c : '?';
            c = EEPROM.read(addr + 2);
            monitorBuffer[2] = isalnum(c) ? c : '?';
            c = EEPROM.read(addr + 3);
            monitorBuffer[3] = isalnum(c) ? c : '?';
            monitorBuffer[4] = '\0';
            break;
        case CMD_DMP:
#ifdef MONITOR_DEBUG
            addr = atoi(arg); // addr holds starting block no.
            t1 = addr * EEPROM_BLOCK_SIZE; // convert block no. to absolute address
            Serial.print("    ");
            for (int i = 0; i < EEPROM_BLOCK_SIZE; i++) {
                sprintf(temp,f_03d,i);
                Serial.print(temp);
            }
            Serial.println(F("                A    B    C    D    E    F"));
            for (int j = 0; j < 8; j++) {
                sprintf(temp,"%03u ", (t1+(EEPROM_BLOCK_SIZE*j))/10);
                Serial.print(temp);
                for (int i = 0; i < EEPROM_BLOCK_SIZE; i++) {
                    sprintf(temp,f_03d,EEPROM.read(t1+(EEPROM_BLOCK_SIZE*j)+i));
                    Serial.print(temp);
                }
                for (int i = 0; i < EEPROM_BLOCK_SIZE; i++) {
                    c = EEPROM.read(t1+(EEPROM_BLOCK_SIZE*j)+i);
                    if (isalnum(c)) {
                        Serial.print(c);
                    } else {
                        Serial.print('.');
                    }
                    Serial.print(" ");
                }
                for (int i = 0; i < STATE_BLOCK_SIZE; i++) {
                    sprintf(temp,f_03d,stateRead(addr + j,i));
                    Serial.print(temp);
                }
                Serial.print('\n');
            }
#else
            strcpy_P(monitorBuffer,F("no debug"));
#endif
            break;
        default:
            break;
    }
}

bool monitorInput () {
    static int ptr = 0;
    char in;

    while(Serial.available()) {
        in = Serial.read();
        if (stateRead(DEVICE_BOARD,STATE_FLAG) & FLAG_ECHO) {
            Serial.print(in);
        }
        if (in == '\b') {
            if (ptr > 0) ptr--;
            return false;
        }
        if (in == '\n') {
            ptr = 0;
            return true;
        } else if (ptr < MONITOR_BUFFER_SIZE) {
            monitorBuffer[ptr++] = in;
        }
    }
    return false;
}

void monitorOutput () {
    if (monitorBuffer[0] != '\0')
        Serial.println(monitorBuffer);
    // Clean up buffer, ready for next time
    for (int i = 0; i < MONITOR_BUFFER_SIZE - 1; i++) {
        monitorBuffer[i] = ' ';
    }
    monitorBuffer[MONITOR_BUFFER_SIZE - 1] = '\0';
}
