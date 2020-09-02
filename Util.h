#ifndef __UTIL_H__
#define __UTIL_H__
//main methods
#include <SoftwareSerial.h>

#define TONE_PIN 3

#define debugOn 0

void debug(int n){
  if(debugOn==0)return;
  //Serial.print(n);
  sSerial.print(n);
}
void debug(char* c){
  if(debugOn==0)return;
  //Serial.print(c);
  sSerial.print(c);
}
void debug(double n){
  if(debugOn==0)return;
  char *numStr;
  sprintf(numStr, "%lf", n);
  //Serial.print(n);
  sSerial.print(numStr);
}
void debug(long int n){
  if(debugOn==0)return;
  char *numStr;
  sprintf(numStr, "%ld", n);
  //Serial.print(n);
  sSerial.print(numStr);
}
void debugln(){
  if(debugOn==0)return;
  //Serial.println();
  sSerial.println("");
}





int getStep(int  n){
    int s = (int )floor((double)(n-28)/40)-2;
    return (s<-2 || s>2)?99:s;
}

void beep(int currentSpeedStep){
  int freq=440;
  switch (currentSpeedStep){
    case 0:
      freq=262;
      break;
    case 1:
      freq=294;
      break;
    case 2:
      freq=330;
      break;
    case 3:
      freq=349;
      break;
    case 4:
      freq=392;
      break;
    case 5:
      freq=440;
      break;
    case 6:
      freq=494;
      break;
    case 7:
      freq=523;
      break;
    case 8:
      freq=587;
      break;
    case 9:
      freq=659;
      break;
    case 10:
      freq=698;
      break;
    default:
      freq=880;
      break;
  }
  int r = (currentSpeedStep % 3) + 1;
  //for(int i=0; i<r; i++){
    tone(TONE_PIN,freq,50) ;
    delay(100);
  //}
}

void soundStar(){
/*
-so  196
so  329
fa  349
mi  330
do#  277
re  294
la---  440
*/
  tone(TONE_PIN,196,250) ;
  delay(250);
  tone(TONE_PIN,392,250) ;
  delay(250);
  tone(TONE_PIN,349,250) ;
  delay(250);
  tone(TONE_PIN,330,250) ;
  delay(250);
  tone(TONE_PIN,277,250) ;
  delay(250);
  tone(TONE_PIN,294,250) ;
  delay(250);
  tone(TONE_PIN,440,500) ;
  delay(1000);
}
void soundStar05(){
/*
-so  98
so  196
fa  175
mi  165
do#  139
re  147
la---  220
*/
  tone(TONE_PIN,98,250) ;
  delay(250);
  tone(TONE_PIN,196,250) ;
  delay(250);
  tone(TONE_PIN,175,250) ;
  delay(250);
  tone(TONE_PIN,165,250) ;
  delay(250);
  tone(TONE_PIN,139,250) ;
  delay(250);
  tone(TONE_PIN,147,250) ;
  delay(250);
  tone(TONE_PIN,220,500) ;
  delay(1000);
}
void soundSun(){
/*
-so  -196
do  262 
re  294
mi-- 330
mi-- 330
*/  
  tone(TONE_PIN,196,250) ;
  delay(250);
  tone(TONE_PIN,262,250) ;
  delay(250);
  tone(TONE_PIN,294,250) ;
  delay(250);
  tone(TONE_PIN,330,250) ;
  delay(500);
  tone(TONE_PIN,330,500) ;
  delay(1000);
  
}
void soundMoon(){
/*
+do--  523
si.  494
la  440
so  392
fa---  349
*/
  tone(TONE_PIN,523,600) ;
  delay(600);
  tone(TONE_PIN,494,200) ;
  delay(200);
  tone(TONE_PIN,440,200) ;
  delay(200);
  tone(TONE_PIN,392,200) ;
  delay(200);
  tone(TONE_PIN,349,700) ;
  delay(900);
 
}
void soundAZALT(){
  tone(TONE_PIN,262,125) ;
  delay(125);
  tone(TONE_PIN,294,125) ;
  delay(125);
  tone(TONE_PIN,330,125) ;
  delay(125);
  tone(TONE_PIN,349,125) ;
  delay(125);
  tone(TONE_PIN,392,125) ;
  delay(500);
  
}

void error(){
  while(1){
    digitalWrite(LED, HIGH);
    tone(TONE_PIN,880) ;
    delay(250);
    
    digitalWrite(LED, LOW);
    noTone(TONE_PIN);
    delay(250);
  }
}
 
#endif
