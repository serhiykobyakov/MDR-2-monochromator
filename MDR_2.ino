// MDR-2
// Version 10.09.2022
//
// Copyright (C) Serhiy Kobyakov


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
    grating.run();
  }
  grating.setCurrentPosition(0);
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
