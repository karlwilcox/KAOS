/*
 * This file defines the standard layout of device data
 * for all Arduino devices using KAOS (Karl's Arduino
 * Operting System)
 */

/*
  Memory is laid out as a series of 10 byte "permanent" data in EEPROM, along with
  corresponding 4 byte blocks of volatile data. The precise meaning of most of the
  data bytes is device dependant, as described below.
*/

// some useful macros
#define EEPROM_BLOCK_SIZE 10          // space set aside for device information
#define EEPROM_ADDRESS(a,b) (((a) * EEPROM_BLOCK_SIZE) + (b))
#define eepromRead(a,b) EEPROM.read(EEPROM_ADDRESS(a,b))
#define eepromWrite(a,b,c) EEPROM.write(EEPROM_ADDRESS(a,b),(c))

#define STATE_BLOCK_SIZE 4
#define STATE_ADDRESS(a,b) (((a) * STATE_BLOCK_SIZE) + (b))
#define stateRead(a,b) (deviceStates[STATE_ADDRESS(a,b)])
#define stateWrite(a,b,c) (deviceStates[STATE_ADDRESS(a,b)] = (c))
#define stateBitSet(a,b,c) (deviceStates[STATE_ADDRESS(a,b)] |= (c))
#define stateBitClear(a,b,c) (deviceStates[STATE_ADDRESS(a,b)] &= (~c))

//////////////////////// Block 0 //////////////////////////////////////////

// byte 0 is not used, (too easy to overwrite)
#define ADDRESS_UNIT_ID 1                // unique ID for each board, used for I2C comms
#define ADDRESS_UNIT_TAG 2           // Tag - Type of board
#define ADDRESS_FLAG 6
// 7 to 9 UNUSED

// tag values for UNITTYPE
#define TAG_UNO3 "UNO3"          // The original (big) Arduino
#define TAG_NANO "NANO"          // Little brother, with more pins
#define TAG_MEGA "MEGA"          // Biggest one, lots of pins

/////////////////////// Layout of remaining blocks /////////////////////////

/* Layout of eeprom block 0 - device block

+--------+---------------------+----------------------------+--------------------------+
| Address| Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
| 0000   | Not used            | (too easy to overwrite)    |                          |
+--------+---------------------+----------------------------+--------------------------+
| 0001   | Board  ID           | used as the I2C address    | ADDRESS_UNIT_ID          |
+--------+---------------------+----------------------------+--------------------------+
| 0002   | Board type          | one of:                    | ADDRESS_UNIT_TAG         |
+--------+                     | UNO3, NANO, MEGA           |                          |
| 0003   |                     |                            |                          |
+--------+                     |                            |                          |
| 0004   |                     |                            |                          |
+--------+                     |                            |                          |
| 0005   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 0006   | Default flag value  | See config.h for mapping   | ADDRESS_FLAG             |
+--------+---------------------+----------------------------+--------------------------+
| 0007   | Unused              |                            |                          |
| to     |                     |                            |                          |
| 0009   |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+

  Layout of state block 0 - device block

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0    | Flag 1              | See config.h               | STATE_FLAG               |
+--------+---------------------+----------------------------+--------------------------+
|   1    |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   2    |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
|   3    |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+

*/

#define DEVICE_BOARD 0 // location of board info, block 0

// Device definitions
#define OFFSET_TYPE 0           // offsets for fixed information
#define OFFSET_NEXT_DEVICE 1      // If in a sequence
#define OFFSET_TAG 2            // bytes 2-5 are the 4 character tag
// bytes 6 to 9 are device dependent, but there are some common ones
#define OFFSET_ACTION 6         // default action
#define OFFSET_PIN1 7            // Use WEP to populate this
#define OFFSET_PIN2 8            // Use WEP to populate this
#define OFFSET_PIN3 9             // Use WEP to populate this
// OFFSET 10 is the coktinuation byte
#define OFFSET_PIN4 11            // Use WEP to populate this
#define OFFSET_PIN5 12            // Use WEP to populate this
#define OFFSET_PIN6 13            // Use WEP to populate this

// Values for the device type field
// - special values first
#define DEVICE_END 255          // Marks the end of the list of devices (default, unwritten EEPROM)
#define DEVICE_CONT 254         // This device needs a further 10 bytes of data
#define DEVICE_DELETED 253      // Marks a deleted device, ignore this
#define DEVICE_UPPER 250        // Everything above this can be ignored

// Default Actions
// generic
#define DEVICE_OFF 255     // EEPROM default (unwritten) value
#define DEVICE_ON 1

// Pin types
// generic
#define NOTUSED 255     // the default (unwritten) value from the EEPROM
#define BAD_PIN 255

#define STATE_FLAG 0

////////////////////////////////// LED Devices ///////////////////////////
/* Layout of generic (also plain LED) block

+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         |                            | OFFSET_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | (next device)       | if part of a chain         | OFFSET_NEXT_DEVICE       |
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
| 7      | Device pin (*)      | Populate with WEP          | OFFSET_PIN1              |
+--------+---------------------+----------------------------+--------------------------+
| 8      | SR device block     | if pin > 100               | OFFSET_SR_BLOCK          |
+--------+---------------------+----------------------------+--------------------------+
| 9      | (spare)             |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
(*) Special values (virtual pins) are as follows:
101 - 132: pins 0 to 32 of shift register in block given in byte 8
And there are separate multiplexors for plain and PWM LED  
  
  Layout of state block for LEDs

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

#define STATE_VALUE 0
#define STATE_TTR   3
#define STATE_ACTION 2
#define TTR_UNIT_20ms 0
#define TTR_UNIT_100ms 1
#define TTR_UNIT_1s 2
#define TTR_UNIT_10s 3

// -- normal LEDs
#define DEVICE_OUTPUT 0            // generic output device
#define DEVICE_LED 1            // generic LED
#define DEVICE_PWM_LED 2         // LED on a PWM output

// LED Action values
#define ACTION_NONE 255
#define ACTION_RUN 1
// values 2,3 not used
#define ACTION_FLSH1 4     // Flash 1s
#define ACTION_FLSH2 5     // Flash 0.5s
#define ACTION_FLSH3 6     // 0.5s on, 1s off
// values 7-9 not used
#define ACTION_RND2S 10     // Random 20% duty cycle short time (10s)
#define ACTION_RND5S 11     // Random 50% duty cycle short time
#define ACTION_RND8S 12    // Random 80% duty cycle short time
#define ACTION_RND2M 13     // Random 20% duty cycle medium time (60s)
#define ACTION_RND5M 14    // Random 50% duty cycle medium time
#define ACTION_RND8M 15    // Random 80% duty cycle medium time
#define ACTION_RND2L 16    // Random 20% duty cycle medium time (180s)
#define ACTION_RND5L 17    // Random 50% duty cycle medium time
#define ACTION_RND8L 18    // Random 80% duty cycle medium time
// value 19 not used
#define ACTION_FLCK1 20    // Flicker fast (<1s cycle time)
#define ACTION_FLCK2 21    // Flicker medium (2s cycle time)
#define ACTION_FLCK3 22    // Flicker long (5s cycle time)
// values 23 -24 not used
#define ACTION_SEQ_SLOW_HEAD 30 // Start of sequence (TTR as per next device)
#define ACTION_SEQ_SLOW 31 // sequence 2s
#define ACTION_SEQ_MED_HEAD  32 // sequence 1s
#define ACTION_SEQ_MED  33 // sequence 1s
#define ACTION_SEQ_FAST_HEAD 34 // sequence 0.5s
#define ACTION_SEQ_FAST 35 // sequence 0.5s



/* Layout of RGB LED block

+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         | DEVICE_RGB_LED             | OFFSET_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | (next device)       | if part of a chain         | OFFSET_NEXT_DEVICE       |
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
| 7      | Red Pin             | Populate with WEP          | OFFSET_RGBPINR           |
+--------+---------------------+----------------------------+--------------------------+
| 8      | Green Pin           | Populate with WEP          | OFFSET_RGBPING           |
+--------+---------------------+----------------------------+--------------------------+
| 9      | Blue Pin            | Populate with WEP          | OFFSET_RGBPINB           |
+--------+---------------------+----------------------------+--------------------------+

*/
// -- RGB LEDs
#define DEVICE_RGB_LED 2 // device type
#define OFFSET_RGBPINR OFFSET_PIN1   // offset of  red pin
#define OFFSET_RGBPING OFFSET_PIN2   // offset of  red pin
#define OFFSET_RGBPINB OFFSET_PIN3  // offset of  red pin

// values 19 to 23 not used

////////////////////////////////// Shift Registers ///////////////////////////
///// NOTE: Shift registers should be defined BEFORE the devices that use them
//////////////////////////////////////////////////////////////////////////////
/* Layout of shift register block
+--------+---------------------+----------------------------+--------------------------+
| Offset | Content             | Notes                      | Defined as               |
+--------+---------------------+----------------------------+--------------------------+
| 0      | Device type         | DEVICE_DIGITAL_SR          | OFFSET_TYPE              |
+--------+---------------------+----------------------------+--------------------------+
| 1      | (next device)       | if part of a chain         | OFFSET_NEXT_DEVICE       |
+--------+---------------------+----------------------------+--------------------------+
| 2      | Device tag          |                            | OFFSET_TAG               |
+--------+                     |                            |                          |
| 3      |                     |                            |                          |
+--------+                     |                            |                          |
| 4      |                     |                            |                          |
+--------+                     |                            |                          |
| 5      |                     |                            |                          |
+--------+---------------------+----------------------------+--------------------------+
| 6      | No. of devices      | (Up to 4 supported)        | OFFSET_SR_NUM            |
+--------+---------------------+----------------------------+--------------------------+
| 7      | Data input pin      | Populate with WEP          | OFFSET_SR_DATA_PIN       |
+--------+---------------------+----------------------------+--------------------------+
| 8      | Clock Pin           | Populate with WEP          | OFFSET_SR_CLOCK_PIN      |
+--------+---------------------+----------------------------+--------------------------+
| 9      | Latch Pin           | Populate with WEP          | OFFSET_SR_LATCH_PIN      |
+--------+---------------------+----------------------------+--------------------------+

SR's don't have a default action, or TTR, each indvidual connection has
  
  Layout of state block for SR's

+--------+---------------------+----------------------------+--------------------------+
| Byte   | Content             | Notes                      | Defined                  |        
+--------+---------------------+----------------------------+--------------------------+
|   0    | State of SR 1       |                            | STATE_SR1_MAP            |
+--------+---------------------+----------------------------+--------------------------+
|   1    | State of SR 2       |                            | STATE_SR2_MAP            |
+--------+---------------------+----------------------------+--------------------------+
|   2    | State of SR 3       |                            | STATE_SR3_MAP            |
+--------+---------------------+----------------------------+--------------------------+
|   3    | State of SR 4       |                            | STATE_SR4_MAP            |
+--------+---------------------+----------------------------+--------------------------+
*/

#define DEVICE_DIGITAL_SR  24       // digital output shift register
#define DEVICE_PWM_SR  25       // pwm output shift register

#define OFFSET_SR_DATA_PIN      OFFSET_PIN1
#define OFFSET_SR_CLOCK_PIN     OFFSET_PIN2
#define OFFSET_SR_LATCH_PIN     OFFSET_PIN3
#define OFFSET_SR_BLOCK 8
#define OFFSET_SR_NUM 6
#define STATE_SR1_MAP 0
#define STATE_SR2_MAP 1
#define STATE_SR3_MAP 2
#define STATE_SR4_MAP 3

// values 26-31 not used
// -- Motors (to be done)
// 32 - 49
// -- Relays (to be done)
// 50 - 59

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

// LCD screen subtypes
#define LCD16x2 1



////////////////////////////////// INPUT Devices ///////////////////////////

#define DEVICE_INPUT 100        // Generic input device (read digital value from pin)
#define DEVICE_ANALOG 101        // Generic input device (read analog value from pin)
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
#define ACTION_SMP100MS 100   // sample every 100ms
#define ACTION_SMP500MS 101   // sample every 500ms
#define ACTION_SMP1S 102   // sample every 1s
#define ACTION_SMP2S 103   // sample every 2s
#define ACTION_SMP5S 104   // sample every 5s
#define ACTION_SMP10S 105   // sample every 10 seconds
#define ACTION_SMP1M 106   // sample every 1 minute

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

#define ACTION_RTC_VIRTUAL 110

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
| 9      | Unused              |                            |                          |
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
