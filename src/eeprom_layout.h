/*
 * This file defines the standard layout of EEPROM data
 * for all Arduino devices using KAOS (Karl's Arduino
 * Operting System)
 */

/*
 * The EEPROM is laid out as 16 byte blocks, each block describes one device, identified
 * by a 4 character, case-sensitive tag. Block 0 is special, and describes the device itself
 */

//////////////////////// Block 0 //////////////////////////////////////////

// byte 0 is not used, (too easy to overwrite)
#define UNITID 1                // unique ID for each board, used for I2C comms
#define UNITTYPE 0x02           // Tag - Type of board
#define DEFAULT_FLAG 0x06
// 0x07 to 0x0F UNUSED

// tag values for UNITTYPE
#define TAG_UNO3 "UNO3"          // The original (big) Arduino
#define TAG_NANO "NANO"          // Little brother, with more pins
#define TAG_MEGA "MEGA"          // Biggest one, lots of pins

/////////////////////// Layout of remaining blocks /////////////////////////

/* Layout of block 0 - device block

+--------+---------------------+----------------------------+
| Offset | Content             | Notes                      |
+--------+---------------------+----------------------------+
| 000    | Not used            | (too easy to overwrite)    |
+--------+---------------------+----------------------------+
| 001    | Board  ID           | used as the I2C address    |
+--------+---------------------+----------------------------+
| 002    | Board type          | one of:                    |
+--------+                     | UNO3, NANO, MEGA           |
| 003    |                     |                            |
+--------+                     |                            |
| 004    |                     |                            |
+--------+                     |                            |
| 005    |                     |                            |
+--------+---------------------+----------------------------+
| 006    | Default flag value  | See config.h for mapping   |
+--------+---------------------+----------------------------+
| 007    | Unused              |                            |
| to     |                     |                            |
| 015    |                     |                            |
+--------+---------------------+----------------------------+

*/

// Device definitions
#define DEVICE_SIZE 16          // space set aside for device information
#define DEVICE_TYPE 0           // offsets for fixed information
#define DEVICE_SUBTYPE 1        // e.g. colour of led?
#define DEVICE_TAG 2            // bytes 2-5 are the 4 character tag
// bytes 6 to 15 are device dependent, but there are some common ones
#define DEVICE_ACTION 6         // default action
#define DEVICE_VALUE 7          // default value
#define DEVICE_PIN 8            // Use WEP to populate this

// Values for the device type field
// - special values first
#define DEVICE_END 255          // Marks the end of the list of devices (default, unwritten EEPROM)
#define DEVICE_CONT 254         // This device needs a further 15 bytes of data
#define DEVICE_DELETED 253      // Marks a deleted device, ignore this
#define DEVICE_INPUT 252        // Generic digital input device
#define DEVICE_ANALOG 251       // Generic analog input device
#define DEVICE_OUTPUT 250       // Generic digital output
#define DEVICE_PWM 249          // Generic PWM output

// Default Actions
// generic
#define DEVICE_OFF 255     // EEPROM default (unwritten) value
#define DEVICE_ON 1

// Pin types
// generic
#define NOTUSED 255     // the default (unwritten) value from the EEPROM
#define BAD_PIN 255

////////////////////////////////// LED Devices ///////////////////////////
/* Layout of generic (also plain LED) block

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
| 008    | Device pin          | Populate with WEP          |
+--------+---------------------+----------------------------+
| 009    | Unused              |                            |
| to     |                     |                            |
| 015    |                     |                            |
+--------+---------------------+----------------------------+

*/


// -- normal LEDs
#define DEVICE_LED 0            // generic LED
#define DEVICE_PWMLED 1         // LED on a PWM output
// LED subtype values
#define LEDRED 1           // One or more LEDs of the given colour
#define LEDGRN 2
#define LEDBLU 3
#define LEDWHT 4
#define LEDYEL 5
#define LEDSFT 6        // Soft white
#define LEDORG 7        // orange ?
// LED Action values
#define DEVICE_BLNKA 2     // Blink on phase A
#define DEVICE_BLNKB 3     // Blink on phase B
#define DEVICE_FLSH1 4     // Flash 1s
#define DEVICE_FLSH2 5     // Flash 0.5s
#define RND2S 6     // Random 20% duty cycle short time (10s)
#define RND5S 7     // Random 50% duty cycle short time
#define RND8S 8    // Random 80% duty cycle short time
#define RND2M 9     // Random 20% duty cycle medium time (60s)
#define RND5M 10    // Random 50% duty cycle medium time
#define RND8M 11    // Random 80% duty cycle medium time
#define RND2L 12    // Random 20% duty cycle medium time (180s)
#define RND5L 13    // Random 50% duty cycle medium time
#define RND8L 14    // Random 80% duty cycle medium time
// values 15-19 not used
#define FLCK1 20    // Flicker fast (<1s cycle time)
#define FLCK2 21    // Flicker medium (2s cycle time)
#define FLCK3 22    // Flicker long (5s cycle time)
// values 23 -24 not used
#define CYC13 25    // Cycle 1 of 3
#define CYC23 26    // Cycle 2 of 3
#define CYC33 27    // Cycle 3 of 3
#define CYC16 28    // As above, cycle of 6
#define CYC26 29
#define CYC36 30
#define CYC46 31
#define CYC56 32
#define CYC66 33

/* Layout of RGB LED block

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
| 008    | Device red pin      | Populate with WEP          |
+--------+---------------------+----------------------------+
| 009    | Device green pin    | Populate with WEP          |
+--------+---------------------+----------------------------+
| 010    | Device blue pin     | Populate with WEP          |
+--------+---------------------+----------------------------+
| 011    | Unused              |                            |
| to     |                     |                            |
| 015    |                     |                            |
+--------+---------------------+----------------------------+

*/
// -- RGB LEDs
#define DEVICE_RGBLED 2 // device type
#define DEVICE_RGBPINR 8   // offset of  red pin
#define DEVICE_RGBPING 9   // offset of  red pin
#define DEVICE_RGBPINB 10  // offset of  red pin

// values 19 to 23 not used
// -- Shift Registers
#define SHFT8  24       // 8 output shift register
#define SHFT16  25       // 16 output shift register
#define SHFT24  26       // 24 output shift register
#define SHFT32  27       // 32 output shift register
#define SHFT48  28       // 48 output shift register
// values 29-31 not used
// -- Motors (to be done)
// 32 - 49
// -- Relays (to be done)
// 50 - 59
// LCD Screen
#define LCDD0 60        // LCD Screen data lines
#define LCDD1 61
#define LCDD2 62
#define LCDD3 63
#define LCDRW 64
#define LCDRS 65
#define LCDEN 66
// values 67 - 69 not used

// inputs
#define TMPHMD  70      // Temperature and humidity sensor
#define BUTTON  71      // Momentary button
#define SWITCH  72      // on/off switch
#define GENPOT  73      // Generic potentiometer
#define IRRECV  74      // IR Receiver
#define RTC     75      // Real-time clock
#define SDCARD  76      // SD card


// Input actions
#define SMP01 100   // sample every 100ms
#define SMP05 101   // sample every 500ms
#define SMP10 102   // sample every 1s
#define SMP20 103   // sample every 2s
#define SMP50 104   // sample every 5s
#define SMP1M 105   // sample every 1 minute

