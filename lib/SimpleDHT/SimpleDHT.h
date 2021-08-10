
#ifndef __SIMPLE_DHT_H
#define __SIMPLE_DHT_H

#include <Arduino.h>

/*
    Simple DHT11

    Simple, Stable and Fast DHT11 library.

    The circuit:
    * VCC: 5V or 3V
    * GND: GND
    * DATA: Digital ping, for example, 2.

    23 Jan 2016 By winlin <winlin@vip.126.com>

    https://github.com/winlinvip/SimpleDHT#usage

*/
class SimpleDHT11 {
public:
    // to read from dht11.
    // @param pin the DHT11 pin.
    // @param ptemperature output, NULL to igore.
    // @param phumidity output, NULL to ignore.
    // @param pdata output 40bits sample, NULL to ignore.
    // @remark the min delay for this method is 1s.
    int read(int pin, byte* ptemperature, byte* phumidity, byte pdata[40]);
private:
    // confirm the OUTPUT is level in us, 
    // for example, when DHT11 start sample, it will
    //    1. PULL LOW 80us, call confirm(pin, 80, LOW)
    //    2. PULL HIGH 80us, call confirm(pin, 80, HIGH)
    // @return 0 success; oterwise, error.
    // @remark should never used to read bits, 
    //    for function call use more time, maybe never got bit0.
    // @remark please use simple_dht11_read().
    int confirm(int pin, int us, byte level);
    // @data the bits of a byte.
    // @remark please use simple_dht11_read().
    byte bits2byte(byte data[8]);
    // read temperature and humidity from dht11.
    // @param pin the pin for DHT11, for example, 2.
    // @param data a byte[40] to read bits to 5bytes.
    // @return 0 success; otherwise, error.
    // @remark please use simple_dht11_read().
    int sample(int pin, byte data[40]);
    // parse the 40bits data to temperature and humidity.
    // @remark please use simple_dht11_read().
    int parse(byte data[40], byte* ptemperature, byte* phumidity);
};

#endif