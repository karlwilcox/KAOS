#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "monitor.h"
#include "actions.h"
#include "devices.h"
#include "eeprom_layout.h"
#include "SimpleDHT.h"
#include "LiquidCrystal.h"

char monitorBuffer[MONITOR_BUFFER_SIZE];
char com[4], arg[5], val[MONITOR_BUFFER_SIZE - 7];
#define TICKRATE 20
#define TICKSPERSECOND (1000 / TICKRATE)
unsigned int ticks = 0;
int hours = 18;
int minutes = 0;
int seconds = 0;
int temperature = 20;
int humidity = 50;
shiftRegister sr1;
uint16_t flags;
// pointers to devices (only created if they exist)
SimpleDHT11 *dht11;
LiquidCrystal *lcd = NULL;
char lcdLine0[17];
char lcdLine1[17];
char stateSR[4]; // bit map for digital shift register (up to 48 outputs)
// shadow array for device actions,
// intially populated from EEPROM
byte deviceActions[MAX_DEVICES]; // should really malloc this TODO!!

void tick() {
      if (++seconds >= 60) {
        seconds = 0;
        if (++minutes >= 60) {
            minutes = 0;
            if (++hours >= 24) {
                hours = 0;
            }
        }
    }
}

void setup(){
  Serial.begin(9600);
  for (int i = 1; i < MAX_DEVICES; i++) {
    deviceActions[i] = 0;
  }
  setupDevices();
  flags = EEPROM.read(ADDRESS_FLAG);
}

void loop(){
  if (monitorInput()) {
    monitorRun();
    monitorOutput();
  }
  delay(TICKRATE);
  ticks += 1;
  if (ticks % (TICKSPERSECOND / 10) == 0) {
    // do stuff once every 10th second
    if (flags & FLAG_AUTO_OUTPUTS) {
      ;
    }
  }
  if (ticks % (TICKSPERSECOND / 4) == 0) {
    // do stuff once every quarter second
    if (flags & FLAG_AUTO_OUTPUTS) {
      if (lcd != NULL) updateLCD();
    }
  }
  if (ticks % (TICKSPERSECOND / 5) == 0) {
    // do stuff once every 5th second
    if (flags & FLAG_AUTO_OUTPUTS) {
      blink();
      flash(ACTION_FLSH2);
      flash(ACTION_FLSH3);
    }
  }
  if (ticks >= TICKSPERSECOND) {
    // do stuff once every second
    if (flags & FLAG_AUTO_OUTPUTS) {
      flash(ACTION_FLSH1);
    }
    if (flags & FLAG_AUTO_INPUTS) {
      ;
    }
    // always do this
    tick();
    // every 10 seconds
    if ( seconds % 10 == 0) {
      if (flags & FLAG_AUTO_INPUTS) {
        sampleInputs(ACTION_SMP10S);
      }
    }
    // once per minute
    if ( seconds == 30) {
      if (flags & FLAG_AUTO_INPUTS) {
        sampleInputs(ACTION_SMP1M);
      }
    }
    ticks = 0;
  }
}