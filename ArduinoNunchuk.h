/*
 * ArduinoNunchuk.h - Improved Wii Nunchuk library for Arduino
 *
 * Copyright 2011-2013 Gabriel Bianconi, http://www.gabrielbianconi.com/
 *
 * Project URL: http://www.gabrielbianconi.com/projects/arduinonunchuk/
 *
 * Based on the following resources:
 *   http://www.windmeadow.com/node/42
 *   http://todbot.com/blog/2008/02/18/wiichuck-wii-nunchuck-adapter-available/
 *   http://wiibrew.org/wiki/Wiimote/Extension_Controllers
 *
 */

#ifndef ArduinoNunchuk_H
#define ArduinoNunchuk_H
#include <Arduino.h>

class ArduinoNunchuk
{
  public:
    int analogX;
    int analogY;
    //int accelX;
    //int accelY;
    //int accelZ;
    int zButton;
    int cButton;

    int analogXAve;
    int analogYAve;
    //int accelXAve;
    //int accelYAve;
    //int accelZAve;
    //int zButtonAve;
    //int cButtonAve;

    int stepX;
    int stepY;
    int stepZ;
    int stepC;

    int notifyFlgX;
    int notifyFlgY;
    int notifyFlgZ;
    int notifyFlgC;

    void init();
    bool update();

  private:
    void _sendByte(byte data, byte location);
    const static int AveQty = 5;
    int currentIndex=0;

    int analogXArr[AveQty];
    int analogYArr[AveQty];
    //int accelXArr[AveQty];
    //int accelXArr[AveQty];
    //int accelXArr[AveQty];
    int zButtonArr[AveQty];
    int cButtonArr[AveQty];
};

#endif
