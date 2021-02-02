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
/* We are only using the first 4 bits (2^4=16 bytes) of the addr bus */
//const byte PROG[] = {0xEA, 0xEA, 0xEA};
//const byte PROG[] = {0XEA, 0XEA, 0xEA, 0xEA, 0XA9, 0XFF, 0X8D, 0x02, 0x60};
/* Program must always be 8 bytes long because of our address bus */
//const byte PROG[] = {0xA9, 0xFF, 0x8d, 0x02, 0x60, 0xEA, 0xEA, 0xEA};
const byte PROG[] = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0XEA, 0xEA, 0xEA};
/* Length of program and Arduino's program counter */
//int proglen = 9;
int pc = 0;
const char *mds[2];


void tickClock(int count){
  while(count--){
    digitalWrite(CLOCK, HIGH);
    digitalWrite(CLOCK, LOW);
  }
}

void db_write(char dat){
  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], OUTPUT);
  }
  for(int i = 0; i < 8; i++){
    digitalWrite(DATA[i], (dat >> i) & 1);
  }
}

char db_read(){
  byte out_byte = 0x00;
  for(int i = 0; i < 8; i++)
  
}
void setup() {
  // put your setup code here, to run once:
  /* Start the serial port so we can see what is going on */
  Serial.begin(57600);
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
  digitalWrite(RST, LOW);
  tickClock(7);
  digitalWrite(RST, HIGH);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  /* Phi2 is low now (AKA we are in PHI1), the MPU sets the RW pin, and the addr pins here */
  /* Retrieve an instruction from our 'ROM' */
  delay(1);
  byte rw = digitalRead(READ_WRITE);
  //byte inst = PROG[pc];
  byte cur_addr = 0x00;
  byte inst = 0x00;
  /* Read from the address bus so we get the right instruction */
  for(int i = 0; i < 3; i++){
    cur_addr += (digitalRead(ADDR[i]) << i);
  }
  inst = PROG[cur_addr];
  /* Prepare some info for the serial port */
  byte out_byte = 0x00;
  char buf[32];
  sprintf(buf,"Addr:0x%x,INST:0x%x,RW:%s\n",cur_addr,inst,mds[rw]);
  /* If the CPU is reading, let's write an instruction */
  if(rw == 1){
    /* Set pins to write mode */
    for(int i = 0; i < 8; i++){
      pinMode(DATA[i], OUTPUT);
    }
    /* Set the data bus before PHI2 is high */
    for(int i = 0; i < 8; i++){
      digitalWrite(DATA[i], (inst >> i) & 1);
    }
  }
  /* If the CPU is writing, then we will fetch and print to serial port DURING PHI2, not now */
  /* However, let's set the pins ahead of time */
  if(rw == 0){
    for(int i = 0; i < 8; i++){
      pinMode(DATA[i], INPUT);
    }
  }
  /* Wait a bit */
  delay(DEL);
  
  /* Set phi2 high (we are in PHI2), now the MPU will read from the data bus*/
  digitalWrite(CLOCK, HIGH);
  /* If the CPU is writing (sending us something), let's read what it has to say */
  if(rw == 0){
    for(int i = 0; i < 8; i++){
      out_byte += digitalRead(DATA[i]) << i;
    }
  }
  /* Wait a bit, and write our info to the serial port while we wait */
  Serial.write(buf);
  char buf2[8];
  /* If RW is low (e.g. CPU is writing), let's print what the CPU said */
  if(rw == 0){
    sprintf(buf2, "W:0x%x\n", out_byte);
    Serial.write(buf2);
  }
  delay(DEL);
  /* Set clock low (starting PHI1)*/
  digitalWrite(CLOCK, LOW);
}
