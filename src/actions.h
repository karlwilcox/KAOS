#ifndef ACTIONS_H
#define ACTIONS_H

void blink();
void flash(unsigned int action);
void sampleInputs(unsigned int action);
void updateLCD();
void myDigitalWrite(unsigned int pin, unsigned int value);
void writeDSR(shiftRegister *sr);

#endif // !ACTIONS_H