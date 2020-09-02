#include "ArduinoNunchuk.h"
#include "Axis.h"
#include <SoftwareSerial.h>

#define LED 13

const float sunPeriod = 86400.00f;
const float moonPeriod = 83019.014f;
const float sediPeriod = 86164.091f;

const double siderealSpeedArr[] = {0.5, 1, 8, 16, 32, 64, 128, 400, 800};
const int maxStep = 6;//sizeof(siderealSpeedArr)/sizeof(*siderealSpeedArr);

ArduinoNunchuk* nunchuk;
Axis* axisX;
Axis* axisY;

SoftwareSerial sSerial(4,5);

#define PT_WAIT(pt, timestamp, usec) PT_WAIT_UNTIL(pt, millis() - *timestamp > usec);*timestamp = millis();
static struct pt pt1, pt2, pt3;
static struct pt_sem mutex;
int serialSem=1;

bool serialInUse;
bool YInvMode;
bool SHemis;
int currentSpeedStep;
int currentStepX;
int currentStepY;
int currentStepC;
int currentStepZ;


extern bool SendReceiveMsg(char* msgToSend, char* res);
extern int getStep(int  n);
extern void debug(int n);
extern void debug(char* c);
extern void beep(int TONE, int TIME);
