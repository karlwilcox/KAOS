#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "monitor.h"
#include "actions.h"
#include "devices.h"
#include "memory_map.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"

char com[MONITOR_COMMAND_SIZE], arg[MONITOR_ARGUMENT_SIZE], val[MONITOR_VALUE_SIZE];
char deviceStates[MAX_DEVICES * STATE_BLOCK_SIZE];
#define TICKRATE 20
char f_03d[] = "%03u ";
// pointers to devices (only created if they exist)
SimpleDHT11 *dht11;
LiquidCrystal *lcd = NULL;
char lcdLine0[17];
char lcdLine1[17];
unsigned int  ticks100ms = 0, ticks1s = 0, ticks10s = 0;
unsigned int runAddr = 0;


void setup(){
  Serial.begin(9600);
  if (analogRead(BUTTON_PIN) > 100) { // button held down on startup
    // run monitor only (in case of corrupt / broken EEPROM)
    stateWrite(DEVICE_BOARD, STATE_FLAG, FLAG_ECHO | FLAG_PROG);
    Serial.print("> ");
  } else {
    setupDevices();
    // set the default flag state, but never set multiline by default
    stateWrite(DEVICE_BOARD, STATE_FLAG, eepromRead(DEVICE_BOARD, EEPROM_FLAG) 
                & ~(FLAG_MULTILINE | FLAG_SLEEP));
  }
}

void loop(){
  if (!(stateRead(DEVICE_BOARD, STATE_FLAG) & FLAG_SLEEP) && monitorInput()) {
    monitorRun();
    monitorOutput();
  }
  if (stateRead(DEVICE_BOARD, STATE_FLAG) & FLAG_RUN) {
    delay(TICKRATE);
    if (++ticks100ms >= 5) {
      ticks100ms = 0;
    }
    if (++ticks1s >= 50) {
      ticks1s = 0;
    }
    if (++ticks10s >= 500) {
      ticks10s = 0;
    }
    // special handling for block 0 timer
    for (int block = 1; block < MAX_DEVICES; block++) {
      if (block > 0) {
        if (stateRead(block, STATE_ACTION) == DEVICE_END) break; // marks end of live devices
        if (stateRead(block, STATE_ACTION) >= DEVICE_UPPER) continue; // suspended or deleted devices
      }
      bool actioned = false;
      byte ttr = stateRead(block, STATE_TTR);
      if (ttr == 0) continue; // no action needed
      // the top two bits are the  counter units
      byte units = ttr & 0b11000000;
      // the lower 6 bits are the counter value
      ttr &= 0b00111111;
      switch (units) {
        // this counter is reduced on every tick
        case TTR_UNIT_20ms:
          if (--ttr == 0) {
            doAction(block);
            actioned = true;
          }
          break;
        case TTR_UNIT_100ms:
        // Note code carefully, we rely on short-circuit evaluation here
        // as the 2nd part of the expression has a required side-effect
          if (ticks100ms == 0 && --ttr == 0) {
            doAction(block);
            actioned = true;
          }
          break;
        case TTR_UNIT_1s:
          if (ticks1s == 0 && --ttr == 0) {
            doAction(block);
            actioned = true;
          }
          break;
        case TTR_UNIT_10s:
          if (ticks10s == 0 && --ttr == 0) {
            doAction(block);
            actioned = true;
          }
          break;
      }
      if (!actioned) {
        // put the units back in the top two bits
        ttr |= units;
        // write the modified counter value back to the state
        stateWrite(block, STATE_TTR, ttr);
      }
    }
  }
}