#ifndef DEVICES_H
#define DEVICES_H

unsigned int findDevice(const char *tag);
void setupDevices();
void getDevice(unsigned int addr);
void allDevices(int state);

#endif // !DEVICES_H
