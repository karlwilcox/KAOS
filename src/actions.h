#ifndef ACTIONS_H
#define ACTIONS_H

void blink();
void flash(unsigned int action);
void sampleInputs(unsigned int action);
void updateLCD();
void writeDSR(unsigned int block);
void doAction(unsigned int block);
void updateDigital(unsigned int block, byte value);
void updatePWM(unsigned int block, byte value);
// byte ms2ttr(unsigned int ms);
byte randomTTR(byte value);
unsigned int tt2ms(byte ttr);

#endif // !ACTIONS_H