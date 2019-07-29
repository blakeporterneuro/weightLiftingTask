#include <ADCTouch.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// MotorShiled setup
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(2);

// Pin outs
const int button = 13; // button for manual dispensing of reward
const int magnet30 = 1; //  the Reed switch at 30cm up
const int magnet0 = 2; // the Reed switch at base
const int LED = 0;
const int TTLPinUp = 4;
const int TTLPin30 = 3;
const int TTLPinDown = 6;

const int TTLPinLick = 5;

// Motor is in M2

// User inputs
const int LEDdelay = 250; // time for LED and pump on
const int motorSpeed = 250; // speed of sucrose pump
const int motorDuration = 0; // additional time for motor on

// Initalize
bool currButtState = true; // button pressed
bool prevButtState = true;

bool currMag30 = true; // the default state of currmag30 is true because if the magnet is not there the REED switches are normally on/HIGH/true
bool prevMag30 = true;

bool currMag0 = false; // the default state of currmag0 should be false because if rat is not lifting weight the magnet will be at the bottom and the magnet will be false
bool prevMag0 = false;

bool trialState = true; // the weight must be lowered back down in order for a new trial to be allowed

int lickRef; //reference values to remove offset


void setup() {
  //  Serial.begin(9600);
  AFMS.begin();
  myMotor->setSpeed(motorSpeed);

  pinMode(button, INPUT_PULLUP); // always input pullup so voltage doesnt float away
  pinMode(magnet30, INPUT_PULLUP);
  pinMode(magnet0, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(TTLPinUp, OUTPUT);
  pinMode(TTLPin30, OUTPUT);
  pinMode(TTLPinDown, OUTPUT);
  pinMode(TTLPinLick, OUTPUT);

  //  for debugging
  //  Serial.println(prevButtState);
  //  Serial.println(currButtState);

  // set initial states
  digitalWrite(LED, LOW);
  digitalWrite(TTLPinUp, LOW);
  digitalWrite(TTLPin30, LOW);
  digitalWrite(TTLPinLick, LOW);

  myMotor->run(RELEASE);

  lickRef = ADCTouch.read(A0, 500); //create reference values of capactive touch
}

void loop() {
  // For debugging
  //  Serial.println("    Prev:");
  //  Serial.println(prevButtState);
  //  Serial.println("    Curr:");
  //  Serial.println(currButtState);

  // Check state of button press and base magnet
  currButtState = digitalRead(button);
  currMag0 = digitalRead(magnet0);

  // Check lick sensor
  int lickValue = ADCTouch.read(A0);

  lickValue -= lickRef; // relative to baseline

  // if lick detected
  if (lickValue > 50) { // 75 and 100 to high, 50 is good but seems low, 60 good too. 10 for epoxy cover
    digitalWrite(TTLPinLick, HIGH);
  } else {
    digitalWrite(TTLPinLick, LOW);
  }


  // If button is pressed, turn on LED and turn on sucrose
  if (currButtState == true && prevButtState == false) {
    //    Serial.println("Button Press");
    digitalWrite(LED , HIGH);
    myMotor->run(FORWARD);
    digitalWrite(TTLPin30, HIGH);
    delay(LEDdelay);
    digitalWrite(TTLPin30, LOW);
    digitalWrite(LED, LOW);
    delay(motorDuration);
    myMotor->run(RELEASE);
  }

  // check if magnet leaves base
  if (currMag0 == true && prevMag0 == false) {
    digitalWrite(TTLPinDown, HIGH);
    delay(100);
    digitalWrite(TTLPinDown, LOW);
  }

// check to make sure the magnet has gone back down before starting a new trial  if (currMag0 == false && prevMag0 == true) {
    digitalWrite(TTLPinUp, HIGH);
    trialState = true;
    delay(100);
    digitalWrite(TTLPinUp, LOW);
  }

  // if the weight has left the base
  if (currMag0 == true && trialState == true) {
    currMag30 = digitalRead(magnet30); // check the magnet at 30cm

    if (currMag30 == true && prevMag30 == false) { // once the magnet reaches 30cm
      digitalWrite(TTLPin30, HIGH);
      digitalWrite(LED , HIGH);
      myMotor->run(FORWARD);
      delay(LEDdelay);
      digitalWrite(TTLPin30, LOW);
      digitalWrite(LED, LOW);
      delay(motorDuration);
      myMotor->run(RELEASE);
      trialState = false;
    }
  }

  // keep track of previous states
  prevMag0 = currMag0;
  prevMag30 = currMag30;
  prevButtState = currButtState;

}
