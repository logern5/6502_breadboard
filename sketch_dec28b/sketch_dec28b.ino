/* Data is LSB to MSB D0 -> D7*/
const char DATA[] = {38, 40, 42, 44, 46, 48, 50, 52};
/* Addr is LSB to MSB A0 -> A4 */
const char ADDR[] = {30, 28, 26, 24, 22};
#define WRITE_ENABLE 2
#define OUT_ENABLE 4
void setup() {
  // put your setup code here, to run once:
  for(int i = 0; i < 5; i++){
    pinMode(ADDR[i], OUTPUT);
  }

  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], OUTPUT);
  }

  pinMode(WRITE_ENABLE, OUTPUT);
  pinMode(OUT_ENABLE, OUTPUT);
  
  Serial.begin(57600);

  digitalWrite(OUT_ENABLE, HIGH);
  digitalWrite(WRITE_ENABLE, HIGH);

  Serial.write("Preparing to write data to RAM\n");
  Serial.write("Writing 0xB9 to addr 16\n");

  byte ad = 16;
  byte dat = 0xB9;

  for (int i = 0; i < 8; i++){
    digitalWrite(DATA[i],((dat >> i) & 1));
  }

  for(int i = 0; i < 5; i++){
    digitalWrite(ADDR[i], ((ad >> i) & 1));
  }

  Serial.write("Data written to addr and data buses\n");
  Serial.write("Enabling /WE \n");
  digitalWrite(WRITE_ENABLE, LOW);
  delay(10);
  Serial.write("Write should be done now\n");
  
  Serial.write("Preparing for read...");
  Serial.write("Disabling /WE\n");
  digitalWrite(WRITE_ENABLE, HIGH);
  Serial.write("Setting databus to input\n");

  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], INPUT);
  }

  byte outb = 0;
  char buf[8];

  Serial.write("Addr bus still should be valid\n");
  Serial.write("Enabling /OE and reading..\n");
  digitalWrite(OUT_ENABLE, LOW);

  for(int i = 0; i < 8; i++){
    byte a = digitalRead(DATA[i]);
    outb += a << i;
  }

  delay(1);

  digitalWrite(OUT_ENABLE, HIGH);
  Serial.write("/OE disabled\n");
  sprintf(buf, "0x%x\n", outb);
  Serial.write("Output:");
  Serial.write(buf);
}

void loop() {
  // put your main code here, to run repeatedly:

}
