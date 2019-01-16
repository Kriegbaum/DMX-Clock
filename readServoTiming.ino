char pinHigh = 23;
char pinLow = 22;
String highString = "High bit: ";
String lowString = "Low bit: ";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);
  pinMode(pinHigh, INPUT);
  pinMode(pinLow, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(highString + pulseIn(pinHigh, HIGH));
  Serial.println(lowString + pulseIn(pinLow, HIGH));
  delay(1000);
}
