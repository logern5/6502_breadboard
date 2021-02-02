#define CLOCK 2

void setup() {
  // put your setup code here, to run once:
  pinMode(CLOCK, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(CLOCK, HIGH);
  delay(50);
  digitalWrite(CLOCK, LOW);
  delay(50);
}
