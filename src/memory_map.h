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
#define stateCopy(a,b,c) (stateWrite((a),(b),eepromRead((a),(c))))

//////////////////////// Block 0 //////////////////////////////////////////

#define EEPROM_UNIT_ID 4                // unique ID for each board, used for I2C comms
#define EEPROM_FLAG 5
#define EEPROM_TAG 0
#define STATE_FLAG 5
#define BAD_PIN 255
// 7 to 9 UNUSED

// tag values for UNITTYPE
#define TAG_UNO3 "UNO3"          // The original (big) Arduino
#define TAG_NANO "NANO"          // Little brother, with more pins
#define TAG_MEGA "MEGA"          // Biggest one, lots of pins

/////////////////////// Layout of Block 0 (the board) /////////////////////////

#define DEVICE_BOARD 0 // location of board info, block 0

/* Layout of eeprom block 0 - board block

+--------+---------------------+----------------------------+--------------------------+
| Address| Content             | Notes                      | Defined                  |
+--------+---------------------+----------------------------+--------------------------+
| 0000   | Board type          | one of:                    | ADDRESS_UNIT_TAG         |
+--------+                     | UNO3, NANO, MEGA           |                          |
| 0001   |                     |                            |                          |
+--------+                     |                            |                          |
| 0002   |                     |                            |                          |
+--------+                     |                            |                          |
| 0003   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 0004   | Board  ID           | used as the I2C address    | ADDRESS_UNIT_ID          |
+--------+---------------------+----------------------------+--------------------------+
| 0005   | Default flag value  | See config.h for mapping   | ADDRESS_FLAG             |
+--------+---------------------+----------------------------+--------------------------+
| 0006   | Unused              |                            |                          |
| to     |                     |                            |                          |
| 0009   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+

  Layout of state block 0 - device block

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0    |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   1    |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   2    | Block of 1st SR     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   3    | Block of DHT11      |                            |  STATE_DHT_BLOCK         |
+--------+---------------------+----------------------------+--------------------------+
|   4    | Block of RTC        |                            |  STATE_RTC_BLOCK         |            
+--------+---------------------+----------------------------+--------------------------+
|   5    |Current flag value   |                            | STATE_FLAG               |
+--------+---------------------+----------------------------+--------------------------+

*/

#define ADDRESS_UNIT_TAG 0
#define ADDRESS_UNIT_ID 4

#define OFFSET_BLOCK_TYPE 0
#define STATE_DHT_BLOCK 3 
#define STATE_RTC_BLOCK 4 
#define STATE_1ST_SR 2

/* 
  Special values for offset 0:
  - If a device is present at this block, OFFSET_BLOCK_TYPE will contain a printable character
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
#define TTR_UNIT_100ms 1   // ends up as 0b01000000   (64)
#define TTR_UNIT_1s 2      // ends up as 0b10000000  (128)
#define TTR_UNIT_10s 3     // ends up as 0b11000000  (192)


/* Layout of eeprom generic block

+--------+---------------------+----------------------------+--------------------------+
| Address| Content             | Notes                      | Defined                  |
+--------+---------------------+----------------------------+--------------------------+
| 0000   | tag (or marker)     | Printable characters only  | OFFSET_TAG               |
+--------+                     |                            |                          |
| 0001   |                     |                            |                          |
+--------+                     |                            |                          |
| 0002   |                     |                            |                          |
+--------+                     |                            |                          |
| 0003   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 0004   | Pin 1 / Param 0     |                            | OFFSET_PIN1              |
|        |                     |                            | OFFSET_PARAM0            |
+--------+---------------------+----------------------------+--------------------------+
| 0005   | Pin 2 / Param 1     |                            | OFFSET_PIN2              |
|        |                     |                            | OFFSET_PARAM1            |
+--------+---------------------+----------------------------+--------------------------+
| 0006   | Pin 3 / Param 2     |                            | OFFSET_PIN3              |
|        |                     |                            | OFFSET_PARAM2            |
+--------+---------------------+----------------------------+--------------------------+
| 0007   | Pin 4 / Param 3     |                            | OFFSET_PIN4              |
|        |                     |                            | OFFSET_PARAM3            |
|        | Value               |                            | OFFSET_VALUE             |
+--------+---------------------+----------------------------+--------------------------+
| 0008   | Action              |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 0009   | Default Runtime     | (Special format, below)    | OFFSET_RUNTIME           |
+--------+---------------------+----------------------------+--------------------------+

  Layout of generic device block

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0    | Parameter 1         |                            | STATE_PARAM1             |
+--------+---------------------+----------------------------+--------------------------+
|   1    | Parameter 2         |                            | STATE_PARAM2             |
+--------+---------------------+----------------------------+--------------------------+
|   2    | Parameter 3         |                            | STATE_PARAM3             |
|        | Target TTR          |                            | STATE_RUNTIME            |
+--------+---------------------+----------------------------+--------------------------+
|   3    | Current action      |                            | STATE_ACTION             |
+--------+---------------------+----------------------------+--------------------------+
|   4    | Current TTR         | Auto-decremented           | STATE_TTR                |
+--------+---------------------+----------------------------+--------------------------+
|   5    |Current value        |                            | STATE_VALUE              |
+--------+---------------------+----------------------------+--------------------------+

*/

// Offsets or pins
#define EEPROM_PIN1 4            // Use WEP to populate this
#define EEPROM_PIN2 5            // Use WEP to populate this
#define EEPROM_PIN3 6             // Use WEP to populate this
#define EEPROM_PIN4 7             // Use WEP to populate this

// Offsets for parameters
#define EEPROM_PARAM1 4
#define EEPROM_PARAM2 5
#define EEPROM_PARAM3 6
#define EEPROM_PARAM4 7
#define EEPROM_VALUE 7

// Offsets for action / ttr
#define EEPROM_ACTION 8         // default action
#define EEPROM_RUNTIME 9      // If in a sequence
#define STATE_TTR 4
#define STATE_RUNTIME 2
#define STATE_ACTION 3
#define STATE_VALUE 5
// Or they could just be parameters
#define STATE_PARAM1 0
#define STATE_PARAM2 1
#define STATE_PARAM3 2

// Action Definitions
// generic
#define ACTION_NONE 255     // EEPROM default (unwritten) value

// Pin types
// generic
#define PIN_NOTUSED 255     // the default (unwritten) value from the EEPROM

#define ACTION_DIGITAL_OUTPUT 0     // e.g. LED, initial value given in EEPROM_VALUE
                              // value remains fixed unless remote controlled

 /*

   ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
 
  //////////////// Initial actions ///////////////////////

  STATE_VALUE is set to 0, Pin 1 set LOW / HIGH

*/

////////////////////////////////// Actions Output Devices ///////////////////////////

 
#define ACTION_PWM_OUTPUT 2   // PWM device, initial value is in EEPROM_VALUE
                              // value remains fixed unless remote controlled
 
 /*

   ///////////// Parameter value meanings ////////////////

  OFFSET_PIN1 - the digital pin in question
  OFFSET_VALUE - 0 - 255 PWM level
 
  //////////////// Initial actions ///////////////////////

  STATE_VALUE is set to OFFSET_VALUE, PIN1 set to value

  ////////////////////// Notes //////////////////////////

  Also note that an RGB LED is treated as 3 separate PWM LEDs, one for each pin / colour

*/

#define ACTION_FLASH_LED 10

 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
   OFFSET_PARAM1 - Runtime for HIGH
   OFFSET_PARAM2 - Runtime for LOW
 
  //////////////// Initial actions ///////////////////////

  STATE_VALUE is set to 1, PIN1 set to 1
  STATE_TTR set to OFFSET_PARAM1

  ////////////////// Action on Timeout //////////////////

  If value is 1, set value 0, PIN1 set to 0, TTR is PARAM2
  If value is 0, set value 1, PIN1 set to 1, TTR is PARAM1

  ////////////////////// Notes //////////////////////////

*/


// value 4 not used
#define ACTION_PULSE_UP 11
#define ACTION_PULSE_DOWN 12

 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
   OFFSET_PARAM1 - High value
   OFFSET_PARAM2 - Low value
   OFFSET_RUNTIME - rise / fall time (so period is twice this)
 
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


#define ACTION_FADE_PWM 7

 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
   OFFSET_PARAM1 - Start value
   OFFSET_PARAM2 - End value
   OFFSET_RUNTIME - rise / fall time
 
  //////////////// Initial actions ///////////////////////

  STATE_VALUE is set to Low value, PIN1 set to low value
  STATE_TTR set to Param 2
  Params 1 & 2 copied from EEPROM to state

  ////////////////// Action on Timeout //////////////////

  Value is incremented / decremented by (high - low) / runtime steps
  on limit, set STATE_TTR to 0

  ////////////////////// Notes //////////////////////////

*/

#define ACTION_FLICKER_PWM 8

 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
   OFFSET_PARAM1 - High value
   OFFSET_PARAM2 - Low value
   OFFSET_RUNTIME - update rate
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/


#define ACTION_SEQ_HEAD 20
#define ACTION_SEQ 21


 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
   OFFSET_PARAM1 - Block number of the next device
   OFFSET_RUNTIME - Time to stay on this device
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/


#define ACTION_SEQ_PWM_HEAD 11
#define ACTION_SEQ_PWM 12


 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
   OFFSET_PARAM1 - Block number of the next device
   OFFSET_PARAM2 - PWM level for this device
   OFFSET_RUNTIME - Time to stay on this device
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/



#define ACTION_RANDOM 13


 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - the digital pin in question
   OFFSET_PARAM1 - Block number of the next device
   OFFSET_PARAM2 - PWM level for this device
   OFFSET_RUNTIME - Time to stay on this device
 
  //////////////// Initial actions ///////////////////////


  ////////////////// Action on Timeout //////////////////


  ////////////////////// Notes //////////////////////////

*/

#define ACTION_SHIFT_REG 30


 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - Data input pin
   OFFSET_PIN2 - Clock pin
   OFFSET_PIN3 - Latch pin
   OFFSET_VALUE - Number of shift registers in total
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

 /*

  ///////////// Parameter value meanings ////////////////

   OFFSET_PIN1 - Data input pin
   OFFSET_PIN2 - Clock pin
   OFFSET_PIN3 - Latch pin
   OFFSET_VALUE - Number of shift registers in total
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

// values 19 to 23 not used


#define DEVICE_DIGITAL_SR  24       // digital output shift register
#define DEVICE_PWM_SR  25       // pwm output shift register

#define OFFSET_SR_DATA_PIN      OFFSET_PIN1
#define OFFSET_SR_CLOCK_PIN     OFFSET_PIN2
#define OFFSET_SR_LATCH_PIN     OFFSET_PIN3

// values 27-31 not used
// -- Motors (to be done)
// 32 - 49
// -- Relays (to be done)
// 50 - 59

/* Layout of controller block - the controller block can be used to randomly enable or
   disable other devices (and itself be controlled through its own action)

+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         |                            | OFFSET_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | Device under control|                            | OFFSET_CTRL_DEVICE       |
+--------+---------------------+----------------------------+--------------------------+
| 2      | Device tag          |                            | OFFSET_TAG               |
+--------+                     |                            |                          |
| 3      |                     |                            |                          |
+--------+                     |                            |                          |
| 4      |                     |                            |                          |
+--------+                     |                            |                          |
| 5      |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 6      | Action for device   |                            | OFFSET_CTRL_ACTION       |
+--------+---------------------+----------------------------+--------------------------+
| 7      | Min switch time     | x 10 seconds               | OFFSET_PIN1              |
+--------+---------------------+----------------------------+--------------------------+
| 8      | Max switch time     | x 10 seconds               | OFFSET_SR_BLOCK          |
+--------+---------------------+----------------------------+--------------------------+
| 9      | TTR                 | for sme actions            | OFFSET_TTR               |
+--------+---------------------+----------------------------+--------------------------+
  
  Layout of state block for Controllers

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0    | Current value       |                            | STATE_VALUE              |
+--------+---------------------+----------------------------+--------------------------+
|   1    |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   2    | Current action      | (see below)                | STATE_ACTION             |
+--------+---------------------+----------------------------+--------------------------+
|   3    | Time to run         | next state change (or 0)   | STATE_TTR                |
+--------+---------------------+----------------------------+--------------------------+

*/

#define DEVICE_CONTROLLER 26
#define OFFSET_CTRL_DEVICE 1
#define OFFSET_CTRL_ACTION 6
#define OFFSET_CTRL_MIN_TIME 7
#define OFFSET_CTRL_MAX_TIME 8
#define ACTION_CTRL_SET_ACTION 40

#define DEVICE_LCD 60          // LCD Screen
// values 67 - 99 not used

/* Layout of LCD block
+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         | DEVICE_RGB_LED             | OFFSET_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | Device subtype      |                            | OFFSET_SUBTYPE           |
+--------+---------------------+----------------------------+--------------------------+
| 2      | Device tag          |                            | OFFSET_TAG               |
+--------+                     |                            |                          |
| 3      |                     |                            |                          |
+--------+                     |                            |                          |
| 4      |                     |                            |                          |
+--------+                     |                            |                          |
| 5      |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 6      | Default action      |                            | OFFSET_ACTION            |
+--------+---------------------+----------------------------+--------------------------+
| 7      | Red Pin             | Populate with WEP          | OFFSET_PIN1              |
+--------+---------------------+----------------------------+--------------------------+
| 8      | Green Pin           | Populate with WEP          | OFFSET_PIN2              |
+--------+---------------------+----------------------------+--------------------------+
| 9      | Blue Pin            | Populate with WEP          | OFFSET_PIN3              |
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
// LCD Screen
#define OFFSET_LCDRS OFFSET_PIN1        // LCD Screen data lines
#define OFFSET_LCDEN OFFSET_PIN2
#define OFFSET_LCDD4 OFFSET_PIN3
#define OFFSET_LCDD5 OFFSET_PIN4
#define OFFSET_LCDD6 OFFSET_PIN5
#define OFFSET_LCDD7 OFFSET_PIN6
#define OFFSET_LCDRW OFFSET_PIN7
#define ACTION_UPDATE_LCD 105

// LCD screen subtypes
#define LCD16x2 1



////////////////////////////////// INPUT Devices ///////////////////////////

#define ACTION_DIGITAL_INPUT 100        // Generic input device (read digital value from pin)
#define ACTION_ANALOG_INPUT 101        // Generic input device (read analog value from pin)
#define DEVICE_DHT11  110      // Temperature and humidity sensor - uses default layout

/* Layout of generic input block
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
| 006    | Default action      |                            |
+--------+---------------------+----------------------------+
| 007    | Device value        |                            |
+--------+---------------------+----------------------------+
| 008    | Device pin (*)      | Populate with WEP          |
+--------+---------------------+----------------------------+
| 009    | Unused              |                            |
+--------+---------------------+----------------------------+
(*) Special values (virtual pins) are as follows:
100 - 147: pins 0 to 47 of the first multiplexor
150 - 197: pins 0 to 47 of the second multiplexor
?? 200 - 247: pins 0 to 47 of the third multiplexor?

And there are separate multiplexors for plain and PWM LEDs
*/

// inputs
#define DEVICE_BUTTON  71      // Momentary button
#define DEVICE_SWITCH  72      // on/off switch
#define DEVICE_GENPOT  73      // Generic potentiometer
#define DEVICE_IRRECV  74      // IR Receiver
#define DEVICE_RTC     75      // Real-time clock
#define DEVICE_SDCARD  76      // SD card


// Input actions
#define ACTION_SAMPLE 106

/* Layout of RTC EEPROM block

+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         | DEVICE_RTC                 | OFFSET_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | Device subtype      |                            | OFFSET_SUBTYPE           |
+--------+---------------------+----------------------------+--------------------------+
| 2      | Device tag          |                            | OFFSET_TAG               |
+--------+                     |                            |                          |
| 3      |                     |                            |                          |
+--------+                     |                            |                          |
| 4      |                     |                            |                          |
+--------+                     |                            |                          |
| 5      |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 6      | Default action      |                            | OFFSET_ACTION            |
+--------+---------------------+----------------------------+--------------------------+
| 7      | I2C Address         |                            | OFFSET_I2C_ADDR          |
+--------+---------------------+----------------------------+--------------------------+
| 8      | Unused              |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 9      | Unused              |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
  
  Layout of state block for RTC

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0    | Hour value (0-23)   |                            | STATE_RTC_HOURS          |
+--------+---------------------+----------------------------+--------------------------+
|   1    | Minute value (0-59) |                            | STATE_RTC_MINS           |
+--------+---------------------+----------------------------+--------------------------+
|   2    | Seconds value (0-59)|                            | STATE_RTC_SECS           |
+--------+---------------------+----------------------------+--------------------------+
|   3    | Time to run         | next state change (or 0)   | STATE_TTR                |
+--------+---------------------+----------------------------+--------------------------+
*/

#define OFFSET_I2C_ADDR  7
#define STATE_RTC_HOURS 0
#define STATE_RTC_MINS 1
#define STATE_RTC_SECS 2

/* Layout of DHT11 EEPROM block

+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         | DEVICE_DHT11               | OFFSET_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | Device subtype      |                            | OFFSET_SUBTYPE           |
+--------+---------------------+----------------------------+--------------------------+
| 2      | Device tag          |                            | OFFSET_TAG               |
+--------+                     |                            |                          |
| 3      |                     |                            |                          |
+--------+                     |                            |                          |
| 4      |                     |                            |                          |
+--------+                     |                            |                          |
| 5      |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 6      | Default action      |                            | OFFSET_ACTION            |
+--------+---------------------+----------------------------+--------------------------+
| 7      | Unused              |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 8      | Unused              |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 9      | (default TTR)       | for some actions           | OFFSET_TTR               |
+--------+---------------------+----------------------------+--------------------------+
  
  Layout of state block for DHT11

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0    | Temperature         |                            | STATE_DHT_TMPT           |
+--------+---------------------+----------------------------+--------------------------+
|   1    | Humidity            |                            | STATE_DHT_HMDT           |
+--------+---------------------+----------------------------+--------------------------+
|   2    | Current action      | (see below)                | STATE_ACTION             |
+--------+---------------------+----------------------------+--------------------------+
|   3    | Time to run         | next state change (or 0)   | STATE_TTR                |
+--------+---------------------+----------------------------+--------------------------+
*/

#define STATE_DHT_TMPT 0
#define STATE_DHT_HMDT 1
#define ACTION_UPDATE_DHT 111
