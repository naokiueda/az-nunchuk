/*
 * ArduinoNunchuk.cpp - Improved Wii Nunchuk library for Arduino
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

#include <Arduino.h>
#include <Wire.h>
#include "ArduinoNunchuk.h"

#define ADDRESS 0x52

void ArduinoNunchuk::init()
{
  Wire.begin();
  ArduinoNunchuk::_sendByte(0x55, 0xF0);
  ArduinoNunchuk::_sendByte(0x00, 0xFB);
  ArduinoNunchuk::_sendByte(0x00, 0x40);
  ArduinoNunchuk::update();
}
bool ArduinoNunchuk::update()
{
  int count = 0;
  int values[6];
  Wire.requestFrom(ADDRESS, 6);
  while(Wire.available())
  {
    values[count] = Wire.read();
    count++;
  }


  ArduinoNunchuk::analogX = (unsigned char)values[0];
  ArduinoNunchuk::analogY = (unsigned char)values[1];
  //ArduinoNunchuk::accelX = (values[2] << 2) | ((values[5] >> 2) & 3);
  //ArduinoNunchuk::accelY = (values[3] << 2) | ((values[5] >> 4) & 3);
  //ArduinoNunchuk::accelZ = (values[4] << 2) | ((values[5] >> 6) & 3);
  ArduinoNunchuk::zButton = !((values[5] >> 0) & 1);
  ArduinoNunchuk::cButton = !((values[5] >> 1) & 1);

  ArduinoNunchuk::_sendByte(0x00, 0x00);

  if(ArduinoNunchuk::analogX==1 && ArduinoNunchuk::analogY==1){// || ArduinoNunchuk::analogX==255 && ArduinoNunchuk::analogX==255){
    return false;
  }

  ArduinoNunchuk::analogXArr[ArduinoNunchuk::currentIndex]=ArduinoNunchuk::analogX;
  ArduinoNunchuk::analogYArr[ArduinoNunchuk::currentIndex]=ArduinoNunchuk::analogY;
  //ArduinoNunchuk::accelXArr[ArduinoNunchuk::currentIndex]=ArduinoNunchuk::accelX;
  //ArduinoNunchuk::accelYArr[ArduinoNunchuk::currentIndex]=ArduinoNunchuk::accelY;
  //ArduinoNunchuk::accelZArr[ArduinoNunchuk::currentIndex]=ArduinoNunchuk::accelZ;
  ArduinoNunchuk::zButtonArr[ArduinoNunchuk::currentIndex]=ArduinoNunchuk::zButton;
  ArduinoNunchuk::cButtonArr[ArduinoNunchuk::currentIndex]=ArduinoNunchuk::cButton;
  ArduinoNunchuk::currentIndex++;
  if(ArduinoNunchuk::currentIndex==ArduinoNunchuk::AveQty)
    ArduinoNunchuk::currentIndex=0;

  double AveanalogX=0;
  double AveanalogY=0;
  //double AveaccelX=0;
  //double AveaccelY=0;
  //double AveaccelZ=0;
  double AvezButton=0;
  double AvecButton=0;
  for(int i=0; i<ArduinoNunchuk::AveQty; i++){
    AveanalogX += ArduinoNunchuk::analogXArr[i];
    AveanalogY += ArduinoNunchuk::analogYArr[i];
    //AveaccelX += ArduinoNunchuk::accelXArr[i];
    //AveaccelY += ArduinoNunchuk::accelYArr[i];
    //AveaccelZ += ArduinoNunchuk::accelZArr[i];
    AvezButton += ArduinoNunchuk::zButtonArr[i];
    AvecButton += ArduinoNunchuk::cButtonArr[i];
  }
  AveanalogX /= (double)ArduinoNunchuk::AveQty;
  AveanalogY /= (double)ArduinoNunchuk::AveQty;
  //AveaccelX /= (double)ArduinoNunchuk::AveQty;
  //AveaccelY /= (double)ArduinoNunchuk::AveQty;
  //AveaccelZ /= (double)ArduinoNunchuk::AveQty;
  AvezButton /= (double)ArduinoNunchuk::AveQty;
  AvecButton /= (double)ArduinoNunchuk::AveQty;

  ArduinoNunchuk::analogXAve = (int)(floor(AveanalogX));
  ArduinoNunchuk::analogYAve = (int)(floor(AveanalogY));
  //ArduinoNunchuk::accelXAve = (int)(floor(AveaccelX));
  //ArduinoNunchuk::accelYAve = (int)(floor(AveaccelY));
  //ArduinoNunchuk::accelZAve = (int)(floor(AveaccelZ));

  stepZ = AvezButton==0?0:1;
  stepC = AvecButton==0?0:1;  

  //For AZ GTi
  ArduinoNunchuk::stepX = (int)(floor((AveanalogX-3)/50)-2);
  ArduinoNunchuk::stepY = (int)(floor((AveanalogY-3)/50)-2);
  if(ArduinoNunchuk::stepX >2)ArduinoNunchuk::stepX =2;
  if(ArduinoNunchuk::stepX <-2)ArduinoNunchuk::stepX =-2;
  if(ArduinoNunchuk::stepY >2)ArduinoNunchuk::stepY =2;
  if(ArduinoNunchuk::stepY <-2)ArduinoNunchuk::stepY =-2;

  return true;
}

void ArduinoNunchuk::_sendByte(byte data, byte location)
{
  Wire.beginTransmission(ADDRESS);
  Wire.write(location);
  Wire.write(data);
  Wire.endTransmission();
  delay(10);
}
