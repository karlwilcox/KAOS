#ifndef MONITOR_H
#define MONITOR_H
#include <Arduino.h>
/*
 * Definitions for monitor program
 */

// Required global variables:
// char monitorBuffer[32];
// int monitorFlags = 0;

void monitorRun(); // Carry out monitor action
bool monitorInput(); // check for monitor input
void monitorOutput(); // Send response

//////////////////// IMPORTANT //////////////////////////
// When creating new devices, set the device type LAST //
// When modifying existing devices, set the device     //
// to DEVICE_DELETED, make changes, then reset (except //
// for single byte changes)                            //
/////////////////////////////////////////////////////////

#define MONITOR_DEBUG true
#define MONITOR_BUFFER_SIZE 24


#endif
