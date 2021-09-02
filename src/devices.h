#ifndef DEVICES_H
#define DEVICES_H

unsigned int findDevice(const char *tag);
void setupDevices();
void setupDevice(unsigned int block);
bool getDevice(unsigned int addr);
void setDevice(unsigned int addr, unsigned int value);

#endif // !DEVICES_H
