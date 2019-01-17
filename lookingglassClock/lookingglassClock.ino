#include <Stepper.h>

//The amount of time in minutes the clock can be ahead of its setpoint before completing forward revolutions
#define TOLERANCE 60

//This is just used to help the stepper library calculate the rotation speed
const int stepsPerRevolution = 200;

//Pins for the stepper motor
const int stepPin1 = 3;
const int stepPin2 = 4;
const int stepPin3 = 5;
const int stepPin4 = 6;
//Pins for the RC4 input
const int highBitPin = 23;
const int lowBitPin = 22;


const int stepsPerClockRevolution = 720;
const int tolerance = stepsPerClockRevolution / (720 / TOLERANCE);
//Actual stepper object
Stepper clockDrive(stepsPerRevolution, stepPin1, stepPin2, stepPin3, stepPin4);

//Step tracking objects
int currentPlacement = 0;
int setPlacement = 0;

int get16bit(int high, int low){
  //takes a high and low order 8 bit integer and combines them into a 16 bit integer
  return high*255 + low;
}

//Used in various print statements, must be initialized first or else strange behavior
//COMMENT THESE OUT AT FINAL PRODUCTON
String DMXprintHigh = "DMX high: ";
String DMXprintLow = "Dmx low: ";
String DMXprint = "DMX 16 bit: ";
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
    Serial.println("Direction: Forward");
    return true;
  }
  //Are we behind, but within tolerance?
  else if (goalAdjusted + tolerance > currentAdjusted) {
    //You're in luck, clear to move backward
    Serial.println("Direction: Backward");
    return false;
  }
 else {
  //Outta luck kiddo, gotta let the clock move a full half-day forward
  Serial.println("Direction: Forward");
  return true;
 }
}

//Moves the clock forward or backward a given amount of time in minutes
void locomote(int minutes){
  int movementSteps = stepsPerClockRevolution / (720 / minutes);
  if (getDirection(currentPlacement, setPlacement)) {
    Serial.println("MOVING FORWARD");
    while (movementSteps > 0) {
      if (setPlacement == currentPlacement) {
        break;
      }
      clockDrive.step(1);
      currentPlacement ++;
      movementSteps --;
      //Is this spilling over midnight?
      if (currentPlacement > stepsPerClockRevolution) {
        currentPlacement = currentPlacement - stepsPerClockRevolution;
      }
    }
  }
  else {
    Serial.println("MOVING BACKWARD");
    while (movementSteps > 0) {
      if (setPlacement == currentPlacement) {
        break;
      }
      clockDrive.step(-1);
      currentPlacement --;
      movementSteps --;
      if (currentPlacement < 0) {
        currentPlacement = stepsPerClockRevolution + currentPlacement;
      }
    }
  }
}

int getDMX(){
  int highBit = pulseIn(highBitPin, HIGH);
  int lowBit = pulseIn(lowBitPin, HIGH);
  highBit = map(highBit, 1000, 2000, 0, 255);
  highBit = constrain(highBit, 0, 255);
  Serial.println(DMXprintHigh + highBit);
  lowBit = map(lowBit, 1000, 2000, 0, 255);
  lowBit = constrain(lowBit, 0, 255);
  Serial.println(DMXprintLow + lowBit);
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
  clockDrive.setSpeed(50);
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
  locomote(50);
  
}
