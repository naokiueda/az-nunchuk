/*
This file is a part of AZ-Nunchuk project.
https://az-nunchuk.fun/

Author: Naoki Ueda, 2020/09/03

This project use external library, "Arduino Nunchuk".

CC BY 4.0
This work is licensed under the Creative Commons Attribution 4.0 International License. To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/
#include <arduino.h>
#include "pt.h"
#include "Axis.h"
#include "Float64.h"
#include <SoftwareSerial.h>

//f64 aa = f64(5) / f64(2);

extern SoftwareSerial sSerial;
extern int serialSem;
extern struct pt pt1, pt2, pt3;

const float sunPeriod = 86400.00f;
const float moonPeriod = 83019.014f;
const float sediPeriod = 86164.091f;


Axis::Axis(int axisId, int eqMode, float baseSecPR, bool isSHemisphere)
{
  AxisId = axisId;
  currentSpeed = 0;
  isRunning = false;
  isTracking = false;
  isCCW = false;
  isFastMode = false;
  isInitialized = false;
  isEQmode = eqMode;//not yet fixed
  isSHemis = isSHemisphere;

  //isNextOperationReserved=false;
  //nextSpeed=0;

  sprintf(sendmsg, ":a%d\0", AxisId);
  CPR=0;
  if(Msg(sendmsg, response)){
    debug("response=");
    debug(response);
    debug("\r\n");
    CPR = convertProtcolHexToInt(response);
    debug("CPR(");
    debug(AxisId);
    debug(") = ");
    //sSerial.print(CPR);
    debug("\r\n");
  }
  //Get TMR_Freq
  //sprintf(sendmsg, ":b%d\r\0", AxisId);
  
  sprintf(sendmsg, ":b%d\0", AxisId);
  TMR_Freq=0;
  if(Msg(sendmsg, response)){
    debug("response=");
    debug(response);
    debug("\r\n");
    TMR_Freq = convertProtcolHexToInt(response);
    debug("TMR Freq(");
    debug(AxisId);
    debug(") = ");
    //sSerial.print(TMR_Freq);
    isInitialized=1;
  }

  

  //Get ExtendedInquire
  baseT1=0;
  sprintf(sendmsg, ":q%d010000\0", AxisId);
  if(Msg(sendmsg, response)){
    TwoAxesMustStartSeparetely = (response[3] & 0x02)==0x00?false:true;//B1: Tow axes must start seperately
    //isEQmode = (response[2] & 0x08)==0x00?0:-1;

    if(CPR>0 && TMR_Freq>0){
      //debug("***********************************");
      baseT1 = (baseSecPR / CPR) * TMR_Freq;
      //sSerial.print(baseT1);
      isInitialized=1;
    }
  }

  double siderealSpeedArr[] = {0.5, 1, 8, 16, 32, 64, 128, 400, 800};
  for(int i=0; i<9; i++){
    SetT1parameter(siderealSpeedArr[i]);
  }
  for(int i=0; i<9; i++){
    SetT1parameter(-1 * siderealSpeedArr[i]);
  } 
  
}

bool Axis::Msg(char* msgToSend, char* res){

  //PT_WAIT_UNTIL(pt, serialSem==1);
  serialSem--;
  bool ret = SendReceiveMsg(msgToSend, res, (int)AxisId);    
  serialSem++;
  return ret;
}

bool Axis::NeedAction(float desiredSpeed) {
  if(AxisId==1 && isEQmode>0 && abs(desiredSpeed)<=128){
    //for EQ Tracking
    //desiredSpeed+=1.0;
    if(!isSHemis){
      desiredSpeed+=1.0;
    }else{
      desiredSpeed*=-1.0;
      desiredSpeed-=1.0;
    }    
  }
  if(currentSpeed == desiredSpeed){
    actualSpeed = desiredSpeed;
    return false;
  }else{
    actualSpeed = desiredSpeed;
    sSerial.print("SPEED=");
    sSerial.println(actualSpeed);
    return true;
  }
}

//Main Logic
bool Axis::SetSpeed(float desiredSpeed) {

  //if(AxisId==1 && isEQmode>0 && abs(desiredSpeed)<128){
  //  //for EQ Tracking
  //  desiredSpeed+=1.0;
  //}
  //if(currentSpeed == desiredSpeed) return true;
  //
  //sSerial.print("Axis ");
  //sSerial.print(AxisId);
  //sSerial.print("  x");
  //sSerial.println(desiredSpeed);
  
  //if(!GetStatus()){
  //  return 0;
  //} 
  
  //ゼロの時は停止
  //トラッキングモードの時は停止
  //回転方向逆転するときはSTOP
  //Fastモードから変更するときは一旦STOP ?
  if(desiredSpeed == 0 
  || isTracking ==1 
  || currentSpeed * desiredSpeed < 0 
  || currentSpeed > 128){
    Stop();   
    int retry=0;
    unsigned long timestamp = 0;
    while(isRunning==1 && retry<10){
      //delay(100);
      if(AxisId==1){
        //PT_WAIT_UNTIL(pt, millis() - timestamp > 100);
      }else if(AxisId==2){
        //PT_WAIT_UNTIL(pt, millis() - timestamp > 100);
      }
      timestamp = millis();
      GetStatus();
      retry++;
    }
    if(retry>=10){
      return 0;
    }
    if(desiredSpeed != 0){
      SetTheMotionMode(desiredSpeed);
      SetT1parameter(desiredSpeed);
      Start();
    }else{
    }
    
    currentSpeed = desiredSpeed;
    return 1;
  }


  
  //今止まっている時は方向を指示
  else if(currentSpeed == 0 && desiredSpeed!=0){
    SetTheMotionMode(desiredSpeed);
    SetT1parameter(desiredSpeed);
    Start();
    currentSpeed = desiredSpeed;
    return 1;
  }

  //同方向移動中（スローモード） T1を変えるだけ
  SetT1parameter(desiredSpeed);
  currentSpeed = desiredSpeed;
  return 1;
}

//void Axis::setBaseSec(float baseT1){
//  baseT1 = baseT1 /CPR * TMR_Freq;
//}

float Axis::convertProtcolHexToInt(char* swValue)
{
  if (swValue[0] != '=')return 0;
  int a, b, c;
  sscanf(swValue, "=%02X%02X%02X", &a, &b, &c);
  /*
  Serial.print("a=");
  Serial.print(a);
  Serial.println("   ");
  Serial.print("b=");
  Serial.print(b);
  Serial.println("   ");
  Serial.print("c=");
  Serial.print(c);
  Serial.println("   ");
  */
  float ret = (float)a + (float)b*256 + (float)c*256*256;
  /*
  Serial.print("Other ");
  Serial.println(ret);
  */
  return(ret);
}


bool Axis::SetTheMotionMode(double desiredSpeed) {
  sprintf(sendmsg, ":G%d%d%d\0", AxisId, ((abs(desiredSpeed) > 128) ? 3 : 1), (desiredSpeed < 0 ? 1 : 0));// when high speed slewing is required(For example, move an axis with higher then 128x sidereal rate).
  return Msg(sendmsg, response);
};

bool Axis::SetT1parameter(float desiredSpeed) {

  float T1 = baseT1 / abs(desiredSpeed);
  float T1_Preset = floor(T1 + 0.5);//Round 
  float d2 = floor(T1_Preset / 65536);
  float d1 = floor((T1_Preset - d2 * 65536) / 256);
  float d0 = floor(T1_Preset - d2 * 65536 - d1 * 256);
  /*
  debug("****");
  sSerial.print(d0);
  sSerial.print(",");
  sSerial.print(d0);
  sSerial.print(",");
  sSerial.print(d0);
  sSerial.println("");
  */
  sprintf(sendmsg, ":I%d%02X%02X%02X\0", AxisId, (int)d0, (int)d1, (int)d2);
  if(sendmsg[3]=='0' && sendmsg[4]=='0' && sendmsg[5]=='0' && sendmsg[6]=='0' && sendmsg[7]=='0' && sendmsg[8]=='0'){
    sSerial.println("ERROR??");
    sSerial.println(sendmsg);
    sSerial.println(desiredSpeed);
    sSerial.println(T1);
    sSerial.println(T1_Preset);
    sSerial.println(d2);
    sSerial.println(d1);
    sSerial.println(d0);
    sSerial.println((int)d2);
    sSerial.println((int)d1);
    sSerial.println((int)d0);
    
  }
  /*
  sSerial.print("x");
  sSerial.print(desiredSpeed);
  sSerial.print("  ");
  sSerial.print(baseT1);
  sSerial.print("  ");
  sSerial.print(T1);
  sSerial.print("  ");
  sSerial.print(T1_Preset);
  sSerial.print("  ");
  sSerial.print(d0);
  sSerial.print("  ");
  sSerial.print(d1);
  sSerial.print("  ");
  sSerial.print(d2);
  
  sSerial.print("  ");
  sSerial.print(sendmsg);
  */
  return Msg(sendmsg, response);
};

bool Axis::Start() {
  sprintf(sendmsg, ":J%d%\0", AxisId);
  return Msg(sendmsg, response);
};

bool Axis::Stop() {
  currentSpeed = 0;
  sprintf(sendmsg, ":K%d%\0", AxisId);
  return Msg(sendmsg, response);
};


bool Axis::GetStatus() {
  sprintf(sendmsg, ":f%d%\0", AxisId);
  bool ret = Msg(sendmsg, response);
  if(ret==true){
    isRunning = (response[2] == '0' ? 0 : 1);
    isTracking = response[1] & 0x01;
    isCCW = response[1] & 0x02;
    isFastMode = response[1] & 0x04;
  }
  return ret;
};

bool Axis::update(){
  

}
void Axis::TestSetT1parameter(float desiredSpeed) {

  float T1 = baseT1 / abs(desiredSpeed);
  long int T1_Preset = (long int)floor(T1 + 0.5);//Round 
  long int d2 = T1_Preset / 65536L;
  long int d1 = (T1_Preset - d2 * 65536L) / 256L;
  long int d0 = T1_Preset - d2 * 65536L - d1 * 256L;
  sprintf(sendmsg, ":%02X%02X%02X\0", (int)d0, (int)d1, (int)d2);

  /*
  Serial.print("Speed");
  Serial.print(":");
  Serial.print(desiredSpeed);
  Serial.print(",  ");

  Serial.print(sendmsg);
  Serial.print(",  ");

  Serial.print("d0");
  Serial.print(":");
  Serial.print((int)d0);
  Serial.print(",  ");

  Serial.print("d1");
  Serial.print(":");
  Serial.print((int)d1);
  Serial.print(",  ");

  Serial.print("d2");
  Serial.print(":");
  Serial.print((int)d2);
  Serial.println("  ");
  */
};
