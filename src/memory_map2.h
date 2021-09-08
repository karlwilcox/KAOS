/*
 * This file defines the standard layout of device data
 * for all Arduino devices using KAOS (Karl's Arduino
 * Operting System)
 */

/*
  Memory is laid out as a series of 10 byte "permanent" data in EEPROM, along with
  corresponding 6 byte blocks of volatile data. The precise meaning of most of the
  data bytes is device dependant, as described below.
*/

// some useful macros
#define EEPROM_BLOCK_SIZE 10          // space set aside for device information
#define STATE_BLOCK_SIZE 6
#define EEPROM_ADDRESS(a,b) (((a) * EEPROM_BLOCK_SIZE) + (b))
#define eepromRead(a,b) EEPROM.read(EEPROM_ADDRESS(a,b))
#define eepromWrite(a,b,c) EEPROM.write(EEPROM_ADDRESS(a,b),(byte)(c))

#define STATE_ADDRESS(a,b) (((a) * STATE_BLOCK_SIZE) + (b))
#define stateRead(a,b) (deviceStates[STATE_ADDRESS(a,b)])
#define stateWrite(a,b,c) (deviceStates[STATE_ADDRESS(a,b)] = (byte)(c))
#define stateBitSet(a,b,c) (deviceStates[STATE_ADDRESS(a,b)] |= (c))
#define stateBitClear(a,b,c) (deviceStates[STATE_ADDRESS(a,b)] &= (~(c)))
#define statePtr(a,b) (&deviceStates[STATE_ADDRESS(a,b)])
#define stateFromEEPROM(a,b,c) (stateWrite((a),(b),eepromRead((a),(c))))

//////////////////////// Block 0 //////////////////////////////////////////

#define EEPROM_UNIT_ID 4                // unique ID for each board, used for I2C comms
#define EEPROM_FLAG 5
#define EEPROM_TAG 0
#define STATE_FLAG 5
#define BAD_PIN 255
// 7 to 9 UNUSED


// echo input over the serial line back to sender
#define FLAG_ECHO           0x01
// Automatically run all devices as per current actions
#define FLAG_RUN   0x02
#define FLAG_MULTILINE 0x04
#define FLAG_SLEEP 0x08
#define FLAG_PROG 0x40


/////////////////////// Layout of Block 0 (the board) /////////////////////////

#define DEVICE_BOARD 0 // location of board info, block 0

/* Layout of eeprom block 0 - board block

+--------+---------------------+----------------------------+--------------------------+
| Address| Content             | Notes                      | Defined                  |
+--------+---------------------+----------------------------+--------------------------+
| 0000   | Board tag           |                            | ADDRESS_BOARD_TAG        |
+--------+                     |                            |                          |
| 0001   |                     |                            |                          |
+--------+                     |                            |                          |
| 0002   |                     |                            |                          |
+--------+                     |                            |                          |
| 0003   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 0004   | Board  ID           | used as the I2C address    | ADDRESS_BOARD_ID         |
+--------+---------------------+----------------------------+--------------------------+
| 0005   | Default flag value  | See config.h for mapping   | ADDRESS_DEFAULT_FLAG     |
+--------+---------------------+----------------------------+--------------------------+
| 0006   | Unused              |                            |                          |
| to     |                     |                            |                          |
| 0009   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+

  Layout of state block 0 - device block

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0 A  |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   1 B  |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   2 C  | Block of 1st SR     |                            |  STATE_1ST_SR            |
+--------+---------------------+----------------------------+--------------------------+
|   3 D  | Block of DHT11      |                            |  STATE_DHT_BLOCK         |
+--------+---------------------+----------------------------+--------------------------+
|   4 E  | Block of RTC        |                            |  STATE_RTC_BLOCK        |           
+--------+---------------------+----------------------------+--------------------------+
|   5 F  |Current flag value   |                            | STATE_FLAG               |
+--------+---------------------+----------------------------+--------------------------+

*/

#define ADDRESS_BOARD_TAG 0
#define ADDRESS_BOARD_ID 4
#define ADDRESS_DEFAULT_FLAG 5

#define STATE_DHT_BLOCK 3 
#define STATE_RTC_BLOCK 4 
#define STATE_1ST_SR 2

/* 
  Special values for offset 0:
  - If a device is present at this block, EEPROM_BLOCK_TYPE will contain a printable character
  - Otherwise it could be one of these special values
*/

#define BLOCK_END_MARKER 255          // Marks the end of the list of devices (default, unwritten EEPROM)
#define BLOCK_CONT 254         // This device needs a further 10 bytes of data
#define BLOCK_DELETED 253      // Marks a deleted device, ignore this
#define BLOCK_SUSPENDED 252
#define DEVICE_UPPER 250        // Everything above this can be ignored

/*
 Definitions of units for TTR (top 2 bits)
*/

#define TTR_UNIT_20ms 0    // ends up as 0b00000000    (0)
#define TTR_UNIT_100ms 64   // ends up as 0b01000000   (64)
#define TTR_UNIT_1s 128      // ends up as 0b10000000  (128)
#define TTR_UNIT_10s 192     // ends up as 0b11000000  (192)o



/* Layout of eeprom block

+--------+---------------------+----------------------------+--------------------------+
| Address| Content             | Notes                      | Defined                  |
+--------+---------------------+----------------------------+--------------------------+
| 0000   | tag                 | Printable characters only  | EEPROM_TAG               |
+--------+                     |                            |                          |
| 0001   |                     |                            |                          |
+--------+                     |                            |                          |
| 0002   |                     |                            |                          |
+--------+                     |                            |                          |
| 0003   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 0004   | Pin 1 / Param 0     |                            | EEPROM_PIN1              |
|        |                     |                            | EEPROM_PARAM0            |
+--------+---------------------+----------------------------+--------------------------+
| 0005   | Pin 2 / Param 1     |                            | EEPROM_PIN2              |
|        |                     |                            | EEPROM_PARAM1            |
+--------+---------------------+----------------------------+--------------------------+
| 0006   | Pin 3 / Param 2     |                            | EEPROM_PIN3              |
|        |                     |                            | EEPROM_PARAM2            |
+--------+---------------------+----------------------------+--------------------------+
| 0007   | Pin 4 / Param 3     |                            | EEPROM_PIN4              |
|        |                     |                            | EEPROM_PARAM3            |
|        | Target TTR          |                            | EEPROM_RUNTIME           |
+--------+---------------------+----------------------------+--------------------------+
| 0008   | Block Type          |                            | EEPROM_BLOCK_TYPE        |
+--------+---------------------+----------------------------+--------------------------+
| 0009   | default value       |                            | EEPROM_VALUE             |
|        |                     |                            | EEPROM_SIGGEN            |
| 0009   | Param 4             |                            | EEPROM_PARAM4            |
+--------+---------------------+----------------------------+--------------------------+

Layout of state block

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  | 
+--------+---------------------+----------------------------+--------------------------+
|   0 A  | Current TTR         | Auto-decremented           | STATE_TTR                |
+--------+---------------------+----------------------------+--------------------------+
|   1 B  | Parameter 1         |                            | STATE_PARAM1             |
+--------+---------------------+----------------------------+--------------------------+
|   2 C  | Parameter 2         |                            | STATE_PARAM2             |
+--------+---------------------+----------------------------+--------------------------+
|   3 D  | Parameter 3         |                            | STATE_PARAM3             |
|        | Target TTR          |                            | STATE_RUNTIME            |
+--------+---------------------+----------------------------+--------------------------+
|   4 E  | Current action      |                            | STATE_SIGGEN             |
|        | Paramter 4          |                            | STATE_PARAM4             |
+--------+---------------------+----------------------------+--------------------------+
|   5 F  |Current value        |                            | STATE_VALUE              |
+--------+---------------------+----------------------------+--------------------------+

*/

// Offsets or pins
#define EEPROM_PIN1 4            // Use WEP to populate this
#define EEPROM_PIN2 5            // Use WEP to populate this
#define EEPROM_PIN3 6             // Use WEP to populate this
#define EEPROM_PIN4 7             // Use WEP to populate this

// Offsets for parameters
#define EEPROM_PARAM0 4
#define EEPROM_PARAM1 5
#define EEPROM_PARAM2 6
#define EEPROM_PARAM3 7
#define EEPROM_RUNTIME 7
#define EEPROM_BLOCK_TYPE 8         // default action
#define EEPROM_VALUE 9
#define EEPROM_SIGGEN 9
#define EEPROM_PARAM4 9

#define STATE_RUNTIME 3
#define STATE_TTR 0
#define STATE_SIGGEN 4
#define STATE_VALUE 5
// Or they could just be parameters
#define STATE_PARAM1 1
#define STATE_PARAM2 2
#define STATE_PARAM3 3
#define STATE_PARAM4 4

// Action Definitions
// generic
#define ACTION_NONE 255     // EEPROM default (unwritten) value

// Pin types
// generic
#define PIN_NOTUSED 255     // the default (unwritten) value from the EEPROM

#define DEVICE_DIGITAL_OUTPUT 1    // single pin1 digital output (same as below)
#define DEVICE_PWM_OUTPUT 2       // single pin1 PWM output, param4 (value) is signal generator,
                                   // param1 is operator to apply to signal value (see below)
                                  // param2 is operand, RUNTIME is TTR,
                                  // (all can be remote modified)
 /* Also note that an RGB LED is treated as 3 separate PWM LEDs, one for each pin / colour */

#define COPY_VALUE      0
#define LESS_THAN       1
#define GREATER_THAN    2
#define EQUAL_TO        3
#define NOT_EQUAL       4
#define GREATER_EQUAL   5
#define LESS_EQUAL      6
#define MOD_EQUAL       7
#define NOT_MOD_EQUAL   8
#define MULTIPLY        9
#define DIVIDE          10
#define PLUS            11
#define MINUS           12
#define VALUES_ABOVE    13
#define VALUES_BELOW    14
#define INVERSE         15

#define ACTION_FLICKER 8
// EEPROM param 0 high value, param1 low value, param2 random max value


#define ACTION_WAVEFORM 9  // produce one waveform per TTR (>= 1s)
                           // there will be 50 samples per cycle
                           // values from 0 to 255
#define WAVE_SAWTOOTH_UP 0 // param0 is waveform type, as shown
#define WAVE_SAWTOOTH_DN 1
#define WAVE_TRIANGULAR 2
#define WAVE_SINE 3
// EEPROM param1 is proportion per step, param3 is TTR each step, param4 initial value?
// STATE param2 direction 1 = up, 0 = down, param1 step size, param3 = tickcount

#define ACTION_ON_OFF_TIMER 10
// EEPROM param1 ontime param2 offtime

#define ACTION_SWITCH 14 // assumed that value is set remotely, but default value is used

#define DIRECTION_UP 1
#define DIRECTION_DN 0

#define ACTION_COUNT 15 // change value on every TTR
#define COUNT_UP 0 // param0 is waveform type, as shown
#define COUNT_DN 1
#define COUNT_BOTH 2
// EEPROM param1 is low value, param2 high, param3 is TTR, param4 step value
// STATE param1 direction 1 = up, 0 = down, param2 various

#define ACTION_RND_DIGITAL 16
// TTR is used a max random value (use big no, small units)

#define ACTION_RND_ANALOG 17
// TTR is max random value (use big no, small units)
// range is 0 to 255


#define DEVICE_DIGITAL_SR  24       // digital output shift register
 /*
   EEPROM_PIN1 - Data input pin
   EEPROM_PIN2 - Clock pin
   EEPROM_PIN3 - Latch pin
   EEPROM_VALUE - Number of shift registers in total
   (RUNTIME not used)

  STATE_PARAM1 value of 1st register
  STATE_PARAM2 value of 2nd register
  STATE_PARAM3 value of 3rd register
  (STATE_VALUE could be used if there are 4 SR's)
*/

#define DEVICE_PWM_SR  25       // pwm output shift register

#define DEVICE_SHIFT_REG 30

 /*
   EEPROM_PIN1 - Data input pin
   EEPROM_PIN2 - Clock pin
   EEPROM_PIN3 - Latch pin
   EEPROM_VALUE - Number of shift registers in total
   (RUNTIME not used)

  STATE_PARAM1 current value of SR1
  STATE_PARAM2 current value of SR2
  STATE_PARAM3 current value of SR3
  (STATE_VALUE could be used if there are 4 SR's)
*/


#define DEVICE_RTC_VIRTUAL 40
#define DEVICE_RTC_I2C 41

#define ACTION_WAIT 50
#define DEVICE_DHT11 42

#define ACTION_UPDATE_LCD 44



/* Layout of LCD block
+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         | DEVICE_RGB_LED             | EEPROM_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | Device subtype      |                            | EEPROM_SUBTYPE           |
+--------+---------------------+----------------------------+--------------------------+
| 2      | Device tag          |                            | EEPROM_TAG               |
+--------+                     |                            |                          |
| 3      |                     |                            |                          |
+--------+                     |                            |                          |
| 4      |                     |                            |                          |
+--------+                     |                            |                          |
| 5      |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 6      | Default action      |                            | EEPROM_BLOCK_TYPE            |
+--------+---------------------+----------------------------+--------------------------+
| 7      | Red Pin             | Populate with WEP          | EEPROM_PIN1              |
+--------+---------------------+----------------------------+--------------------------+
| 8      | Green Pin           | Populate with WEP          | EEPROM_PIN2              |
+--------+---------------------+----------------------------+--------------------------+
| 9      | Blue Pin            | Populate with WEP          | EEPROM_PIN3              |
+--------+---------------------+----------------------------+--------------------------+

+--------+---------------------+----------------------------+
| Offset | Content             | Notes                      |
+--------+---------------------+----------------------------+
| 000    | Device type         |                            |
+--------+---------------------+----------------------------+
| 001    | Device subtype      |                            |
+--------+---------------------+----------------------------+
| 002    | Device tag          |                            |
+--------+                     |                            |
| 003    |                     |                            |
+--------+                     |                            |
| 004    |                     |                            |
+--------+                     |                            |
| 005    |                     |                            |
+--------+---------------------+----------------------------+
| 006    | Device action       |                            |
+--------+---------------------+----------------------------+
| 007    | Device value        |                            |
+--------+---------------------+----------------------------+
| 008    | Data pin 0          | Populate with WEP          |
+--------+---------------------+----------------------------+
| 009    | Data pin 1          | Populate with WEP          |
+--------+---------------------+----------------------------+
| 010    | Data pin 2          | Populate with WEP          |
+--------+---------------------+----------------------------+
| 011    | Data pin 3          | Populate with WEP          |
+--------+---------------------+----------------------------+
| 012    | RW Pin              | Populate with WEP          |
+--------+---------------------+----------------------------+
| 013    | RS Pin              | Populate with WEP          |
+--------+---------------------+----------------------------+
| 014    | (EN pin, if reqd)   | Populate with WEP          |
+--------+---------------------+----------------------------+
| 015    | (Not used)          | Populate with WEP          |
+--------+---------------------+----------------------------+

*/

// Suspend or activate a device (or signal generator) based on the output of a signal generator
#define CONTROLLER 50      // param0 is block to enable/disable, param4 (value) is signal generator,
                                   // param1 is operator to apply to signal value (see below)
                                  // param2 is operand, RUNTIME is TTR,
                                  // (all can be remote modified)


////////////////////////////////// INPUT Devices ///////////////////////////

#define ACTION_DIGITAL_INPUT 100        // Generic input device (read digital value from pin) uese TTR
                                        // param 1 is input process, as below, param2 is debounce time
#define INPUT_RAW 0                     // just put the reading into value
#define INPUT_TOGGLE 1                  // change state on button press

#define ACTION_ANALOG_INPUT 101        // Generic input device (read analog value from pin)
