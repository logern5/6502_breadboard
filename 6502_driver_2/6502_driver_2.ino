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
  tickClock(4);
  digitalWrite(RST, HIGH);
  Serial.write("Low reset done\n");
  db_write(0xEA);
  tickClock(6); /* This needs to be done 6 time for some reason */
  db_write(0xEA);
  tickClock(2);
  /* Now, the reset operation is done, going into the loop! */
  
}

void loop() {
  /* We start on PHI2 low (PHI1)*/
  set_output();
  /* Because PHI2 is low, let's do a store operation. HC=0*/
  set_output();
  db_write(0xA9); /*LDA */
  digitalWrite(CLOCK, HIGH);
  /* Phi2 is high now, the CPU is reading this instruction. HC=1 */
  delay(10);
  digitalWrite(CLOCK, LOW);
  /* Phi2 is low again, the CPU is now setting it's address bus for the next byte(0xBF), and will read it next. HC=2 */
  db_write(0xBF);
  digitalWrite(CLOCK, HIGH);
  /* Phi2 is high, CPU fetches this byte (0xBF). HC=3 */
  delay(10);
  digitalWrite(CLOCK, LOW);
  /* LDA $BF is done! */
  
  /* Phi2 is low, CPU set's its ad bus for next byte (0x85). Let's prepare that byte. HC=4*/
  db_write(0x85); /*STA */
  digitalWrite(CLOCK, HIGH);
  /* Phi2 is high, CPU retrieves the byte 0x85. HC=5*/
  delay(10);
  digitalWrite(CLOCK, LOW);
  /* Phi2 is low, CPU set's ad bus for next byte (0xFA, where we are storing this byte). HC=6 */
  db_write(0xFA);
  digitalWrite(CLOCK, HIGH);
  /* Phi2 is high, CPU reads the byte 0xFA. HC=7*/
  delay(10);
  set_input();
  digitalWrite(CLOCK, LOW);
  /* Phi2 is low again. CPU sets ad bus to 0xFA (adress to store into. Last 3 digits are 010=2) HC=8 */
  /* RW SHOULD be set to 0 (write mode, 1 is read mode) now */
  set_input();
  char buf[32];
  byte add = 0;
  byte rw = 2;
  for(int i = 0; i < 3; i++){
    add += (digitalRead(ADDR[i]) << i);
  }
  rw = digitalRead(READ_WRITE);
  sprintf(buf, "Addr HC8:0x%x,RW:%d\n", add, rw);
  Serial.write(buf);
  set_input();
  /* Phi2 is high now, now it's going to write. HC=9*/
  delay(10);
  sprintf(buf,"Read byte: 0x%x\n",db_read());
  Serial.write(buf);
  delay(10);
  digitalWrite(CLOCK, LOW);
  /* Phi2 is low now, CPU setting addr bus to next instr. HC=10 */
}
