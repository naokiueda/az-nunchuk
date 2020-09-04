/*
This file is a part of AZ-Nunchuk project.
https://az-nunchuk.fun/

Author: Naoki Ueda, 2020/09/03

This project depends on following external libraries.
"ArduinoNunchuk"  http://www.gabrielbianconi.com/projects/arduinonunchuk/
"protothreads" http://dunkels.com/adam/pt/download.html

CC BY 4.0
This work is licensed under the Creative Commons Attribution 4.0 International License. To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/
#ifndef __AXIS_H__
#define __AXIS_H__
#include <arduino.h>

extern bool SendReceiveMsg(char* msgToSend, char* res, int axisid);
extern void debug(int n);
extern void debug(char* c);
extern bool serialInUse;

class Axis
{
public:

  //constructor
  Axis(int axisId, int eqMode, float pbaseT1, bool isSHemisphere);

  double requestSpeed;

  double actualSpeed;

  bool NeedAction(float desiredSpeed) ;
  bool SetSpeed(float siderealSpeedRatio);
  bool isInitialized;
  int isEQmode;
  bool isSHemis;
  void setBaseSec(float baseSec);

  bool update();

  int mailbox[10];
  
  ~Axis();
  int status;
  const int STATUS_MOVING=5;
  const int STATUS_STOPPING=1;
  const int STATUS_STOPPED=0;
  const int STATUS_STOPPING_FORNEXT=4;
  const int STATUS_WAITING_SERIAL=2;
  const int STATUS_WAITING_RESPONSE=3;

//private:
  char AxisId;//id of this axis
  
  //Set in constructor ans used like counstant value
  float  TMR_Freq;//16000000 if AZ GTi
  float  CPR;// 2073600 for AZ GTi
  float baseT1;
  const double SiderealSpeed = 0.00417807901211643;//(double)(360) / (double)((23 * 3600 + 56 * 60 + 4)); // 360/(23:56:04 in sec) = 0.004178079 deg/sec =恒星時

  //Variables
  double currentSpeed;
  bool isRunning;
  bool isTracking;
  bool isCCW;
  bool isFastMode;
  bool TwoAxesMustStartSeparetely;

  char sendmsg[11];
  char response[11];
  
  bool Msg(char* msgToSend, char* res);
  float convertProtcolHexToInt(char* swValue);
  bool SetTheMotionMode(double desiredSpeed);
  bool SetT1parameter(float desiredSpeed);
  void TestSetT1parameter(float desiredSpeed);
  bool Start();
  bool Stop();
  bool GetStatus();
};

#endif
