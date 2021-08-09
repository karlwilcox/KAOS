#include <Arduino.h>
#include <EEPROM.h>
#include "monitor.h"
#include "actions.h"
#include "devices.h"
#include "config.h"
#include "eeprom_layout.h"

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
unsigned int flags;

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
  setupDevices();
  flags = EEPROM.read(DEFAULT_FLAG);
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
    if (flags & FLAG_RUN) {
      ;
    }
  }
  if (ticks % (TICKSPERSECOND / 5) == 0) {
    // do stuff once every 5th second
    if (flags & FLAG_RUN) {
      blink();
      flash0_5s();
    }
  }
  if (ticks >= TICKSPERSECOND) {
    // do stuff once every second
    if (flags & FLAG_RUN) {
      flash1s();
    }
    // always do these
    tick();
    ticks = 0;
  }
}