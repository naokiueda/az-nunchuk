/*
This file is a part of AZ-Nunchuk project.
https://az-nunchuk.fun/

Author: Naoki Ueda, 2020/09/03

This project depends on following external libraries.
"ArduinoNunchuk"  http://www.gabrielbianconi.com/projects/arduinonunchuk/
"protothreads" http://dunkels.com/adam/pt/download.html

Update-2025/02/08
Make receiving buffer longer, to adopt new messages of later firmware.

CC BY 4.0
This work is licensed under the Creative Commons Attribution 4.0 International License. To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/
#include <arduino.h>
#include "pt.h"
#include "pt-sem.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "WiiNunchuk4AZGTi.h"
#include "Util.h"
#include "ArduinoNunchuk.h"
#include "Axis.h"

bool msgX=false;
bool msgY=false;

bool SendReceiveMsg(char* msgToSend, char* res, int axisId) {
  serialInUse=true;
  sSerial.println("");
  sSerial.print(" ");
  sSerial.print(msgToSend);
  int len = strlen(msgToSend);
  msgToSend[len] = '\r';
  msgToSend[len+1] = '\0';
  Serial.print(msgToSend);
  int idx = 0;
  //delay(10);
  char retry=0;
  Serial.flush();
  while (Serial.available() > 0 || retry<100) {
    char r = Serial.read();
    if(r==-1){
      delay(10);
      retry++;
      continue;
    }
    if( r=='=' == r=='!'){
      idx=0;
    }
    if (r != '\r') {
  //    debug(" ");
  //    debug(r);
      res[idx] = r;
      idx++;
    }else{
      break;
    }
    delay(10);
    retry++;
  }
  res[idx++] = '\0';
  if(retry==100){
    debug("TIMEOUT");
  }
  //sSerial.println("");
  //sSerial.print("-->");
  //sSerial.println(res);

  serialInUse=false;
  return (res[0] == '=');
}

static int threadX(struct pt *pt1) {
    PT_BEGIN(pt1);
    while(true) {
      PT_WAIT_UNTIL(pt1, msgX==true);
      //debug("Thread X ..");
      //sSerial.println(axisX->requestSpeed);
      //debugln();
      msgX=false;
      
      if(axisX->NeedAction(axisX->requestSpeed)){
        sSerial.print("X speed =");
        sSerial.println(axisX->requestSpeed);
        bool ret=false;
        PT_SEM_WAIT(pt1, &mutex);    
        ret = axisX->GetStatus();
        PT_SEM_SIGNAL(pt1, &mutex);
        unsigned long timestamp = 0;
        if(ret){
          //ゼロの時は停止
          //トラッキングモードの時は停止
          //回転方向逆転するときはSTOP
          //Fastモードから変更するときは一旦STOP ?
          if(axisX->actualSpeed == 0 
          || axisX->isTracking ==1 
          || axisX->currentSpeed * axisX->actualSpeed < 0 
          || axisX->currentSpeed > 128){
            PT_SEM_WAIT(pt1, &mutex);    
            ret = axisX->Stop();
            PT_SEM_SIGNAL(pt1, &mutex);
            while(ret==false){
              PT_WAIT_UNTIL(pt1, millis() - timestamp > 100);
              timestamp = millis();
              PT_SEM_WAIT(pt1, &mutex);    
              axisX->Stop();
              PT_SEM_SIGNAL(pt1, &mutex);
            }
            int retry=0;
            PT_SEM_WAIT(pt1, &mutex);    
            axisX->GetStatus();
            PT_SEM_SIGNAL(pt1, &mutex);
            while(axisX->isRunning==1 && retry<13){
              PT_WAIT_UNTIL(pt1, millis() - timestamp > 250);
              timestamp = millis();
              PT_SEM_WAIT(pt1, &mutex);    
              axisX->GetStatus();
              PT_SEM_SIGNAL(pt1, &mutex);
              retry++;
            }
            if(retry>=13){
              debug("Can't confirm Stop");
              continue;
            }
            if(axisX->actualSpeed != 0){
              if(isnan(axisX->actualSpeed)){
                sSerial.println("NAN!!!");
                //sSerial.println(axisX->actualSpeed);
                //sSerial.println(axisX->currentSpeed);
                //sSerial.println(axisX->requestSpeed);
              }
              PT_SEM_WAIT(pt1, &mutex);    
              axisX->SetTheMotionMode(axisX->actualSpeed);
              axisX->SetT1parameter(axisX->actualSpeed);
              axisX->Start();
              PT_SEM_SIGNAL(pt1, &mutex);
            }else{
            }
            axisX->currentSpeed = axisX->actualSpeed;
            continue;
          }
          //今止まっている時は方向を指示
          else if(axisX->currentSpeed == 0 && axisX->actualSpeed!=0){
            PT_SEM_WAIT(pt1, &mutex);    
            axisX->SetTheMotionMode(axisX->actualSpeed);
            axisX->SetT1parameter(axisX->actualSpeed);
            axisX->Start();
            PT_SEM_SIGNAL(pt1, &mutex);
            axisX->currentSpeed = axisX->actualSpeed;
            continue;
          }else{
            //同方向移動中（スローモード） T1を変えるだけ
            PT_SEM_WAIT(pt1, &mutex);    
            axisX->SetT1parameter(axisX->actualSpeed);
            axisX->currentSpeed = axisX->actualSpeed;
            PT_SEM_SIGNAL(pt1, &mutex);
          }
        }        
      }
    } 
    PT_END(pt1);
}

static int threadY(struct pt *pt2) {
    PT_BEGIN(pt2);
    while(true) {
      PT_WAIT_UNTIL(pt2, msgY==true);
      debug("Thread Y ..");
      //sSerial.println(axisY->requestSpeed);
      debugln();
      msgY=false;
      if(axisY->NeedAction(axisY->requestSpeed)){
        bool ret=false;
        PT_SEM_WAIT(pt2, &mutex);    
        ret = axisY->GetStatus();
        PT_SEM_SIGNAL(pt2, &mutex);
        unsigned long timestamp = 0;
        if(ret){
          //ゼロの時は停止
          //トラッキングモードの時は停止
          //回転方向逆転するときはSTOP
          //Fastモードから変更するときは一旦STOP ?
          if(axisY->actualSpeed == 0 
          || axisY->isTracking ==1 
          || axisY->currentSpeed * axisY->actualSpeed < 0 
          || axisY->currentSpeed > 128){
            PT_SEM_WAIT(pt2, &mutex);    
            ret = axisY->Stop();
            PT_SEM_SIGNAL(pt2, &mutex);
            while(ret==false){
              PT_WAIT_UNTIL(pt2, millis() - timestamp > 100);
              timestamp = millis();
              PT_SEM_WAIT(pt2, &mutex);
              axisY->Stop();
              PT_SEM_SIGNAL(pt2, &mutex);
            }
            int retry=0;
            PT_SEM_WAIT(pt2, &mutex);    
            axisY->GetStatus();
            PT_SEM_SIGNAL(pt2, &mutex);
            while(axisY->isRunning==1 && retry<13){
              PT_WAIT_UNTIL(pt2, millis() - timestamp > 250);
              timestamp = millis();
              PT_SEM_WAIT(pt2, &mutex);    
              axisY->GetStatus();
              PT_SEM_SIGNAL(pt2, &mutex);
              retry++;
            }
            if(retry>=13){
              debug("Can't confirm Stop");
              continue;
            }
            if(axisY->actualSpeed != 0){
              PT_SEM_WAIT(pt2, &mutex);    
              axisY->SetTheMotionMode(axisY->actualSpeed);
              axisY->SetT1parameter(axisY->actualSpeed);
              axisY->Start();
              PT_SEM_SIGNAL(pt2, &mutex);
            }else{
            }
            axisY->currentSpeed = axisY->actualSpeed;
            continue;
          }
          //今止まっている時は方向を指示
          else if(axisY->currentSpeed == 0 && axisY->actualSpeed!=0){
            PT_SEM_WAIT(pt2, &mutex);    
            axisY->SetTheMotionMode(axisY->actualSpeed);
            axisY->SetT1parameter(axisY->actualSpeed);
            axisY->Start();
            PT_SEM_SIGNAL(pt2, &mutex);
            axisY->currentSpeed = axisY->actualSpeed;
            continue;
          }else{
          //同方向移動中（スローモード） T1を変えるだけ
            PT_SEM_WAIT(pt2, &mutex);    
            axisY->SetT1parameter(axisY->actualSpeed);
            axisY->currentSpeed = axisY->actualSpeed;
            PT_SEM_SIGNAL(pt2, &mutex);
          }
        }
      }
    } 
    PT_END(pt2);
}


static int threadNunchuk(struct pt *pt) {
    // スレッド内で基準となるタイムスタンプ
    static unsigned long timestamp = 0;
    PT_BEGIN(pt);
    while(true) {
      //PT_WAIT(pt, &timestamp, 10);
      PT_WAIT_UNTIL(pt, millis() - timestamp > 10);
      timestamp = millis();
      
      if(nunchuk->update()==false){
        continue;
      }
      int newStepX=nunchuk->stepX;
      int newStepY=nunchuk->stepY;
      int newStepZ=nunchuk->stepZ;
      int newStepC=nunchuk->stepC;

      bool changedX=false;
      bool changedY=false;
      if(YInvMode){
        newStepY *= -1;
      }
      if(currentStepZ==0 && newStepZ==1){
        if(currentSpeedStep>0 && newStepX==0 && newStepY==0){
            currentSpeedStep--;
            //changedX=true;
            //changedY=true;
            debug("Speed Down :");
            debug(currentSpeedStep);
            debug("\n\r");
            beep(currentSpeedStep);
            currentStepZ = newStepZ;
            currentStepC = newStepC;        
        }
        continue;
      }else if(currentStepC==0 && newStepC==1){
        if(currentSpeedStep < maxStep-1 && newStepX==0 && newStepY==0){
            currentSpeedStep++;
            //changedX=true;
            //changedY=true;
            debug("Speed Up   :");
            debug(currentSpeedStep);
            debug("\n\r"); 
            beep(currentSpeedStep);
            currentStepZ = newStepZ;
            currentStepC = newStepC;
        }
        continue;         
      }
      if(currentStepX!=newStepX){
            changedX=true;
              debug("Joy Stick  :");
              debug(newStepX);
              debug("  ");
              debug(newStepY);
              debug("  ");
              debug("\n\r");
      }
      if(currentStepY!=newStepY){
            changedY=true;
              debug("Joy Stick  :");
              debug(newStepX);
              debug("  ");
              debug(newStepY);
              debug("  ");
              debug("\n\r");    
      }
      if(changedX==true && changedY==false){
        msgX=true;
      }else if(changedX==false && changedY==true){
        msgY=true;
      }else if(changedX==true && changedY==true){
        msgX=true;
        msgY=true;
      }
      if(msgX==true){
        axisX->requestSpeed = siderealSpeedArr[currentSpeedStep] * (double)newStepX * (SHemis?(-1.0f):1.0f);
        if(siderealSpeedArr[currentSpeedStep]>=128 && newStepX!=0){
          axisX->requestSpeed /= ((double)(abs(newStepX)));
        }       
      }
      if(msgY==true){
        axisY->requestSpeed = siderealSpeedArr[currentSpeedStep] * (double)newStepY;
        if(siderealSpeedArr[currentSpeedStep]>=128 && newStepY!=0){
          axisY->requestSpeed /= ((double)(abs(newStepY)));
        }       
      }
      
      currentStepX = newStepX;
      currentStepY = newStepY;
      currentStepZ = newStepZ;
      currentStepC = newStepC;
      PT_WAIT_UNTIL(pt, millis() - timestamp > 50);
      timestamp = millis();
    }

    PT_END(pt);
}


void setup() {
  //Init IO
  pinMode(TONE_PIN, OUTPUT);
  pinMode(LED, OUTPUT);

  //Init Serial
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
  sSerial.begin(9600);
  serialInUse=false;

  //Init Numchuk I2C
  //debug("Initializing nunchuk interface:\n");  
  nunchuk = new ArduinoNunchuk();
  nunchuk->init();

  //Operation Mode change
  YInvMode=(EEPROM.read(0x000)==1?true:false);
  SHemis=(EEPROM.read(0x001)==1?true:false);
  debug("Reading YInvMode from EEPROM:");
  debug(YInvMode);
  debugln();
  int change=0;
  for(int i=0; i<10; i++){
    nunchuk->update();
    delay(100);
  }
  if(nunchuk->stepZ){
    YInvMode=(!YInvMode);
    EEPROM.update(0x000, YInvMode?1:0);//
    debug("YInvMode Updated:");
    debug(YInvMode);
    debugln();
    if(!YInvMode){
      for(int i=0; i<8; i++){
        beep(i);
        delay(100);
      }
    }else{
      for(int i=7; i>=0; i--){
        beep(i);
        delay(100);
      }      
    }
  }
  if(nunchuk->stepC){
    SHemis=(!SHemis);
    EEPROM.update(0x001, SHemis?1:0);//
    debug("SHemis Updated:");
    debug(SHemis);
    debugln();
    if(!SHemis){
      for(int i=0; i<3; i++){
        beep(i*2);
        delay(200);
        
      }
    }else{
      for(int i=3; i>=0; i--){
        beep(i*2);
        delay(200);
      }      
    }
  }
  //Tracking Mode selection if EQ mode enabled
  delay(500);
  debug("Access to Mount..:");
  debugln();
  float baseSecPR=sediPeriod;
  int eqMode=0;
  //Get ExtendedInquire
  char response[100];//20250208
  int retry=0;
  while(retry <60){
    if(SendReceiveMsg(":e1\0", response,1)){
      //Connection is OK
      break;
    }
    retry++;
    delay(1000);

    if(retry>10){
      //Error Beep
      debug("Error: Mount disconnected.. \n");
      while(1){
        //Error
        tone(TONE_PIN,880,500) ;
        //noTone(TONE_PIN);
        delay(500);
      }       
    }
  }
  //Get Extended Inquire
  retry=0;
  while(retry <60){
    if(SendReceiveMsg(":q1010000\0", response,1)){//20250208  :q3010000-> :q1010000
      if((response[2] & 0x08)!=0x00){
        debug("Tracking mode selection....");
        debugln();
        //User's selection
        //ここで赤道儀か経緯台を選ばせる
        debug("Enter EQ Mode Selection");
        int loopc=0;
        while(true){
          if(loopc==0){
              tone(TONE_PIN,880,125) ;
              loopc=40;
          }
          nunchuk->update();
          //sSerial.println(nunchuk->analogX);
          //sSerial.println(nunchuk->analogY);
          
          if(nunchuk->analogX<50){
            baseSecPR=sediPeriod;
            debug("AZ Mode Selected \n");
            soundAZALT();
            SHemis=false;
            break;
          }else if(nunchuk->analogX>206){
            if(nunchuk->stepC==0){
              baseSecPR=sediPeriod;
              eqMode=1;
              debug("EQ Mode Selected \n");
              soundStar();
              break;
            }else{
            baseSecPR=sediPeriod/2;
              eqMode=1;
              debug("0.5 EQ Mode Selected \n");
              soundStar05();
              break;              
            }
          }else if(nunchuk->analogY<50){
            baseSecPR=moonPeriod;
            eqMode=2;
            debug("Luner Mode Selected \n");  
            soundMoon();          
            break;
          }else if(nunchuk->analogY>206){
            baseSecPR=sunPeriod;
            eqMode=3;
            debug("Solar Mode Selected \n");
            soundSun();
            break;
          }
          delay(25);
          loopc--;
        }
        break;
      }else{
        debug("AZ Mode Only... \n");
      }
      break;
    }else{
      //Ignore
      //Old Mount may not accept ExtendenInquire Command
    }
    retry++;
    delay(1000);

    if(retry>10){
      //Ignore
      //Old Mount may not accept ExtendenInquire Command
      break;    
    }
  }

  debug("Initializeing Component.. \n");

  //Anti chattering inputs
  axisX=new Axis(1, eqMode, baseSecPR, SHemis);
  axisY=new Axis(2, eqMode, baseSecPR, SHemis);
  currentSpeedStep=1;
  currentStepX=0;
  currentStepY=0;
  currentStepC=0;
  currentStepZ=0;

  //Stop once
  axisX->SetSpeed(0.0);
  axisY->SetSpeed(0.0);
  debug("Initialized");  

  // ptを初期化
  serialSem=1;
  PT_SEM_INIT(&mutex, 1);
  PT_INIT(&pt1);
  PT_INIT(&pt2);
  PT_INIT(&pt3);
  beep(currentSpeedStep);
}

void loop() {

  threadNunchuk(&pt3);
  threadX(&pt1);
  threadY(&pt2);
    

}
