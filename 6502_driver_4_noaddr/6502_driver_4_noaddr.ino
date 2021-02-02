/* Control pins */
#define CLOCK 2
#define READ_WRITE 4
#define RST 51
/* Should we delay the PHI2 clock? */
//#define DEL_PHI2
/* How long to delay the PHI2 clock */
#define DEL 1
/* Should debug prints be displayed? */
//#define DEBUG
/* Should we slow down the clock during the reset sequence */
/* This seems to be necessary or else some garabage gets printed */
#define RST_DEL
/* How long to delay PHI2 during reset? */
#define RST_TIME 5
/* How many seconds should we delay in reset? */
#define RST_TOTAL_TIME 2000


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
/* We aren't using the address pins */
//const char ADDR[] = {44, 46, 48, 50};
/* Write 'Hello world'*/
const byte PROG[] = {
  0xA9, 'H',
  0x85, 0x0C,
  0xEA,
  0xA9, 'e',
  0x85, 0x0C,
  0xEA,
  0xA9, 'l',
  0x85, 0x0C,
  0xea,
  0xA9, 'l',
  0x85, 0x0C,
  0xEA,
  0xA9, 'o',
  0x85, 0x0C,
  0xEA,
  0xA9, ' ',
  0x85, 0x0c,
  0xEA,
  0xA9, 'W',
  0x85, 0x0c,
  0xea,
  0xa9, 'o',
  0x85, 0x0c,
  0xea,
  0xa9, 'r',
  0x85, 0x0c,
  0xea,
  0xa9, 'l',
  0x85, 0x0c,
  0xea,
  0xa9, 'd',
  0x85, 0x0c,
  0xea,
  0xea, 0xea, 0xea, 0xea, 0xea, 0xea, 0xea
};
byte codelen = 62;
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

/* ad_read() is no longer needed because we aren't using the address bus */
/*
char ad_read(){
  byte out_byte = 0x00;
  set_input();
  for(int i = 0; i < 4; i++){
    out_byte += digitalRead(ADDR[i]) << i;
  }
  return out_byte;
}
*/

byte ad = 0x0;
#define EA_COUNT 40

void spam_ea(int cnt){
  set_output();
  db_write(0xEA);
  debug_print("***Sending extra 0xEAs\n");
  while(cnt--){
    digitalWrite(CLOCK, LOW);
    set_output();
    db_write(0xEA);
    digitalWrite(CLOCK, HIGH);
  }
  debug_print("***Done sending extra 0xEAs\n\n");
}

void nClocks(int count){
  while(count--) {
    digitalWrite(CLOCK, HIGH);
    digitalWrite(CLOCK, LOW);
  }
}

void code_uploader(){
  //spam_ea(EA_COUNT);
  while(codelen--){
    debug_print("\n****In main loop!\n");
    phi2_wait(DEL);
    digitalWrite(CLOCK, LOW);
    /* Phi2 is low, 6502 does mem access */
    char buf[64];
    byte rw = 0x0;
    //byte ad = 0x0;
    rw = digitalRead(READ_WRITE);
    byte instr = PROG[ad];
    debug_sprintf(buf, "RW: %d, Ad: 0x%x, next/cur instr: 0x%x\n",rw,ad,instr);
    set_output();
    /* If the 6502 is reading from the data bus */
    if(rw == 1){
      debug_print("****Got RW=1 (reading!)\n");
      db_write(instr);
    }
    /* Else, set the data pins to input mode */
    if(rw == 0){
      set_input();
    }
    /* Always increment the address */
    ad++;
    phi2_wait(DEL);
    digitalWrite(CLOCK, HIGH);
    /* Phi2 is high (CPU does mem access), so let's check the data bus */
    debug_print(buf);
    if(rw == 0){
      byte w_byte = 0x00;
      memset(buf,32,0);
      debug_print("****6502 writing...\n");
      w_byte = db_read();
      debug_sprintf(buf, "**W: <0x%x>\n", w_byte);
      debug_print(buf);
      /* There are no addresses, so memory mapping doesn't matter */
      if(1){
       debug_print("****6502 writing to memory mapped location!\n");
       sprintf(buf, "%c", w_byte);
       Serial.write(buf);
      }
    }
  }
  debug_print("****code_uploader() over\n\n");
}


void setup() {
  ad = 0x0;
  // put your setup code here, to run once:
  randomSeed(analogRead(0));
  /* Start the serial port so we can see what is going on */
  Serial.begin(57600);
  /* Set up RW modes table for serial printing */
  mds[0] = "Write";
  mds[1] = "Read";
  /* We will start out using data as output */
  for(int i = 0; i < 8; i++){
    pinMode(DATA[i], OUTPUT);
  }
  /* Set mode of control pins */
  pinMode(CLOCK, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(READ_WRITE, INPUT);
  //int count = 1000;
  set_output();
  digitalWrite(CLOCK, LOW);
  db_write(0xEA); /* Write this for now, until we get a reset vector */
  digitalWrite(CLOCK, HIGH);
  Serial.write("\n***Waiting for reset NOADDR mode***\n");
  Serial.write("***Meanwhile spamming 0xEA on the data bus***\n");
  Serial.write("***Setting reset vector through the Arduino pin***\n");
  int max_cycles = RST_TOTAL_TIME/RST_TIME;
  /* Spin, give the user time to reset the CPU */
  /* This seems to have issues, as garbled text sometimes appears if the user resets the MPU */
  /*
  while(max_cycles--){
    digitalWrite(CLOCK, LOW);
    rst_wait(RST_TIME);
    digitalWrite(CLOCK, HIGH);
    rst_wait(RST_TIME);
  }
  rst_wait(RST_TIME);
  digitalWrite(CLOCK, HIGH);
  rst_wait(RST_TIME);
  Serial.write("***Reset grace period over, entering code exec***\n");
  */
  digitalWrite(RST, LOW);
  nClocks(7); /* This needs to be 7 or more cycles */
  digitalWrite(RST, HIGH);
  db_write(0xEA);
  nClocks(7); /*Having the right amount of reset cycles fixes the error weirdness. A guide said 6 for this, but it didn't work for me. */
  db_write(0x00); /*These are the 2 bytes of the reset vector */
  nClocks(2);
  
  code_uploader();
  set_output();
  db_write(0xEA);
  Serial.write("\n***Code exec is over, holding clock low.\n");
}


void loop() {
  digitalWrite(CLOCK, LOW);
}
