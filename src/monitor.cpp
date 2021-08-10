/*
 * Monitor code
 */
#include "monitor.h"
#include "eeprom_layout.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "devices.h"

int monitorFlags = 0;
const char badAddr[] PROGMEM = "Bad addr";
const char badCmd[] PROGMEM = "Bad cmd";
const char badTag[] PROGMEM = "Bad tag";
const char badDev[] PROGMEM = "Bad dev";
extern char monitorBuffer[];
extern int hours, minutes, seconds, temperature, humidity;
extern char com[], arg[], val[];
extern unsigned int flags;
extern byte deviceActions[];

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
where vvv is one of the device action values listed in eeprom_layout.h

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
    const static char f_03d[] = "%03u";
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
    };
    const static char commands[] PROGMEM = "BADSAYALLRUNWEBREBWHORSTWETRETSETGETDMPWEPACT";
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
    TAG_ACTS,   // Dump shadow actions (read only)
    // TAG_BORD,   // Board identity (read only)
    };
    const static char taglist[] PROGMEM = "NONESEEDHOURMINSFLAGALLLALLDTMPTHMDTACTS";
    enum cmds cmdVal = CMD_BAD;
    enum tags tagVal = TAG_NONE;
    char temp[6], c;
    unsigned int addr;
    int block;

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
        case CMD_WHO:
            sprintf(monitorBuffer,f_03d,EEPROM.read(UNITID));
            monitorBuffer[3] = ' ';
            monitorBuffer[4] = ' ';
            monitorBuffer[5] = EEPROM.read(UNITTYPE);
            monitorBuffer[6] = EEPROM.read(UNITTYPE+1);
            monitorBuffer[7] = EEPROM.read(UNITTYPE+2);
            monitorBuffer[8] = EEPROM.read(UNITTYPE+3);
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
                        flags = atoi(val);
                        break;
                    case TAG_HOUR:
                        hours = atoi(val);
                        if (hours > 23) hours = 0;
                        break;
                    case TAG_MINS:
                        minutes = atoi(val);
                        if (minutes > 59) minutes = 0;
                        seconds = 0;
                        break;
                    case TAG_ALLL:
                    case TAG_ALLD:
                        allDevices(atoi(val));
                        break;
                    case TAG_TMPT:
                        temperature = atoi(val);
                        break;
                    case TAG_HMDT:
                        humidity = atoi(val);
                        break;
                    default:
                        break;
                }
            } else if ((block = findDevice(arg)) == 0) {
                strcpy_P(monitorBuffer,badTag);
            } else { // should be one of our defined names
                setDevice(block);
            }
            break;
        case CMD_GET:
            // Is this tag a reserved name?
            if (tagVal != TAG_NONE) {
                switch (tagVal) {
                    case TAG_SEED:
                    case TAG_ALLL:
                    case TAG_ALLD:
                        strcpy_P(monitorBuffer,badTag);
                        break;
                    case TAG_FLAG:
                        sprintf(monitorBuffer,f_03d,flags);
                        break;
                    case TAG_HOUR:
                        sprintf(monitorBuffer,f_03d,hours);
                        break;
                    case TAG_MINS:
                        sprintf(monitorBuffer,f_03d,minutes);
                        break;
                    case TAG_TMPT:
                        sprintf(monitorBuffer,f_03d,temperature);
                        break;
                    case TAG_HMDT:
                        sprintf(monitorBuffer,f_03d,humidity);
                        break;
#ifdef MONITOR_DEBUG
                    case TAG_ACTS:
                        for (addr = 0; addr < MAX_DEVICES; addr += 1) {
                            sprintf(temp,f_03d,(int)deviceActions[addr]);
                            Serial.print(temp);
                            if (addr > 0 && addr % 8 == 0) {
                                Serial.println(" ");
                            } else {
                                Serial.print(" ");
                            }
                        }
                        Serial.println(" ");
                        break;
#endif
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
            strcpy(monitorBuffer, arg);
            strcat(monitorBuffer," ");
            strcat(monitorBuffer, val);
            break;
        case CMD_RST:
            reset();
            break;
        case CMD_ACT: // Set device action
            if ((block = findDevice(arg)) == 0) {
                strcpy_P(monitorBuffer,badTag);
            } else { // should be one of our defined names
                addr = atoi(val); // not really address, just reusing variable
                if (addr == 0) { // read request
                    sprintf(monitorBuffer,f_03d,(int)deviceActions[block]);
                } else { // write request
                    deviceActions[block] = (byte)addr;
                    // if this is just plain on or off, make sure it is actioned
                    if (addr == DEVICE_ON) {
                        setDevice(block, 1);
                    } else if (addr == DEVICE_OFF || addr == 0) {
                        setDevice(block, 0);
                    }
                }
            }
            break;
        case CMD_WEB: // Write EEPROM byte
        case CMD_WEP: // write EEPROM pin value
            addr = atoi(arg);
            if (addr == 0) {
                strcpy_P(monitorBuffer, badAddr);
            } else {
                if (cmdVal == CMD_WEB) {
                    EEPROM.write(addr,atoi(val));
                } else {
                    EEPROM.write(addr,str2pin(val[0],val[1],val[2]));
                }
            }
            break;
        case CMD_REB: // Read EEPROM byte
            sprintf(monitorBuffer,f_03d,EEPROM.read(atoi(arg)));
            break;
        case CMD_WET: // Write EEPROM tag
            addr = atoi(arg);
            if (addr == 0) {
                strcpy_P(monitorBuffer, badAddr);
            } else {
                EEPROM.write(addr,val[0]);
                EEPROM.write(addr+1,val[1]);
                EEPROM.write(addr+2,val[2]);
                EEPROM.write(addr+3,val[3]);
            }
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
            addr = atoi(arg);
            addr -= addr % 16; // round down to nearest 16
            Serial.print("     ");
            for (int i = 0; i < 16; i++) {
                sprintf(temp,f_03d,i);
                Serial.print(temp);
                Serial.print(" ");
            }
            Serial.println(" ");
            for (int j = 0; j < 8; j++) {
                sprintf(temp,"%04u ", addr+(16*j));
                Serial.print(temp);
                for (int i = 0; i < 16; i++) {
                    sprintf(temp,f_03d,EEPROM.read(addr+(16*j)+i));
                    Serial.print(temp);
                    Serial.print(" ");
                }
                for (int i = 0; i < 16; i++) {
                    c = EEPROM.read(addr+(16*j)+i);
                    if (isalnum(c)) {
                        Serial.print(c);
                    } else {
                        Serial.print('.');
                    }
                    Serial.print(" ");
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
        if (flags & FLAG_ECHO) {
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
