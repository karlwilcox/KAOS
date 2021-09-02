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

const char badAddr[] PROGMEM = "Bad addr";
const char badCmd[] PROGMEM = "Bad cmd";
const char badTag[] PROGMEM = "Bad tag";
const char badDev[] PROGMEM = "Bad dev";
const char badPin[] PROGMEM = "Bad pin";
extern char com[], arg[], val[];
extern byte deviceStates[];
extern char f_03d[];
extern unsigned int runAddr;

unsigned int char2int(char * in) {
    unsigned int retval = 0;
    unsigned int len;
    unsigned int i;
    len = strlen(in);
    // if it is 1 or 2 digits must be decimal
    if (len == 0) return 0;
    if (in[0] == '\'' && len > 1) { // treat as a character
        return (byte)in[1];
    }
    if (len <= 2) return atoi(in);
    switch(in[1]) {
        case 'x':
        case 'X':
            sscanf(in,"%x",&retval);
            break;
        case 'b':
        case 'B':
            // have to do this ourselves
            for (i = 2; i < len; i++) {
                if (in[i] == '1') {
                    retval = (retval << 1) + 1;
                } else if (in[i] == '0') {
                    retval = retval << 1;
                } else { // not binary digit
                    break;
                }
            }
            break;
        default: // should be decimal
            retval = atoi(in);
            break;
    }

    return retval;
}


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

void padTag() {
    if (arg[1] == '\0') {
        arg[1] = arg[2] = arg[3] = ' ';
        arg[4] = '\0';
    } else if (arg[2] == '\0') {
        arg[2] = arg[3] = ' ';
        arg[3] = ' ';
        arg[4] = '\0';
    } else if (arg[3] == '\0') {
        arg[3] = ' ';
        arg[4] = '\0';
    }
}

void monitorRun() {
    enum cmds { 
        // in the following commands the abbreviations are as follows:
        // dddf - up to 3 decimal digits for the device number followed by a hex digit
        //        for the field (byte) These correspond to the row/column titles
        //         shown in the DMP command
        // ddd  - up to 3 decimal digits for a device number
        // vvv  - up to 3 decimal digits for a byte value
        // tttt - up to 4 printable characters for a tag name (padding with trainling spaces)
        // ppp  - up to 3 decimal digits for a digital pin (>100 are pins on SRs)
        //        or A followed by one or 2 digits for analog input pins
        // aaaa - up to 4 decimal digits, an abosolute EEPROM address or offset into
        //        the deviceStates array
    CMD_BAD,    // error flag
    CMD_WDB,    // WRITE DEVICE BYTE write a byte to a device: wdb dddf vvv
    CMD_RDB,    // READ DEVICE BYTE read a byte from device: wdb dddf
    CMD_RST,    // RESET reset device (e.g. after entering new device into EEPROM)
    CMD_TAG,    // TAG set or read a device tag : TAG ddd [tttt]
    CMD_SET,    // SET set a device (or reserved tag) to a value: SET ddd vvv
    CMD_GET,    // GET get a value from a device (or reserved tag): GET ddd
    CMD_DMP,    // DUMP STATE display one or more device states: DMP [ddd] [vvv]
                // starting at ddd (default 0) and including vvv devices (default 8)
    CMD_WDP,    // WRITE DEVICE PIN write a pin value to a device block: WDP ddd ppp
    CMD_ACT,    // SET ACTION set device to a specific action:  ACT tttt vvv
                // same as writing the STATE_ACTION value but also runs setupDevice()
    CMD_WES,    // WRITE EEPROM STREAM write a sequence of bytes to EEPROM, one per line
                // starting at aaaa, end with a space line: WES aaa
    CMD_WSS,    // WRITE STATE STREAM write a sequence of bytes to state, one per line
                // starting offset aaaa, end with space line: WSSS aaa
    CMD_STO,    // SAVE copy state into EEPROM : SAV tttt
    CMD_SLP,    // WAI vvv suspend command input processing for time vvv
    CMD_WCS,    // WRITE CHARACTER STREAM write a sequence of bytes to eeprom, including \r
                // starting offset aaaa, end with space line: WSSS aaa
    CMD_RUN,    // RUN aaaa Execute commands from EEPROM starting at aaaa
    };
    const static char commands[] PROGMEM = "BADWDBRDBRSTTAGSETGETDMPWDPACTWESWSSSTOSLPWCSRUN";
    enum tags { // Note these are effectively RESERVED tag names and should NOT be used
                // for LEDs or other devices. Values are read/write unless shown otherwise
    TAG_NONE,   // No built in value
    TAG_SEED,   // Set the seed for the random number generator (write only)
    TAG_HOUR,   // Set or get the hour
    TAG_MINS,   // Set/get the minutes (and the seconds to 0)
    TAG_ECHO,   // Set/get whether monitor input should be echoed
    TAG_RUN,    // Set/get whether to automatically run all devices
    TAG_ALLD,   // Refers to all devices (write only)
    TAG_TMPT,   // temperature value
    TAG_HMDT,   // Humidity
    TAG_TAGS,   // List all device tags (read only)
    };
    const static char taglist[] PROGMEM = "NONESEEDHOURMINSECHORUN ALLDTMPTHMDTTAGS";
    static unsigned int streamAddr = 0;
    static enum streamTargets { STREAM_NONE, STREAM_EEPROM, STREAM_STATE, STREAM_EEPROM_CHAR } streamTarget = STREAM_NONE;
    enum cmds cmdVal = CMD_BAD;
    enum tags tagVal = TAG_NONE;
    char temp[6], c;
    unsigned int t1, t2; // temporaries, various uses
    int block;

    if (stateRead(DEVICE_BOARD, STATE_FLAG) & FLAG_MULTILINE) {
        if (val[0] == ' ' || val[0] == 'x' || val[0] == 'X') { // TODO Check this and remove if space works
            streamAddr = 0; // read no more bytes
            streamTarget = STREAM_NONE;
            runAddr = 0;
            stateBitClear(DEVICE_BOARD, STATE_FLAG, FLAG_MULTILINE);
        } else {
            if (streamTarget == STREAM_EEPROM_CHAR) {
                t1 = 0;
                while(val[t1] != '\0') {
                    EEPROM.write(streamAddr++, val[t1]);
                }
                EEPROM.write(streamAddr++,'\r');
            } else {
                t1 = char2int(val);
                if (streamTarget == STREAM_EEPROM) {
                    EEPROM.write(streamAddr++, (byte)t1);
                } else if (streamTarget == STREAM_STATE) {
                    deviceStates[streamAddr++] = (byte)t1;
                }
            }
            if (stateRead(DEVICE_BOARD, STATE_FLAG) & FLAG_ECHO) {
                sprintf(val,"%04u ", streamAddr);
            } else {
                val[0] = '\0';
            }
        }
        return;
    }

    // Find which command we have
    for (unsigned int i = 0; i < strlen_P(commands); i++) {
        if (strncasecmp_P(com,&commands[i*3], 3) == 0) {
                cmdVal = (cmds)i;
                break;
            }
    }
    // Some commands might then use a built-in tag
    if (cmdVal == CMD_GET || cmdVal == CMD_SET || cmdVal == CMD_STO) {
        padTag();
        for (unsigned int i = 0; i < strlen_P(taglist); i++) {
            if (strncasecmp_P(arg,&taglist[i*4], 4) == 0) {
                    tagVal = (tags)i;
                    break;
                }
        }
    }

    switch (cmdVal) {
        case CMD_BAD:
            strcpy_P(val, badCmd);
            break;
        case CMD_RUN:
            runAddr = atoi(arg);
            break;
        case CMD_WES:
            stateBitSet(DEVICE_BOARD, STATE_FLAG, FLAG_MULTILINE);
            streamTarget = STREAM_EEPROM;
            streamAddr = char2int(arg);
            val[0] = '\0';
            break;
        case CMD_WSS:
            stateBitSet(DEVICE_BOARD, STATE_FLAG, FLAG_MULTILINE);
            streamTarget = STREAM_STATE;
            streamAddr = char2int(arg);
            val[0] = '\0';
            break;
        case CMD_WCS:
            stateBitSet(DEVICE_BOARD, STATE_FLAG, FLAG_MULTILINE);
            streamTarget = STREAM_EEPROM_CHAR;
            streamAddr = char2int(arg);
            val[0] = '\0';
            break;
        case CMD_SLP:
            stateBitSet(DEVICE_BOARD, STATE_FLAG, FLAG_SLEEP);
            stateWrite(DEVICE_BOARD, STATE_TTR,char2int(arg));
            val[0] = '\0';
            break;
        case CMD_SET:
            // Is this tag a reserved name?
            if (tagVal != TAG_NONE) {
                switch (tagVal) {
                    case TAG_SEED:
                        randomSeed(char2int(val));
                        break;
                    case TAG_ECHO:
                        if (char2int(val) > 0) {
                            stateBitSet(DEVICE_BOARD,STATE_FLAG,FLAG_ECHO);
                        } else {
                            stateBitClear(DEVICE_BOARD,STATE_FLAG,FLAG_ECHO);
                        }
                        break;
                    case TAG_RUN:
                        if (char2int(val) > 0) {
                            stateBitSet(DEVICE_BOARD,STATE_FLAG,FLAG_RUN);
                        } else {
                            stateBitClear(DEVICE_BOARD,STATE_FLAG,FLAG_RUN);
                        }
                        break;
                    case TAG_HOUR:
                        t1 = char2int(val);
                        if (t1 > 23) t1 = 0;
                        if ((block = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK) != 0 ) && block != 255) 
                            stateWrite(block,STATE_PARAM1,t1);
                        break;
                    case TAG_MINS:
                        t1 = char2int(val);
                        if (t1 > 59) t1 = 0;
                        if ((block = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK) != 0 ) && block != 255)  {
                            stateWrite(block,STATE_PARAM2,t1);
                            stateWrite(block,STATE_PARAM3,0);
                        }
                        break;
                    case TAG_TMPT:
                        if ((block = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK) != 0 ) && block != 255) 
                            stateWrite(block, STATE_PARAM1, char2int(val));
                        break;
                    case TAG_HMDT:
                        if ((block = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK) != 0 ) && block != 255) 
                            stateWrite(block, STATE_PARAM2, char2int(val));
                        break;
                        // ToDo implement TAG_ALLD
                    default:
                        break;
                }
                val[0] = '\0';
            } else if ((block = findDevice(arg)) == 0) {
                strcpy_P(val,badTag);
            } else { // should be one of our defined names
                setDevice(block, char2int(val)); 
                val[0] = '\0';
            }                    
            break;
        case CMD_GET:
            // Is this tag a reserved name?
            if (tagVal != TAG_NONE) {
                switch (tagVal) {
                    case TAG_RUN:
                        val[0] = (stateRead(DEVICE_BOARD,STATE_FLAG) & FLAG_RUN) ? '1' : '0';
                        val[1] = '\0';
                        break;
                    case TAG_ECHO:
                        val[0] = (stateRead(DEVICE_BOARD,STATE_FLAG) & FLAG_ECHO) ? '1' : '0';
                        val[1] = '\0';
                        break;
                    case TAG_HOUR:
                        t1 = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(val,f_03d,stateRead(t1,STATE_PARAM1));
                        break;
                    case TAG_MINS:
                        t1 = stateRead(DEVICE_BOARD, STATE_RTC_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(val,f_03d,stateRead(t1,STATE_PARAM2));
                        break;
                    case TAG_TMPT:
                        t1 = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(val,f_03d,stateRead(t1,STATE_PARAM1));
                        break;
                    case TAG_HMDT:
                        t1 = stateRead(DEVICE_BOARD, STATE_DHT_BLOCK);
                        if (t1 != 0 && t1 != 255)
                            sprintf(val,f_03d,stateRead(t1,STATE_PARAM2));
                        break;
                    case TAG_TAGS:
                        for (t1 = 1; t1 < MAX_DEVICES; t1++) {
                            t2 = eepromRead(t1,EEPROM_ACTION);
                            if (t2 == DEVICE_END) break;
                            if (t2 == DEVICE_CONT || t2 == DEVICE_DELETED) continue;
                            sprintf(val,"%c%c%c%c %03u", 
                                eepromRead(t1,EEPROM_TAG),
                                eepromRead(t1,EEPROM_TAG+1),
                                eepromRead(t1,EEPROM_TAG+2),
                                eepromRead(t1,EEPROM_TAG+3),
                                eepromRead(t1,EEPROM_ACTION));
                            Serial.println(val);
                        }
                        val[0] = '\0';
                        break;
                    default:
                        break;
                }
            } else if ((block = findDevice(arg)) == 0) {
                    strcpy_P(val,badTag);
            } else { // should be one of our defined names
                // sets val
                if (!getDevice(block)) {
                    strcpy_P(val,badDev);
                }
            }
            break;
        case CMD_STO:
            if ((block = findDevice(arg)) == 0) {
                    strcpy_P(val,badTag);
            } else { // should be one of our defined names
                eepromWrite(block, EEPROM_PARAM1, stateRead(block, STATE_PARAM1));
                eepromWrite(block, EEPROM_PARAM2, stateRead(block, STATE_PARAM2));
                eepromWrite(block, EEPROM_PARAM3, stateRead(block, STATE_PARAM3));
                eepromWrite(block, EEPROM_ACTION, stateRead(block, STATE_ACTION));
                eepromWrite(block, EEPROM_VALUE, stateRead(block,EEPROM_VALUE));
                val[0] = '\0';
            }
            break;
        case CMD_RST:
            reset();
            break;
        case CMD_ACT: // Set device action
            if ((block = findDevice(arg)) == 0) {
                strcpy_P(val,badTag);
            } else { // should be one of our defined names
                t1 = char2int(val);
                stateWrite(block, STATE_ACTION, t1);
                setupDevice(block);
                val[0] = '\0';
            }
            break;
        case CMD_WDB: // Write device byte
        case CMD_WDP: // write device pin value
        case CMD_RDB: // read device byte
            if (arg[1] == '\0') { // need block + offset
                strcpy_P(val,badDev);
                break;
            } // else
            if (arg[2] == '\0') { // 1 digit for block, 1 for offset
                c = arg[1];
                arg[1] = '\0';
            } else if (arg[3] == '\0') {
                c = arg[2];
                arg[2] = '\0';
            } else {
                c = arg[3];
                arg[3] = '\0';
            } // we now have the last character in c, and the device block
              // number in arg;
            t1 = atoi(arg);
            if (isdigit(c)) { // write to eeprom, offset t1
                if (cmdVal == CMD_WDB) {
                    eepromWrite(t1, (c - '0'),(byte)char2int(val));
                    val[0] = '\0';
                } else if (cmdVal == CMD_RDB) {
                    sprintf(val,f_03d,eepromRead(t1,(c - '0')));
                } else {
                    t2 = str2pin(tolower(val[0]),val[1],val[2]);
                    if (t2 == BAD_PIN) {
                        strcpy_P(val,badPin);
                    } else {
                        eepromWrite(t1, (c - '0'), (byte)t2);
                        val[0] = '\0';
                    }
                }
            } else {
                c = tolower(c);
                if (c >= 'a' && c <= 'f') { // read/write state
                    if (cmdVal == CMD_WDB) {
                        stateWrite(t1, (c - 'a'), char2int(val));
                        val[0] = '\0';
                    } else {
                        sprintf(val,f_03d,stateRead(t1, (c - 'a')));
                    }
                } else { 
                    strcpy_P(val, badDev);
                }
            }
            break;
        case CMD_TAG:
            t1 = char2int(arg);
            if (t1 >= MAX_DEVICES) {
                strcpy_P(val, badAddr);
                break;
            }
            if (val[0] == '\0') { // no value given, read existing
                c = (char)eepromRead(t1,EEPROM_TAG);
                val[0] = isprint(c) ? c : '.';
                c = (char)eepromRead(t1,EEPROM_TAG + 1);
                val[1] = isprint(c) ? c : '.';
                c = (char)eepromRead(t1,EEPROM_TAG + 2);
                val[2] = isprint(c) ? c : '.';
                c = (char)eepromRead(t1,EEPROM_TAG + 3);
                val[3] = isprint(c) ? c : '.';
                val[4] = '\0';
            } else { // write value 
                eepromWrite(t1,EEPROM_TAG,val[0]);
                eepromWrite(t1,EEPROM_TAG+1,val[1] != '\0' ? val[1] : ' ');
                eepromWrite(t1,EEPROM_TAG+2,val[2] != '\0' ? val[2] : ' ');
                eepromWrite(t1,EEPROM_TAG+3,val[3] != '\0' ? val[3] : ' ');
                val[0] = '\0';
            }
            break;
        case CMD_DMP:
            t1 = char2int(arg); // t1 holds starting block no.
            t2 = char2int(val); // value might be limit
            if (t2 == 0) t2 = 1; // default rows to show
            // print address information line
            if (t2 > 1) { // count
                Serial.print("    ");
                for (int i = 0; i < EEPROM_BLOCK_SIZE; i++) {
                    sprintf(temp,f_03d,i);
                    Serial.print(temp);
                }
                Serial.println(F("                     A   B   C   D   E   F"));
            }
            // print content lines
            for (unsigned int j = t1; (j < (unsigned int)(t1 + t2)) && ((unsigned int)(t1 + t2) < MAX_DEVICES); j++) {
                // print starting address
                sprintf(temp,"%03u ", (j*EEPROM_BLOCK_SIZE)/10);
                Serial.print(temp);
                for (int i = 0; i < EEPROM_BLOCK_SIZE; i++) {
                    sprintf(temp,f_03d,EEPROM.read((j*EEPROM_BLOCK_SIZE)+i));
                    Serial.print(temp);
                }
                for (int i = 0; i < EEPROM_BLOCK_SIZE; i++) {
                    c = EEPROM.read((j*EEPROM_BLOCK_SIZE)+i);
                    if (isalnum(c)) {
                        Serial.print(c);
                    } else {
                        Serial.print('.');
                    }
                    Serial.print(" ");
                }
                for (int i = 0; i < STATE_BLOCK_SIZE; i++) {
                    sprintf(temp,f_03d,stateRead(j,i));
                    Serial.print(temp);
                }
                Serial.print('\n');
            }
            val[0] = '\0';
            break;
        default:
            break;
    }
}

char getInputChar() {
    if (runAddr != 0) {
        return (EEPROM.read(runAddr++));
    } else if (Serial.available()) {
        return (Serial.read());
    }
    return '\0';
}

bool monitorInput () {
    static int ptr = 0;
    static enum parts { COMMAND, ARGUMENT, VALUE, SPACE1, SPACE2, COMMENT } part = COMMAND; 
    char in;

    while ((in = getInputChar()) != '\0') {
        if (in == 0x15) { // control - u
            com[0] = '\0';
            arg[0] = '\0';
            val[0] = '\0';
            ptr = 0;
            part = COMMAND;
            Serial.print('\r');
            return false;
        }
        if (stateRead(DEVICE_BOARD,STATE_FLAG) & FLAG_ECHO) {
            Serial.print(in);
        }
        if (in == '\b') {
            if (ptr > 0) ptr--;
            return false;
        }
        if (in == '\n') {
            ptr = 0;
            part = COMMAND;
            return true;
        } else {
            if (in == '#') part = COMMENT;
            switch (part) {
                case COMMAND:
                    if (isspace(in)) {
                        part = SPACE1;
                    } else {
                        if (ptr < MONITOR_COMMAND_SIZE) {
                            com[ptr++] = in;
                            com[ptr] = '\0';
                        }
                    }
                    break;
                case SPACE1:
                    if (!isspace(in)) {
                        ptr = 1;
                        arg[0] = in;
                        arg[1] = '\0';
                        part = ARGUMENT;
                    }
                    break;
                case ARGUMENT:
                    if (isspace(in)) {
                        part = SPACE2;
                    } else {
                        if (ptr < MONITOR_ARGUMENT_SIZE) {
                            arg[ptr++] = in;
                            arg[ptr] = '\0';
                        }
                    }
                    break;
                case SPACE2:
                    if (!isspace(in)) {
                        ptr = 1;
                        val[0] = in;
                        val[1] = '\0';
                        part = VALUE;
                    }
                    break;
                case VALUE:
                    if (ptr < MONITOR_VALUE_SIZE) {
                        val[ptr++] = in;
                        val[ptr] = '\0';
                    }
                    break;
                case COMMENT:
                    // do nothing
                default:
                    break;
            }
        }
    }
    return false;
}

void monitorOutput () {
    if (val[0] != '\0')
        Serial.println(val);
    com[0] = '\0';
    arg[0] = '\0';
    val[0] = '\0';
}
