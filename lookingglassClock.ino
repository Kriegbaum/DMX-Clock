#include <Stepper.h>

//The amount of time in minutes the clock can be ahead of its setpoint before completing forward revolutions
#define TOLERANCE 5

const int stepsPerRevolution = 2038;

const int stepPin1 = 3;
const int stepPin2 = 4;
const int stepPin3 = 5;
const int stepPin4 = 6;
const int highBitPin = 23;
const int lowBitPin = 24;

const int stepsPerClockRevolution = 4076;
//The value '5' in the below expression represents
const int tolerance = stepsPerClockRevolution / (720 / TOLERANCE);
Stepper clockDrive(stepsPerRevolution, pin1, pin2, pin3, pin4);

int currentPlacement = 0;
int setPlacement = 0;

int get16bit(high, low){
  return high*255 + low;
}


int DMXval = 0;

//Returns true for forward, false for backward
bool getDirection(current, goal) {
  //Shift all values up so we don't have to monkey around with cycling through midnight
  int goalAdjusted = goal + tolerance;
  int currentAdjusted = current + tolerance;
  //If goal is ahead of current, just move forward
  if (goalAdjusted > currentAdjusted) {
    return true;
  }
  //Are we behind, but within tolerance?
  else if (goalAdjusted + tolerance > currentAdjusted) {
    //You're in luck, clear to move backward
    return false;
  }
 else {
  //Outta luck kiddo, gotta let the clock move a full half-day forward
  return true;
 }
}

//Moves the clock forward or backward a given amount of time in minutes
void locomote(minutes){
  int movementSteps = stepsPerClockRevolution / (720 / minutes);
  if getDirection(currentPlacement, setPlacement) {
    clockDrive.step(movementSteps);
    currentPlacement = currentPlacement + movementSteps;
    //Is this spilling over midnight?
    if (currentPlacement > stepsPerClockRevolution) {
      currentPlacement = currentPlacement - stepsPerClockRevolution);
    }
  }
  else {
    clockDrive.step(movementSteps * -1);
    currentPlacement = currentPlacement - movementSteps;
    if (currentPlacement < 0) {
      currentPlacement = stepsPerClockRevolution + currentPlacement;
    }
  }
}

int getDMX(){
  int highBit = pulseIn(highBitPin, HIGH);
  int lowBit = pulseIn(lowBitPin, HIGH);
  highBit = map(highBit, 1000, 2000, 0, 255);
  lowBit = map(lowBit, 1000, 2000, 0, 255);
  return get16bit(highBit, lowBit);
}

void setup() {
  // put your setup code here, to run once:
  clockDrive.setSpeed(6);
  pinMode(highBitPin, INPUT);
  pinMode(lowBitPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  DMXval = getDMX();
  setPlacement = map(DMXval, 0, 65535, 0, stepsPerClockRevolution);
  
}
