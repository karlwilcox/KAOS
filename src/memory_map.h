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
#define eepromWrite(a,b,c) EEPROM.write(EEPROM_ADDRESS(a,b),(c))

#define STATE_ADDRESS(a,b) (((a) * STATE_BLOCK_SIZE) + (b))
#define stateRead(a,b) (deviceStates[STATE_ADDRESS(a,b)])
#define stateWrite(a,b,c) (deviceStates[STATE_ADDRESS(a,b)] = (c))
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

#define DEVICE_END 255          // Marks the end of the list of devices (default, unwritten EEPROM)
#define DEVICE_CONT 254         // This device needs a further 10 bytes of data
#define DEVICE_DELETED 253      // Marks a deleted device, ignore this
#define DEVICE_SUSPENDED 252
#define DEVICE_UPPER 250        // Everything above this can be ignored

/*
 Definitions of units for TTR (top 2 bits)
*/

#define TTR_UNIT_20ms 0    // ends up as 0b00000000    (0)
#define TTR_UNIT_100ms 64   // ends up as 0b01000000   (64)
#define TTR_UNIT_1s 128      // ends up as 0b10000000  (128)
#define TTR_UNIT_10s 192     // ends up as 0b11000000  (192)


/* Layout of eeprom block

+--------+---------------------+----------------------------+--------------------------+
| Address| Content             | Notes                      | Defined                  |
+--------+---------------------+----------------------------+--------------------------+
| 0000   | tag (or marker)     | Printable characters only  | EEPROM_TAG               |
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
+--------+---------------------+----------------------------+--------------------------+
| 0008   | Action              |                            | EEPROM_ACTION            |
+--------+---------------------+----------------------------+--------------------------+
| 0009   | default value       |                            | EEPROM_VALUE             |
| 0009   | Param 4             |                            | EEPROM_PARAM4            |
+--------+---------------------+----------------------------+--------------------------+

  Layout of state block (BY default, fields 5 to 9 above are copied to 1 to 5 below)

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
|   4 E  | Current action      |                            | STATE_ACTION             |
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
#define EEPROM_ACTION 8         // default action
#define EEPROM_VALUE 9
#define EEPROM_PARAM4 9

#define STATE_RUNTIME 3
#define STATE_TTR 0
#define STATE_ACTION 4
#define STATE_VALUE 5
// Or they could just be parameters
#define STATE_PARAM1 1
#define STATE_PARAM2 2
#define STATE_PARAM3 3

// Action Definitions
// generic
#define ACTION_NONE 255     // EEPROM default (unwritten) value

// Pin types
// generic
#define PIN_NOTUSED 255     // the default (unwritten) value from the EEPROM

#define ACTION_DIGITAL_OUTPUT 1     // e.g. LED, initial value given in EEPROM_VALUE
                                    // value remains fixed unless remote controlled

 /*
   ///////////// Parameter value meanings ////////////////
   EEPROM_PIN1 - the digital pin in question
   EEPROM_VALUE - the inital value set
  //////////////// Initial actions ////////////////////////
  STATE_VALUE is set to 0, Pin 1 set LOW / HIGH
*/

////////////////////////////////// Actions Output Devices ///////////////////////////

#define ACTION_PWM_OUTPUT 2   // PWM device, initial value is in EEPROM_VALUE
                              // value remains fixed unless remote controlled
 
 /*
   ///////////// Parameter value meanings ////////////////
  EEPROM_PIN1 - the digital pin in question
  EEPROM_VALUE - 0 - 255 PWM level
  //////////////// Initial actions ///////////////////////
  STATE_VALUE is set to EEPROM_VALUE, PIN1 set to value
  ////////////////////// Notes //////////////////////////
  Also note that an RGB LED is treated as 3 separate PWM LEDs, one for each pin / colour
*/

#define ACTION_FLASH_LED 10

 /*
  ///////////// Parameter value meanings ////////////////
   EEPROM_PIN1 - the digital pin in question
   EEPROM_PARAM1 - Runtime for HIGH
   EEPROM_PARAM2 - Runtime for LOW
  //////////////// Initial actions ///////////////////////
  STATE_VALUE is set to 1, PIN1 set to 1
  STATE_TTR set to EEPROM_PARAM1
  ////////////////// Action on Timeout //////////////////
  If value is 1, set value 0, PIN1 set to 0, TTR is PARAM2
  If value is 0, set value 1, PIN1 set to 1, TTR is PARAM1
*/

#define ACTION_FADE_ONCE 11
#define ACTION_FADE_TWICE 12
#define ACTION_FADE_CYCLE 13
 /*
  ///////////// Parameter value meanings ////////////////
   EEPROM_PIN1 - the digital pin in question
   EEPROM_PARAM1 - start / end values
   EEPROM_PARAM2 - increment (see below)
   EEPROM_PARAM3 - TTR
  //////////////// Initial actions ///////////////////////
  STATE_VALUE is set to Low value, PIN1 set to low value
  STATE_TTR set to Param 2
  Params 1 & 2 copied from EEPROM to state
  Action is modified to show direction
  ////////////////// Action on Timeout //////////////////
  Value is incremented / decremented by (high - low) / runtime steps
  on limits, reverse direction.
  ////////////////////// Notes //////////////////////////
*/


#define ACTION_FLICKER 8

 /*

  ///////////// Parameter value meanings ////////////////

   EEPROM_PIN1 - the digital pin in question
   EEPROM_PARAM1 - high/low range
   EEPROM_PARAM2 - Maximum change (use big no, small unit)
   EEPROM_PARAM3 - (direction counter)
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/


#define ACTION_SEQ_HEAD 20
#define ACTION_SEQ 21


 /*

  ///////////// Parameter value meanings ////////////////

   EEPROM_PIN1 - the digital pin in question
   EEPROM_PARAM1 - Block number of the next device
   EEPROM_RUNTIME - Time to stay on this device
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/


#define ACTION_SEQ_PWM_HEAD 22
#define ACTION_SEQ_PWM 23


 /*

  ///////////// Parameter value meanings ////////////////

   EEPROM_PIN1 - the digital pin in question
   EEPROM_PARAM1 - Block number of the next device
   EEPROM_PARAM2 - PWM level for this device
   EEPROM_RUNTIME - Time to stay on this device
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/



#define ACTION_CTRL_RND 35
#define ACTION_CTRL_ON 36
#define ACTION_CTRL_OFF 37

 /*
  ///////////// Parameter value meanings ////////////////
   EEPROM_PARAM0 - first affected device 
   EEPROM_PARAM1 - second affected device (or 0 or 255)
   EEPROM_PARAM2 - third affected device (or 0 or 255)
   EEPROM_PARAM3 - fourth affected device (or 0 or 255)
   EEPROM_VALUE - 4 bits of off time (x10s), 4 bits on time (x1s) 
  //////////////// Initial actions ///////////////////////

  ////////////////// Action on Timeout //////////////////
*/

 /*

  ///////////// Parameter value meanings ////////////////

   EEPROM_PIN1 - the digital pin in question
   EEPROM_PARAM1 - Block number of the next device
   EEPROM_PARAM2 - PWM level for this device
   EEPROM_RUNTIME - Time to stay on this device
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/

#define ACTION_SHIFT_REG 30


 /*

  ///////////// Parameter value meanings ////////////////

   EEPROM_PIN1 - Data input pin
   EEPROM_PIN2 - Clock pin
   EEPROM_PIN3 - Latch pin
   EEPROM_VALUE - Number of shift registers in total
   (RUNTIME not used)
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// State Block usage //////////////////////////

  STATE_PARAM1 current value of SR1
  STATE_PARAM2 current value of SR2
  STATE_PARAM3 current value of SR3
  (STATE_VALUE could be used if there are 4 SR's)

*/


#define ACTION_RTC_VIRTUAL 40
#define ACTION_RTC_I2C 41



#define ACTION_WAIT 50

 /*

  ///////////// Parameter value meanings ////////////////

   EEPROM_PIN1 - Data input pin
   EEPROM_PIN2 - Clock pin
   EEPROM_PIN3 - Latch pin
   EEPROM_VALUE - Number of shift registers in total
   (RUNTIME not used)
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// State Block usage //////////////////////////

  STATE_PARAM1 current value of Hours
  STATE_PARAM2 current value of Minutes
  STATE_PARAM3 current value of Seconds
  (STATE_VALUE could be used if there are 4 SR's)

*/

////////////////////////////////// Shift Registers ///////////////////////////
///// NOTE: Shift registers should be defined BEFORE the devices that use them
//////////////////////////////////////////////////////////////////////////////

#define ACTION_DHT11 42

#define ACTION_UPDATE_LCD 44

// values 19 to 23 not used


#define DEVICE_DIGITAL_SR  24       // digital output shift register
#define DEVICE_PWM_SR  25       // pwm output shift register

#define EEPROM_SR_DATA_PIN      OFFSET_PIN1
#define EEPROM_SR_CLOCK_PIN     OFFSET_PIN2
#define EEPROM_SR_LATCH_PIN     OFFSET_PIN3



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
| 6      | Default action      |                            | EEPROM_ACTION            |
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



////////////////////////////////// INPUT Devices ///////////////////////////

#define ACTION_DIGITAL_INPUT 100        // Generic input device (read digital value from pin)
#define ACTION_ANALOG_INPUT 101        // Generic input device (read analog value from pin)
