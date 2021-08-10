#ifndef DEVICES_H
#define DEVICES_H

unsigned int findDevice(char *tag);
void setupDevices();
void setDevice(unsigned int addr, int value = -1);
void getDevice(unsigned int addr);
void allDevices(int state);

#endif // !DEVICES_H
