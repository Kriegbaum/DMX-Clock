#include <Stepper.h>

//Comment this out to remove debug statements
//#define DEBUG

//The amount of time in minutes the clock can be ahead of its setpoint before completing forward revolutions
#define TOLERANCE 60

//Enable and disable debug printing. Timing will improve if debug is disabled, comment out line above to disable
#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
#endif

//Used to filter out noise in the servo pulse reception
int prevPulseUpper = 0;
int prevPulseLower = 0;
const int noiseReduction = 2;

//This is just used to help the stepper library calculate the rotation speed
const int stepsPerRevolution = 513;

//Pins for the stepper motor
const int stepPin1 = 3;
const int stepPin2 = 4;
const int stepPin3 = 5;
const int stepPin4 = 6;
const int sleepPin = 21;

//Pins for the RC4 input
const int highBitPin = 23;
const int lowBitPin = 22;

//Test what the servo controller actually puts out (may not between 1000 and 2000) and input its upper and lower bounds
const int servoPulseLower = 960;
const int servoPulseUpper = 2082;

bool clockDirection = true;

const int stepsPerClockRevolution = 5488;
//const int stepsPerClockRevolution = 21930;
const int tolerance = stepsPerClockRevolution / (720 / TOLERANCE);
//Actual stepper object
Stepper clockDrive(stepsPerRevolution, stepPin1, stepPin2, stepPin3, stepPin4);

//Step tracking objects
int currentPlacement = 0;
int setPlacement = 0;

int get16bit(int high, int low){
  //takes a high and low order 8 bit integer and combines them into a 16 bit integer
  return high*256 + low;
}

//Used in various print statements, must be initialized first or else strange behavior
//COMMENT THESE OUT AT FINAL PRODUCTON
String DMXprintHigh = "DMX high: ";
String DMXprintLow = "Dmx low: ";
String DMXprint = "DMX 16 bit: ";
String positionPrint = "Position: ";
String setPrint = "Set Position: ";

//Update this every cycle with the calculated 16 bit DMX value
int DMXval = 0;

//Returns true for forward, false for backward
bool getDirection(int current, int goal) {
  //Shift all values up so we don't have to monkey around with cycling through midnight
  int goalAdjusted = goal + tolerance + 1;
  int currentAdjusted = current + tolerance + 1;
  //If goal is ahead of current, just move forward
  if (goalAdjusted > currentAdjusted) {
    DEBUG_PRINT("Direction: Forward");
    if (!clockDirection) {
      delay(150);
    }
    clockDirection = true;
    return true;
  }
  //Are we behind, but within tolerance?
  else if (goalAdjusted + tolerance > currentAdjusted) {
    //You're in luck, clear to move backward
    DEBUG_PRINT("Direction: Backward");
    if (clockDirection) {
      delay(150);
    }
    clockDirection = false;
    return false;
  }
 else {
  //Outta luck kiddo, gotta let the clock move a full half-day forward
  DEBUG_PRINT("Direction: Forward OVER-ROTATE");
  if(!clockDirection) {
    delay(150);
  }
  clockDirection = true;
  return true;
 }
}

//Moves the clock forward or backward a given amount of time in minutes
void locomote(int movementSteps){
  if (getDirection(currentPlacement, setPlacement)) {
    DEBUG_PRINT("MOVING FORWARD");
    while (movementSteps > 0) {
      if (setPlacement == currentPlacement) {
        break;
      }
      clockDrive.step(-1);
      currentPlacement ++;
      movementSteps --;
      //Is this spilling over midnight?
      if (currentPlacement > stepsPerClockRevolution) {
        currentPlacement = 0;
      }
    }
  }
  else {
    DEBUG_PRINT("MOVING BACKWARD");
    while (movementSteps > 0) {
      if (setPlacement == currentPlacement) {
        break;
      }
        clockDrive.step(1);
        currentPlacement --;
        movementSteps --;
        if (currentPlacement < 0) {
          currentPlacement = stepsPerClockRevolution;
      }
    }
  }
}

int getDMX(){
  int highBit = pulseIn(highBitPin, HIGH);
  int lowBit = pulseIn(lowBitPin, HIGH);
  
  if (abs(highBit - prevPulseUpper) < noiseReduction) {
    highBit = prevPulseUpper;
  }
  else{
    prevPulseUpper = highBit; 
  }
  if (abs(lowBit - prevPulseLower) < noiseReduction) {
    lowBit = prevPulseLower;
  }
  else {
    prevPulseLower = lowBit;
  }
    
  highBit = map(highBit, servoPulseLower, servoPulseUpper, 0, 255);
  highBit = constrain(highBit, 0, 255);
  DEBUG_PRINT(DMXprintHigh + highBit);
  lowBit = map(lowBit, servoPulseLower, servoPulseUpper, 0, 255);
  lowBit = constrain(lowBit, 0, 255);
  DEBUG_PRINT(DMXprintLow + lowBit);
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
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  //Drive this pin high to activate motor driver
  pinMode(sleepPin, OUTPUT);
  clockDrive.setSpeed(25);
  pinMode(highBitPin, INPUT);
  pinMode(lowBitPin, INPUT);
  //This is the pin for our homing switch
  pinMode(17, INPUT_PULLUP);
  DEBUG_PRINT("**********************************************************");
  DEBUG_PRINT(tolerance);
}

void loop() {
  DMXval = getDMX();
  setPlacement = map(DMXval, 0, 65535, 0, stepsPerClockRevolution);

  DEBUG_PRINT(DMXprint + DMXval);
  DEBUG_PRINT(positionPrint + currentPlacement);
  DEBUG_PRINT(setPrint + setPlacement);
  
  if (setPlacement != currentPlacement) {
    digitalWrite(sleepPin, HIGH);
    delay(10);
    locomote(20);  
  }
  else {
    digitalWrite(sleepPin, LOW);
  }
  
  //Sets current position to 0 (Home) if homing switch is pressed
  if (!(digitalRead(17))) {
    digitalWrite(sleepPin, HIGH);
    delay(50);
    currentPlacement = 0;
    clockDrive.step(17);
    delay(100);
  }
}
