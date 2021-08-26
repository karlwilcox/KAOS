#ifndef DEVICES_H
#define DEVICES_H

unsigned int findDevice(const char *tag);
void setupDevices();
unsigned int setupDevice(unsigned int block, unsigned int deviceAction);
void getDevice(unsigned int addr);
void setDevice(unsigned int addr, unsigned int value);

#endif // !DEVICES_H
