/* Control pins */
#define CLOCK 2
#define READ_WRITE 4
#define RST 6
/* How long to delay the PHI2 clock */
#define DEL 500

/* LSB to MSB 24..36 --> D0..D7 */
const char DATA[] = {22, 24, 26, 28, 30, 32, 34, 36};
/* LSB to MSB A0..A2 --> 44..48 */
const char ADDR[] = {44, 46, 48, 50};
/* We are only using the first 3 bits (2^3=8 bytes) of the addr bus */
/* Program must always be 8 bytes long because of our address bus */
//const byte PROG[] = {0xA9, 0xFF, 0x8d, 0x02, 0x60, 0xEA, 0xEA, 0xEA};
//const byte PROG[] = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0XEA, 0xEA, 0xEA};
//const byte PROG[] = {0xEA, 0xA9, 0xBF, 0x85, 0xDA, 0xEA, 0xEA, 0xEA};
const byte PROG[] = {0xEA, 0x85, 0xDA, 0x85, 0xDA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA};
const char *mds[2];


void tickClock(int count){
  while(count--){
    digitalWrite(CLOCK, HIGH);
    digitalWrite(CLOCK, LOW);
  }
}

void set_input(){
  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], INPUT);
  }
}

void set_output(){
  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], OUTPUT);
  }
}

void db_write(char dat){
  //set_output();
  for(int i = 0; i < 8; i++){
    digitalWrite(DATA[i], (dat >> i) & 1);
  }
  delay(1);
}

char db_read(){
  byte out_byte = 0x00;
  set_input();
  for(int i = 0; i < 8; i++){
    out_byte += digitalRead(DATA[i]) << i;
  }
  return out_byte;
}


char ad_read(){
  byte out_byte = 0x00;
  set_input();
  for(int i = 0; i < 4; i++){
    out_byte += digitalRead(ADDR[i]) << i;
  }
  return out_byte;
}

void setup() {
  // put your setup code here, to run once:
  /* Start the serial port so we can see what is going on */
  Serial.begin(57600);
  delay(2000);
  /* Set up RW modes table for serial printing */
  mds[0] = "Write";
  mds[1] = "Read";
  /* We will start out using data as output */
  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], OUTPUT);
  }
  /* Address pins will ALWAYS be input */
  for(int i = 0; i < 3; i++){
    pinMode(ADDR[i], INPUT);
  }
  /* Set mode of control pins */
  pinMode(CLOCK, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(READ_WRITE, INPUT);
  /* Reset the MPU, set a reset vector */
  /* The MPU needs to be held in reset for a minimum of 2 clock cycles */
  Serial.write("\nStarting low reset\n");
  db_write(0x00);
  digitalWrite(RST, LOW);
  tickClock(7);
  delay(1);
  digitalWrite(RST, HIGH);
  Serial.write("Low reset done\n");
  db_write(0xEA);
  tickClock(4); /* This needs to be done 6 time for some reason */
  for(int i = 0; i < 20; i++){
  char buf[64];
  byte ad = 0xFF;
  ad = ad_read();
  sprintf(buf, "AD: 0x%x\n", ad);
  Serial.write(buf);
  tickClock(1);
  }
  db_write(0x00);
  tickClock(1); /* The third operation is needed to go from addr 0x5 to 0x0 */
  /* Now, the reset operation is done, going into the loop! */
}

void loop() {
  delay(DEL);
  digitalWrite(CLOCK, LOW);
  /* Pni2 is low */
  char buf[64];
  byte rw = 0x0;
  byte ad = 0x0;
  rw = digitalRead(READ_WRITE);
  ad = ad_read();
  //ad = digitalRead(ADDR[0]);
  sprintf(buf, "RW: %d, Ad: 0x%x, next instr: 0x%x\n",rw,ad,PROG[ad]);
  set_output();
  if(rw == 1){
    db_write(0xEA);
  }
  if(rw == 0){
    set_input();
  }
  delay(DEL);
  /* Phi2 is high (CPU does mem access) */
  Serial.write(buf);
  if(rw == 0){
    byte w_byte = 0x00;
    Serial.write("****6502 writing...\n");
    w_byte = db_read();
    sprintf(buf, "W: 0x%x\n", w_byte);
    Serial.write(buf);
  }
  digitalWrite(CLOCK, HIGH);
}
