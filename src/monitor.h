#ifndef MONITOR_H
#define MONITOR_H
#include <Arduino.h>
/*
 * Definitions for monitor program
 */


void monitorRun(); // Carry out monitor action
bool monitorInput(); // check for monitor input
void monitorOutput(); // Send response
unsigned int char2int(char *in);

//////////////////// IMPORTANT //////////////////////////
// When creating new devices, set the device type LAST //
// When modifying existing devices, set the device     //
// to DEVICE_DELETED, make changes, then reset (except //
// for single byte changes)                            //
/////////////////////////////////////////////////////////

#define MONITOR_VALUE_SIZE 24
#define MONITOR_COMMAND_SIZE 4
#define MONITOR_ARGUMENT_SIZE 5


#endif
