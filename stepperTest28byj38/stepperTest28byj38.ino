#include <Stepper.h>

#define STEPS 220

Stepper stepper(STEPS, 3, 4, 5, 6);

void setup() {
  // put your setup code here, to run once:
  stepper.setSpeed(50);
}

void loop() {
  // put your main code here, to run repeatedly:
  stepper.step(200);
  delay(1000);
  stepper.step(-200);
  delay(1000);
}
