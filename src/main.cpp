#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "monitor.h"
#include "actions.h"
#include "devices.h"
#include "memory_map.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"

char monitorBuffer[MONITOR_BUFFER_SIZE];
char com[4], arg[5], val[MONITOR_BUFFER_SIZE - 7];
char deviceStates[MAX_DEVICES * STATE_BLOCK_SIZE];
#define TICKRATE 20
char f_03d[] = "%03u ";
// pointers to devices (only created if they exist)
SimpleDHT11 *dht11;
LiquidCrystal *lcd = NULL;
char lcdLine0[17];
char lcdLine1[17];
unsigned int ticks20ms = 0, ticks100ms = 0, ticks1s = 0, ticks10s = 0;


void setup(){
  Serial.begin(9600);
  setupDevices();
  // set the default flag state
  stateWrite(DEVICE_BOARD, STATE_FLAG, eepromRead(DEVICE_BOARD, ADDRESS_FLAG));
}

void loop(){
  if (monitorInput()) {
    monitorRun();
    monitorOutput();
  }
  if (stateRead(DEVICE_BOARD, STATE_FLAG) & FLAG_RUN) {
    delay(TICKRATE);
    ticks20ms += 1;
    if (++ticks100ms >= 5) {
      ticks100ms = 0;
    }
    if (++ticks1s >= 50) {
      ticks1s = 0;
    }
    if (++ticks10s >= 500) {
      ticks10s = 0;
      ticks20ms = 0; // don't need to count higher than 10 seconds
    }
    for (int block = 0; block < MAX_DEVICES; block++) {
      bool updated = false;
      byte ttr = stateRead(block, STATE_TTR);
      if (ttr == 0) continue; // no action needed
      // the top two bits are the  counter units
      byte units = (ttr & 0b11000000) >> 6; 
      // the lower 6 bits are the counter value
      ttr &= 0b00111111;
      switch (units) {
        // this counter is reduced on every tick
        case TTR_UNIT_20ms:
          if (--ttr == 0) {
            ttr = doAction(block);
            updated = true;
          }
          break;
        case TTR_UNIT_100ms:
        // Note code carefully, we rely on short-circuit evaluation here
        // as the 2nd part of the expression has a required side-effect
          if (ticks100ms == 0 && --ttr == 0) {
            ttr = doAction(block);
            updated = true;
          }
          break;
        case TTR_UNIT_1s:
          if (ticks1s == 0 && --ttr == 0) {
            ttr = doAction(block);
            updated = true;
          }
          break;
        case TTR_UNIT_10s:
          if (ticks10s == 0 && --ttr == 0) {
            ttr = doAction(block);
            updated = true;
          }
          break;
      }
      if (!updated) {
        // put the units back in the top two bits
        ttr |= units << 6;
        // write the modified counter value back to the state
      }
      stateWrite(block, STATE_TTR, ttr);
    }
  }
}