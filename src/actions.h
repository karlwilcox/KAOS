#ifndef ACTIONS_H
#define ACTIONS_H

void blink();
void flash(unsigned int action);
void sampleInputs(unsigned int action);
void updateLCD();
void writeDSR(unsigned int block);
byte doAction(unsigned int block);
void updateValue(unsigned int block, byte value);

#endif // !ACTIONS_H