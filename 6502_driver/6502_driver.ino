/* Control pins */
#define CLOCK 2
#define READ_WRITE 4
#define RST 6
/* How long to delay the PHI2 clock */
#define DEL 500

/* LSB to MSB 24..36 --> D0..D7 */
const char DATA[] = {22, 24, 26, 28, 30, 32, 34, 36};
/* LSB to MSB A0..A2 --> 44..48 */
const char ADDR[] = {44, 46, 48};
/* We are only using the first 3 bits (2^3=8 bytes) of the addr bus */
/* Program must always be 8 bytes long because of our address bus */
//const byte PROG[] = {0xA9, 0xFF, 0x8d, 0x02, 0x60, 0xEA, 0xEA, 0xEA};
const byte PROG[] = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0XEA, 0xEA, 0xEA};
const char *mds[2];


void tickClock(int count){
  while(count--){
    digitalWrite(CLOCK, HIGH);
    //delay(200);
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
  digitalWrite(RST, LOW);
  tickClock(7);
  digitalWrite(RST, HIGH);
  Serial.write("Low reset done\n");
  /* Now, the first 5 cycles of the reset interrupt are internal operations, the last 2 read the vector */
  /* We are going to jump to address 0x5555 */
  Serial.write("Writing 0xEA * 6 and 0x55 * 2\n");
  db_write(0xEA);
  tickClock(6); /* This needs to be done 6 time for some reason */
  db_write(0xEA);
  tickClock(2);
  /* Now, the reset operation is done, going into the loop! */
  
}

void loop() {
  /* We start on PHI2 low (PHI1)*/
  /* Write instruction to jmp to 0xAAAA (last 4 1010)*/
  /* JMP $AAAA = 4C AA AA */
  Serial.write("***Writing 0x4C\n");
  db_write(0x4C);
  tickClock(1);
  Serial.write("Writing 0xAA\n");
  db_write(0xAA);
  tickClock(2);
  Serial.write("Writes jmp 1 done\n");
  Serial.write("Reading from Addr bus!***\n");
  byte out_b = 0x00;
  char buf[16];
  for(int i = 0; i < 3; i++){
    out_b += (digitalRead(ADDR[i]) << i);
  }
  sprintf(buf,"A:0x%x,RW:%d\n",out_b, digitalRead(READ_WRITE));
  Serial.write(buf);
  out_b = 0x00;
  /* Delay on PHI2 low so we can see the LEDs (address bus is set in PHI1 aka PHI2 low)*/
  delay(2000);
  Serial.write("Delay 1 done\n\n");
  
  /* Now jump to 0x5555 (last 4 0101) */
  Serial.write("***Writing 0x4C again\n");
  db_write(0x4C);
  tickClock(1);
  Serial.write("Writing 0x55\n");
  db_write(0x55);
  tickClock(2);
  Serial.write("Writes jmp 2 done\n");
  Serial.write("Reading from addr bus***\n");
  for(int i = 0; i < 3; i++){
    out_b += (digitalRead(ADDR[i]) << i);
  }
  sprintf(buf,"A:0x%x,RW:%d\n",out_b,digitalRead(READ_WRITE));
  Serial.write(buf);
  delay(2000);
  Serial.write("Delay 2 done\n\n");
}
