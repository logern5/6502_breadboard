/* Control pins */
#define CLOCK 2
#define READ_WRITE 4
#define RST 6
/* Should we delay the PHI2 clock? */
//#define DEL_PHI2
/* How long to delay the PHI2 clock */
#define DEL 1
/* Should debug prints be displayed? */
#define DEBUG
/* Should we slow down the clock during the reset sequence */
/* This seems to be necessary or else some garabage gets printed */
#define RST_DEL
/* How long to delay PHI2 during reset? */
#define RST_TIME 100

#ifdef DEBUG
#define debug_print Serial.write
#else
#define debug_print (void)
#endif

#ifdef DEBUG
#define debug_sprintf sprintf
#else
#define debug_sprintf (void)
#endif

#ifdef RST_DEL
#define rst_wait delay
#else
#define rst_wait (void)
#endif

#ifdef DEL_PHI2
#define phi2_wait delay
#else
#define phi2_wait (void)
#endif

/* LSB to MSB 24..36 --> D0..D7 */
const char DATA[] = {22, 24, 26, 28, 30, 32, 34, 36};
/* LSB to MSB A0..A2 --> 44..48 */
const char ADDR[] = {44, 46, 48, 50};
/* Programs can be at most 32 bytes long */
/* LDA 'h'; STA $0C; LDA 'i'; STA $0C; NOP; NOP; NOP; NOP; JMP 0xB  */
/* Write 'Hi' and loop forever */
const byte PROG[] = {0xA9, 0x48, 0x85, 0x0C, 0xA9, 0x69, 0x85, 0x0C, 0xEA, 0xEA, 0xEA, 0xEA, 0x4C, 0x0B, 0x00, 0xEA};
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
  set_output();
  for(int i = 0; i < 8; i++){
    digitalWrite(DATA[i], (dat >> i) & 1);
  }
}

char db_read(){
  byte out_byte = 0x00;
  set_input();
  for(int i = 0; i < 8; i++){
    out_byte += digitalRead(DATA[i]) << i;
    debug_print(digitalRead(DATA[i]));
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
  randomSeed(analogRead(0));
  /* Start the serial port so we can see what is going on */
  Serial.begin(57600);
  //delay(2000);
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
  int count = 1000;
  byte old_vec = 0x7; /* To make sure we are getting 0xC and 0xD in order */
  set_output();
  db_write(0x00); /* Write this for now, until we get a reset vector */
  Serial.write("\n***Waiting for reset***\n");
  while(1){
    digitalWrite(CLOCK, LOW);
    rst_wait(RST_TIME);
    char buf[64];
    byte ad = ad_read();
    debug_sprintf(buf, "Addr: 0x%x\n",ad);
    debug_print(buf);
    if(ad == 0xC){
      debug_print("****Reset vector 0xC detected!\n");
      db_write(0x00);
    }
    if(ad == 0xD && old_vec == 0xC){
      debug_print("****Reset vector 0xD detected!\n");
      db_write(0xF1);
      break;
    }
    old_vec = ad;
    rst_wait(RST_TIME);
    //db_write(0x00);
    digitalWrite(CLOCK, HIGH);
    rst_wait(RST_TIME);
  }
  rst_wait(RST_TIME);
  digitalWrite(CLOCK, HIGH);
  rst_wait(RST_TIME);
  Serial.write("***Reset OK***\n");
}

void loop() {
  debug_print("****In main loop!\n");
  phi2_wait(DEL);
  digitalWrite(CLOCK, LOW);
  /* Pni2 is low */
  char buf[64];
  byte rw = 0x0;
  byte ad = 0x0;
  rw = digitalRead(READ_WRITE);
  ad = ad_read();
  byte instr = PROG[ad];
  debug_sprintf(buf, "RW: %d, Ad: 0x%x, next instr: 0x%x\n",rw,ad,instr);
  set_output();
  if(rw == 1){
    db_write(instr);
  }
  if(rw == 0){
    set_input();
  }
  phi2_wait(DEL);
  digitalWrite(CLOCK, HIGH);
  /* Phi2 is high (CPU does mem access), so let's check the data bus */
  debug_print(buf);
  if(rw == 0){
    byte w_byte = 0x00;
    debug_print("****6502 writing...\n");
    w_byte = db_read();
    debug_sprintf(buf, "W: 0x%x\n", w_byte);
    debug_print(buf);
    /* Only print if 6502 wrote to memory mapped location */
    if(ad == 0xC){
     sprintf(buf, "%c", w_byte);
     Serial.write(buf);
    }
  }
}
