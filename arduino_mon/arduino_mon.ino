#define CLOCK 2
#define READ_WRITE 7
//const char ADDR[] = {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52};
const char DATA[] = {39, 41, 43, 45, 47, 49, 51, 53};
const char ADDR[] = {52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24, 22};
//const char DATA[] = {53, 51, 49, 47, 45, 43, 41, 39};
void setup() {
  //pinMode(9, OUTPUT);
  //digitalWrite(9, HIGH);
  // put your setup code here, to run once:
  for(int i = 0; i < 16; i++){
    pinMode(ADDR[i], INPUT);
  }
  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], INPUT);
  }
  pinMode(CLOCK, OUTPUT); /* We control the clock with the Arduino */
  pinMode(READ_WRITE, INPUT);
  Serial.begin(57600);
}

void onClock(){
char output[15];

  unsigned int address = 0;
  for (int n = 0; n < 16; n += 1) {
    int bit = digitalRead(ADDR[n]) ? 1 : 0;
    Serial.print(bit);
    address = (address << 1) + bit;
  }
  
  Serial.print("   ");
  
  unsigned int data = 0;
  for (int n = 0; n < 8; n += 1) {
    int bit = digitalRead(DATA[n]) ? 1 : 0;
    Serial.print(bit);
    data = (data << 1) + bit;
  }

  sprintf(output, "   %04x  %c %02x", address, digitalRead(READ_WRITE) ? 'r' : 'W', data);
  Serial.println(output);  
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(CLOCK, HIGH);
  /* Clock is high, so this is where we do stuff */
  delay(1);
  onClock();
  delay(200);
  digitalWrite(CLOCK, LOW);
  delay(1);
}
