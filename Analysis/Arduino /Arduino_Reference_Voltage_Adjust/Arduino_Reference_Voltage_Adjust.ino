#define SUPPLY_VOLTAGE 8

void setup() {
  Serial.begin(38400);
  pinMode(SUPPLY_VOLTAGE, OUTPUT);
  digitalWrite(SUPPLY_VOLTAGE, HIGH); 
  Serial.println("Supply is on");
}

void loop() {
}
