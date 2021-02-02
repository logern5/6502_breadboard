/* LSB to MSB 24..36 --> D0..D7 */
#define CLOCK 2
const char DATA[] = {22, 24, 26, 28, 30, 32, 34, 36};
void setup() {
  // put your setup code here, to run once:
  /* Set up the pins */
  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], INPUT);
  }
  pinMode(CLOCK, OUTPUT);
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly
  digitalWrite(CLOCK, HIGH);
  /* Read from the pins */
  byte dat = 0;
  char buf[16];
  for(int i = 0; i < 8; i++){
    dat += digitalRead(DATA[i]) << i;
  }
  sprintf(buf,"Data:0x%x\n",dat);
  Serial.write(buf);
  delay(100);
  digitalWrite(CLOCK, LOW);
  delay(100);
}
