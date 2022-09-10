// MDR-2
// -*- mode: C++ -*-
//
// Copyright (C) 2019 Serhiy Kobyakov
// $Id:  $

#include <AccelStepper.h>

#define microstep 8
#define MAXSPEED 320.0*microstep
#define MAXACCEL 290.0*microstep
const int BackLash = 150*microstep; // Backlash of the grating gear

// The X Stepper pins
#define GRATING_DIR_PIN 3
#define GRATING_STEP_PIN 2
#define L_end 4  // Left endstop
#define R_end 5  // Right endstop
#define Buzzer 6

String gotData = "";

//unsigned long t0;
//unsigned long t1;

// Define the stepper and the pins it will use
AccelStepper grating(AccelStepper::DRIVER, GRATING_STEP_PIN, GRATING_DIR_PIN);


void setup()
{   
  digitalWrite(Buzzer, HIGH);
  grating.setMaxSpeed(MAXSPEED);
  grating.setAcceleration(MAXACCEL);

// alarm if grating hit endstops
  if (digitalRead(R_end) || digitalRead(L_end)) HitEndstop();  
   
// some other init stuff here
  analogReference(DEFAULT);
  pinMode(A0, INPUT);
  pinMode(A1, OUTPUT);
  digitalWrite(A1, LOW);
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);
  pinMode(A4, OUTPUT);
  digitalWrite(A4, LOW);
  pinMode(A5, OUTPUT);
  digitalWrite(A5, LOW);

// start serial communication
  Serial.begin(115200);
    
// alarm if grating hit endstops
  if (digitalRead(R_end) || digitalRead(L_end)) HitEndstop();
    
// setup done, sound and go!    
  delay(30);
  digitalWrite(Buzzer, LOW);
}

void beepn(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(Buzzer, HIGH);
    delay(100);
    digitalWrite(Buzzer, LOW);
    delay(100);
  }
}

void beepE(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(Buzzer, HIGH);
    delay(400);
    digitalWrite(Buzzer, LOW);
    delay(400);
  }
}

void HitEndstop() {
  grating.stop();
  grating.setSpeed(0);
  grating.setCurrentPosition(0);
  grating.moveTo(0);
  beepE(2);
  if (Serial.available() > 0) {
    Serial.readStringUntil('\n');
    if (digitalRead(R_end)) Serial.println("R");
    if (digitalRead(L_end)) Serial.println("L");
  }
}
/*
void goTo() {
  delay(5);
  if(Serial.available() > 0) {
    gotData = Serial.readStringUntil('\n');
    long pos = gotData.toInt();
    if (pos != grating.currentPosition()) {
      if (pos >= grating.currentPosition()) {
        grating.moveTo(pos);
        while (grating.distanceToGo() != 0) {
          if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }
          grating.run();
        }
      }
      else {
        grating.moveTo(pos - BackLash);
        while (grating.distanceToGo() != 0) {
          if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }
          grating.run();
        }
        delay(500);
        grating.moveTo(pos);
        while (grating.distanceToGo() != 0) {
          if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }
          grating.run();
        }
      }
    }
    Serial.println(grating.currentPosition());    
  } 
}
*/

void goTo() {
  if (Serial.available() > 0) {
    gotData = Serial.readStringUntil('\n');
    long pos = gotData.toInt();
    if (pos != grating.currentPosition()) {
      if (pos <= grating.currentPosition()) {
      // if we have to go backwards - go BackLash further
        grating.moveTo(pos - BackLash);
        while (grating.distanceToGo() != 0) {
          if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }            
          grating.run(); }
        delay(500);
      }  // end of go backwards
      grating.moveTo(pos);
      while (grating.distanceToGo() != 0) {
        if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }
        grating.run(); }
    } // end of pos != current position
  } // end of Serial.available() > 0
}


void parkGrating() {
  if (grating.currentPosition() != 0) { 
    grating.moveTo(0 - BackLash);
    while (grating.distanceToGo() != 0) {
      if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }
      grating.run();
    }
    delay(500);
    grating.moveTo(0);
    while (grating.distanceToGo() != 0) {
      if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }
      grating.run();
    } 
  }
//  Serial.println(grating.currentPosition());
}

word getRead() {
  word theRead = 0;
  byte sample_count = 0;
  while (sample_count < 64) {   //64
    theRead += analogRead(A0);
    sample_count++;
    delay(3); // TRY LONGER DELAY! 10?
  }
  return theRead;
} 

void initMDR(){
  word signRead = 0;
  word signMax = 0;
  long thePos = 0;
  
  grating.setMaxSpeed(MAXSPEED/6);
  grating.setAcceleration(MAXACCEL/6);
  grating.moveTo(-100000);
  while (grating.distanceToGo() != 0) {
    if (digitalRead(L_end)) grating.setCurrentPosition(0);            
    grating.run();
  }
  
  delay(600);
  grating.setMaxSpeed(MAXSPEED);
  grating.setAcceleration(MAXACCEL);  
  grating.moveTo(13000);
  while (grating.distanceToGo() != 0) {
//    if (digitalRead(R_end) || digitalRead(L_end)) HitEndstop();            
    grating.run();
  }
  grating.setCurrentPosition(0);
//  Serial.print("11111 ");
//  Serial.println(15000);  
  
/*  
  delay(600);
// едем в начало сканирования диапазона зеркального отражения  
  grating.moveTo(3300);
  while (grating.distanceToGo() != 0) {
//    if (digitalRead(R_end) || digitalRead(L_end)) HitEndstop();            
    grating.run();
  }

  grating.setCurrentPosition(0);

// сканируем до конца диапазона зеркального отражения
  while (grating.currentPosition() < 100) {
    grating.moveTo(grating.currentPosition() + 1);
    while (grating.distanceToGo() != 0) {
      grating.run();
    }
    delay(50);
    Serial.print(grating.currentPosition());
    signRead = getRead();
      Serial.print(" ");
      Serial.println(signRead);
    if (signRead > signMax) {
      signMax = signRead;
      thePos = grating.currentPosition();
    }
  }

  grating.setMaxSpeed(MAXSPEED);
  grating.setAcceleration(MAXACCEL);

  if (signMax < 1000) {
//    signMax = 0;
    digitalWrite(Buzzer, HIGH);
    delay(4000);
    digitalWrite(Buzzer, LOW);
  }
  else {
    // go to position "0"
    grating.moveTo(thePos + 1200*microstep); // 1200 is the distance from
                                   //  grating mirror reflection to "0" position
    while (grating.distanceToGo() != 0) {
      if (digitalRead(R_end) || digitalRead(L_end)) { HitEndstop(); break; }
      grating.run();
    }
    grating.setCurrentPosition(0);
    beepn(4);
  }
    Serial.print("11111 ");
    Serial.println(signMax);
*/    
}

void loop() {  
  if (digitalRead(R_end) || digitalRead(L_end)) HitEndstop();
  
  if (Serial.available() > 0 ) { 
    char data_in = (char) Serial.read();
    switch (data_in) {
// identification
      case '?':
          delay(4);
          Serial.println("MDR-2");
          break;

// go to position
      case 'g':
          delay(4);
          goTo();
          Serial.println(grating.currentPosition());
          break;

// initialization
      case 'i':
          delay(4);
          initMDR();
          Serial.println(grating.currentPosition());          
          break;
          
// get position
      case 'p':
          delay(4);
          Serial.println(grating.currentPosition());
          break;

// set position
      case 's':
        delay(4);
        gotData = Serial.readStringUntil('\n');
        grating.setCurrentPosition(gotData.toInt());
        Serial.println(grating.currentPosition());
        break;

// park grating
      case 'z':
          parkGrating();
          beepn(5);
          Serial.println(grating.currentPosition());
          break;

      default:
          break;           
    }
  }
}
