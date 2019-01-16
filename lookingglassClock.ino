#include <Stepper.h>

//The amount of time in minutes the clock can be ahead of its setpoint before completing forward revolutions
#define TOLERANCE 20

const int stepsPerRevolution = 2038;

const int stepPin1 = 3;
const int stepPin2 = 4;
const int stepPin3 = 5;
const int stepPin4 = 6;
const int highBitPin = 23;
const int lowBitPin = 24;

const int stepsPerClockRevolution = 720;
//The value '5' in the below expression represents
const int tolerance = stepsPerClockRevolution / (720 / TOLERANCE);
Stepper clockDrive(stepsPerRevolution, stepPin1, stepPin2, stepPin3, stepPin4);

int currentPlacement = 0;
int setPlacement = 0;

int get16bit(int high, int low){
  return high*255 + low;
}

String DMXprint = "DMX Value: ";
String positionPrint = "Position: ";
String setPrint = "Set Position: ";


int DMXval = 0;

//Returns true for forward, false for backward
bool getDirection(int current, int goal) {
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
void locomote(int minutes){
  int movementSteps = stepsPerClockRevolution / (720 / minutes);
  if (getDirection(currentPlacement, setPlacement)) {
    while (movementSteps > 0) {
      if (setPlacement == currentPlacement) {
        break;
      }
      clockDrive.step(1);
      currentPlacement = currentPlacement + movementSteps;
      //Is this spilling over midnight?
      if (currentPlacement > stepsPerClockRevolution) {
        currentPlacement = currentPlacement - stepsPerClockRevolution;
      }
      movementSteps --;
    }
  }
  else {
    while (movementSteps > 0) {
      if (setPlacement == currentPlacement) {
        break;
      }
      clockDrive.step(-1);
      currentPlacement = currentPlacement - movementSteps;
      if (currentPlacement < 0) {
        currentPlacement = stepsPerClockRevolution + currentPlacement;
      }
      movementSteps --;
    }
  }
}

int getDMX(){
  int highBit = pulseIn(highBitPin, HIGH);
  int lowBit = pulseIn(lowBitPin, HIGH);
  highBit = map(highBit, 1000, 2000, 0, 255);
  lowBit = map(lowBit, 1000, 2000, 0, 255);
  int sixteenBit = get16bit(highBit, lowBit);
  if (sixteenBit < 0) {
    sixteenBit = 0;
  }
  if (sixteenBit > 65535) {
    sixteenBit = 65535;
  }
  return sixteenBit;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  clockDrive.setSpeed(6);
  pinMode(highBitPin, INPUT);
  pinMode(lowBitPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  DMXval = getDMX();
  Serial.println(DMXprint + DMXval);
  Serial.println(positionPrint + currentPlacement);
  Serial.println(setPrint + setPlacement);
  setPlacement = map(DMXval, 0, 65535, 0, stepsPerClockRevolution);
  locomote(5);
  
}
