#include <EEPROM.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>

/*
  This is where the user can program their own time limits
  for session time and reaction time
*/

int sessionTimes[] = {1, 3, 5};
int reactionTimes[] = {3, 2, 1};
int sIndex;
int rIndex;

const int b1 = 30;             // input button 0 (digital pin number?)
const int b2 = 33;             // input button 1 (digital pin number?)
const int b3 = 32;             // input button 2 (digital pin number?)
const int b4 = 35;             // input button 3 (digital pin number?)
const int b5 = 34;             // input button 4 (digital pin number?)

const int LED1 = 27;           // output pin to LED and Button (digital pin number?)
const int LED2 = 26;           // output pin to LED and Button (digital pin number?)
const int LED3 = 29;           // output pin to LED and Button (digital pin number?)
const int LED4 = 28;           // output pin to LED and Button (digital pin number?)
const int LED5 = 31;           // output pin to LED and Button (digital pin number?)

const int bottom = 25;           // output pin to LED and Button (digital pin number?)
const int right = 22;           // output pin to LED and Button (digital pin number?)
const int left = 24;           // output pin to LED and Button (digital pin number?)
const int top = 23;           // output pin to LED and Button (digital pin number?)

const int timePin = 49;            // time button (digital pin number?)
const int modePin = 46;            // mode button (digital pin number?)
const int goPin = 47;              // go button   (digital pin number?)
const int quitPin = 6;            // quit button (digital pin number?)
const int reactionPin = 48;        // reaction timer buttom (digital pin number?)
const int speakerPin = 8;         // speaker  (PWM pin number?)
const int cardDetectPin = 18;     // card detect pin for sd socket

// SD pin definitions
const int sdCardSelect = 53;      // card setect 

long TRT;                         // used to count reaction times
int ledSelect;                    // used to select active led
int armSelect;                    // used to select active arm
int oldArm;                       // used to not light same led twice in a row
int oldLed;                       // used to not light same led twice in a row
volatile int timeLeft;            // used for session timer
volatile int reactionTimeLeft;    // used for reaction mode timer
volatile long counter;            // used for each reaction time
int sessionNum;                   // session number
int misses;                       // number of misses in reaction mode
int hits;                         // number of correct hits
int time;                         // time setting
int mode;                         // mode setting
boolean started;                  // alerts that session has started
boolean update;
boolean initial;
volatile boolean cardDetect;
volatile boolean quit;            // alerts that session has been quit
volatile boolean go;              // alerts that session can begin
volatile boolean timeUp;          // alerts that time limit is reached
volatile boolean reactionTimeUp;  // alerts that reaction time limit is reached for reaction mode

int second;
int startCounter;
boolean newBest;
boolean change;

int BSN;
int BMD;
int BT;
int BH;
int BM;
long BTRT;

int ART,whole,dec,BRT,bwhole,bdec;
String sn,sm,h,m,t,art,stats,rs;
String bsn,bmd,bh,bm,bt,brt;

int numSavedSessions;
#define MAX_ADR 2763

File sessionFile;

//LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LiquidCrystal lcd(37, 36, 43, 42, 45, 44);  // digital pins?

void setup() {
  lcd.clear();
  pinMode(timePin, INPUT_PULLUP);          // time pin as input with internal pullup
  pinMode(modePin, INPUT_PULLUP);          // mode pin as input with internal pullup
  pinMode(goPin, INPUT_PULLUP);            // go pin as input with internal pullup
  pinMode(quitPin, INPUT_PULLUP);          // quit pin as input with internal pullup
  pinMode(reactionPin, INPUT_PULLUP);      // reaction pin as input with internal pullup
  Timer1.initialize(1000);                 // sets timer1 for every ms
  Timer1.attachInterrupt(sessionTimer);    // sessionTimer updates timeLeft every timer1 interrupt
  pinMode(b1, INPUT_PULLUP);            // b pin 0 as input with internal pullup
  pinMode(b2, INPUT_PULLUP);            // b pin 1 as input with internal pullup
  pinMode(b3, INPUT_PULLUP);            // b pin 2 as input with internal pullup
  pinMode(b4, INPUT_PULLUP);            // b pin 3 as input with internal pullup
  pinMode(b5, INPUT_PULLUP);
  
  pinMode(LED1, OUTPUT);                // out pin 0 as output
  pinMode(LED2, OUTPUT);                // out pin 1 as output
  pinMode(LED3, OUTPUT);                // out pin 2 as output
  pinMode(LED4, OUTPUT);                // out pin 3 as output
  pinMode(LED5, OUTPUT);                // out pin 4 as output
  
  pinMode(bottom, OUTPUT);
  pinMode(right, OUTPUT);
  pinMode(left, OUTPUT);
  pinMode(top, OUTPUT);

  pinMode(speakerPin, OUTPUT);             // set speaker pin to output
  pinMode(cardDetectPin, INPUT_PULLUP);
  attachInterrupt(5, cdisr, RISING);
  randomSeed(analogRead(0));               // sets seed from random floating pin 0
  lcd.begin(20, 4);                        // set up the LCD for 20 columns and 4 rows
  lcd.clear();
  newBest = false;
  cardDetect = false;
  initial = false;
  
  sessionNum = ((int)EEPROM.read(0))*256 + (int)EEPROM.read(1);
  numSavedSessions = (int)EEPROM.read(2);
  
  BSN = ((int)EEPROM.read(3))*256 + (int)EEPROM.read(4);
  BMD = (long)EEPROM.read(5);
  BTRT = (((long)EEPROM.read(6))<<16) + (((long)EEPROM.read(7))<<8) + (long)EEPROM.read(8);
  BH = ((int)EEPROM.read(9))*256 + (int)EEPROM.read(10);
  BM = ((int)EEPROM.read(11))*256 + (int)EEPROM.read(12);
  BT = (int)EEPROM.read(13);
  
  pinMode(53, OUTPUT);
  
  ledSelect = 0;
  oldLed = 5;
  armSelect = 0;
  oldArm = 4;
  
  sIndex = 0;
  rIndex = 0;
  mode = 0;
  
  runFlashyStart();
  
  Serial.begin(9600);
}

void cdisr(){
  if(cardDetect)
    cardDetect = false;
}

// timer1 interrupt handler
// updates session time and displays every second
// if session is over is the timer for viewing results
void sessionTimer(){
  startCounter--;
  update = false;
  counter++;
  second++;
  if(second == 1000){
    second = 0;
    update = true;
    reactionTimeLeft--;
    timeLeft--;
  }
  
  if(reactionTimeLeft == 0)
    reactionTimeUp = true;
    
  if(timeLeft == 0){
    if(!timeUp){
      timeUp = true;
    }
    else{
      quit = true;
    }
  }
  
  if(!timeUp && update)
    updateDisplay();
}

// restores default values for starting a session
void setDefaults(){
  second = 0;
  go = false;
  change = true;
  timeUp = false;
  reactionTimeUp = false;
  started = false;
  quit = false;
  hits = 0;
  misses = 0;
  time = sessionTimes[sIndex];
  reactionTimeLeft = reactionTimes[rIndex];
  TRT = 0;
  Timer1.stop();
  sessionNum++;
}

// 1 loop is full session
void loop(){
  setDefaults();
  while(!go)
    checkSettings();
  while(!timeUp){
    if(!started){
      displayStartCounter();
      Timer1.start();
    }
    if(mode == 0)
      runStandardMode();
    else{
      reactionTimeUp = false;
      runReactionMode();
    }
  }
  Timer1.stop();
  tone(speakerPin, 500, 1000);
  turnOffLEDs();
  if(!quit){
    computeSessionStats();
    saveSession();
    displayEndSession();
    waitForQuit();
    Timer1.stop();
  }
  else
    sessionNum--;
}

void computeSessionStats(){
  sn = String(sessionNum);
  h = String(hits);
  m = String(misses);
  t = String(time);
  ART = 0;
  if(hits != 0)
    ART = TRT/(hits+misses);
  whole = ART/1000;
  dec = ART%1000;
  if(ART > 0){
    art = String(whole);
    art += ".";
    art += String(dec);
    art += " s";
  }
  else
    art = "no hits";
  
  if(BSN == 0 && hits != 0){
    BSN = sessionNum;
    BMD = mode;
    BT = time;
    BH = hits;
    BM = misses;
    BTRT = TRT;
    saveToEEPROM(true);
  }
  
  BRT = 0;
  if(BH != 0){
    BRT = BTRT/(BH+BM);
  }
  
  if(ART < BRT && ART > 0 && BRT > 0){
    BSN = sessionNum;
    BMD = mode;
    BT = time;
    BH = hits;
    BM = misses;
    BTRT = TRT;
    BRT = ART;
    saveToEEPROM(true);
  }
  
  bwhole = BRT/1000;
  bdec = BRT%1000;
  if(BRT > 0){
    brt = String(bwhole);
    brt += ".";
    brt += String(bdec);
    brt += " s";
  }
  else
    brt = "no best data";
  
  if(mode==0){
    sm = "Standard";
    rs = "";
  }
  else{
    sm = "Reaction";
    rs = ", Reaction Length: " + String(reactionTimeLeft) + " s";
  }
  stats = "Session # " + sn + ", Mode:" + sm + ", Average Reaction Time:" + art + 
          ", Hits:" + h + ", Misses:" + m + ", Time:" + time + " min" + rs;
}

void dumpEEPROMToSD(){
  int adr, val1, val2, val4, val5, val6;
  long reactionTime;
  String s,v1,v2,v3,v4,v5,v6;
  if(numSavedSessions <= 250)
    adr = 14;
  else
    adr = ((numSavedSessions-1)%250)*11 + 14;
  sessionFile = SD.open("data.txt", FILE_WRITE);
  if(!sessionFile)
    return;
  for(int i=0; i < 250 && i < numSavedSessions; i++, adr+=11){
    if(adr > MAX_ADR)
      adr = 14;
    val1 = ((int)EEPROM.read(adr))*256 + (int)EEPROM.read(adr+1);
    val2 = (int)EEPROM.read(adr+2);
    reactionTime = (((long)EEPROM.read(adr+3))<<16) + (((long)EEPROM.read(adr+4))<<8) + (long)EEPROM.read(adr+5);
    val4 = ((int)EEPROM.read(adr+6))*256 + (int)EEPROM.read(adr+7);
    val5 = ((int)EEPROM.read(adr+8))*256 + (int)EEPROM.read(adr+9);
    val6 = (int)EEPROM.read(adr+10);
    v1 = String(val1);
    if(val2 == 0)
      v2 = "Standard";
    else
      v2 = "Reaction";
    if(val4 == 0)
      v3 = "not hits";
    else
      v3 = String(reactionTime/1000) + "." + String(reactionTime%1000) + " s";
    v4 = String(val4);
    v5 = String(val5);
    v6 = String(val6);
    s= "Session # " + v1 + ", Mode:" + v2 + ", Average Reaction Time:" + v3 + 
       ", Hits:" + v4 + ", Misses:" + v5 + ", Time:" + v6;
    
    sessionFile.println(s);
  }
  sessionFile.close();
  numSavedSessions = 0;
  EEPROM.write(2,0);
}

// save session to sd if its there and if its not to the EEPROM
void saveSession(){
  EEPROM.write(0,sessionNum/256);
  EEPROM.write(1,sessionNum%256);
  
  int row = 0;
  lcd.clear();
  lcd.print("writing to the SD");
  if(!initial){
    if(SD.begin(53)){
      lcd.setCursor(0,++row);
      lcd.print("setup worked");
      initial = true;
      cardDetect = true;
      dumpEEPROMToSD();
    }
    else{
      lcd.setCursor(0,++row);
      lcd.print("card setup failed");
      lcd.setCursor(0,++row);
      lcd.print("saved on device");
    }
  }
  
  if(cardDetect){
    sessionFile = SD.open("data.txt", FILE_WRITE);
    if(sessionFile){
      lcd.setCursor(0,++row);
      lcd.print("card found");
      if(sessionFile.println(stats) == 0){
        lcd.setCursor(0,++row);
        lcd.print("write failed");
        delay(1000);
      }
      sessionFile.close();
    }
    lcd.setCursor(0,++row);
    lcd.print("done writing");
  }
  else{
    if(numSavedSessions < 250){
      numSavedSessions++;
      EEPROM.write(2,numSavedSessions);
    }
    saveToEEPROM(false);
    lcd.setCursor(0,++row);
    lcd.print("wrote to EEPROM");
  }
}

void saveToEEPROM(boolean best){
  if(best){
    EEPROM.write(3,(BSN/256));
    EEPROM.write(4,(BSN%256));
    EEPROM.write(5,BMD);
    EEPROM.write(6,(BTRT>>16)&0xFF);
    EEPROM.write(7,(BTRT>>8)&0xFF);
    EEPROM.write(8,BTRT&0xFF);
    EEPROM.write(9,BH/256);
    EEPROM.write(10,BH%256);
    EEPROM.write(11,BM/256);
    EEPROM.write(12,BM%256);
    EEPROM.write(13,BT);
  }
  else{
    int loc = ((numSavedSessions-1)%250)*11+14;
    EEPROM.write(loc,sessionNum/256);
    EEPROM.write(loc+1,sessionNum%256);
    EEPROM.write(loc+2,mode);
    EEPROM.write(loc+3,(TRT>>16)&0xFF);
    EEPROM.write(loc+4,(TRT>>8)&0xFF);
    EEPROM.write(loc+5,(TRT&0xFF));
    EEPROM.write(loc+6,hits/256);
    EEPROM.write(loc+7,hits%256);
    EEPROM.write(loc+8,misses/256);
    EEPROM.write(loc+9,misses%256);
    EEPROM.write(loc+10,time);
  }
}

// checks the settings buttons and updates display
void checkSettings(){
  if(digitalRead(quitPin) == LOW){
    change = true;
    sIndex = 0;
    time = sessionTimes[sIndex];
    mode = 0;
    rIndex = 0;
    reactionTimeLeft = reactionTimes[rIndex];
  }
  
  if(digitalRead(timePin) == LOW){
    change = true;
    sIndex = (sIndex+1)%((sizeof(sessionTimes)/sizeof(int)));
    time = sessionTimes[sIndex];
    delay(20);
    while(digitalRead(timePin) == LOW)
      delay(20);
  }
  
  if(digitalRead(modePin) == LOW){
    change = true;
    mode = (-1)*mode + 1;
    delay(20);
    while(digitalRead(modePin) == LOW)
      delay(20);
  }
  
  if(digitalRead(goPin) == LOW){
    change = true;
    go = true;
    delay(20);
    while(digitalRead(goPin) == LOW)
      delay(20);
  }
  
  if(mode == 1){
    if(digitalRead(reactionPin) == LOW){
      change = true;
      rIndex = (rIndex+1)%((sizeof(reactionTimes)/sizeof(int)));
      reactionTimeLeft = reactionTimes[rIndex];
      delay(20);
      while(digitalRead(reactionPin) == LOW)
        delay(20);
    }
  }
  
  updateDisplay();
}

// update the lcd display
void updateDisplay(){
  if(!started){
    if(change){
      lcd.clear();
      change = false;
      if(mode == 0)
        lcd.print("Mode: Standard");
      else
        lcd.print("Mode: Reaction");
      lcd.setCursor(0,1);
      String s2 = "Time: ";
      String s3 = s2 + time + " min(s)";
      lcd.print(s3);
      if(mode == 1){
        lcd.setCursor(0,2);
        String s4 = "Reaction Time: ";
        String s5 = s4 + reactionTimeLeft + " sec";
        lcd.print(s5);
      }
    }
  }
  else{
      lcd.clear();
      lcd.setCursor(3,1);
      lcd.print("Time Remaining");
      lcd.setCursor(9,2);
      lcd.print(timeLeft);
  }
}

void waitStart(int num){
  startCounter = num;
  while(startCounter > 0 && !quit){
    if(digitalRead(quitPin) == LOW){
      quit = true;
      timeUp = true;
    }
  }
}

// display the starting count down
void displayStartCounter(){
  //Serial.println("did I get here");
  timeLeft = 100;
  quit = false;
  timeUp = false;
  Timer1.start();
  lcd.clear();
  lcd.setCursor(10,1);
  waitStart(500);
  lcd.print("3");
  waitStart(1000);
  lcd.clear();
  lcd.setCursor(10,1);
  lcd.print("2");
  waitStart(1000);
  lcd.clear();
  lcd.setCursor(10,1);
  lcd.print("1");
  waitStart(1000);
  timeLeft = time*10;        // session time limit is time*60 seconds
  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("Time Remaining");
  lcd.setCursor(9,2);
  lcd.print(timeLeft);
  started = true;
  Timer1.stop();
}

// display ending statistics
void displayEndSession(){
  lcd.clear();
  lcd.print("Session #: " + sn);
  //lcd.print(" num:" + String(numSavedSessions));
  lcd.setCursor(0,1);
  lcd.print("Hits: " + h);
  lcd.print(", Miss: " + m);
  lcd.setCursor(0,2);
  lcd.print("Reaction: " + art);
  lcd.setCursor(0,3);
  lcd.print("Best: " + brt);
}

// quits session after 2 minutes or when they press quit
void waitForQuit(){
  timeLeft = 120;
  Timer1.start();
  while(digitalRead(quitPin) == HIGH && digitalRead(goPin) == HIGH && !quit);
  Timer1.stop();
  delay(20);
  lcd.clear();
  if(!quit)
    while(digitalRead(quitPin) == LOW || digitalRead(goPin) == LOW);
}

void turnOffLEDs(){
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, HIGH);
  digitalWrite(right, HIGH);
  digitalWrite(left, HIGH);
  digitalWrite(top, HIGH);
}

 
void playSound(){
  tone(speakerPin, 1000, 250);
}


// runs reaction mode
// each iteration lights 1 led
void runReactionMode(){
  
  lightRandomLED();
  reactionTimeLeft = reactionTimes[rIndex];
  counter = -1;
  Timer1.start();
  // check for correct button
  if(ledSelect == 0){
    while(digitalRead(b1) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 1){
    while(digitalRead(b2) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 2){
    while(digitalRead(b3) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 3){
    while(digitalRead(b4) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else{
    while(digitalRead(b5) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  Timer1.stop();
  if(quit){
    timeUp = true;
  }
  else if(!timeUp){
    TRT += counter;
    if(!reactionTimeUp){
      hits++;
      playSound();
    }
    else{
      misses++;
    }
    while(digitalRead(b1) == LOW || digitalRead(b2) == LOW ||
          digitalRead(b3) == LOW || digitalRead(b4) == LOW  || digitalRead(b5) == LOW);
    
  }
}


// runs standard mode
// each iteration lights 1 led
void runStandardMode(){
  lightRandomLED();
  counter = -1;
  Timer1.start();
  // check for correct button
  
  if(ledSelect == 0){
    while(digitalRead(b1) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 1){
    while(digitalRead(b2) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 2){
    while(digitalRead(b3) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 3){
    while(digitalRead(b4) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else{
    while(digitalRead(b5) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  
  Timer1.stop();
  if(quit)
    timeUp = true;
  else if(!timeUp){
    TRT += counter;
    hits++;
    playSound();
    while(digitalRead(b1) == LOW || digitalRead(b2) == LOW ||
          digitalRead(b3) == LOW || digitalRead(b4) == LOW || digitalRead(b5) == LOW);
  }
}


void lightRandomLED(){
  //there may be a better way for this
  turnOffLEDs();
  
  
  // this makes sure a different led will light every time
  while(ledSelect == oldLed && armSelect == oldArm){
    armSelect = random(4);
    ledSelect = random(5);
  }
  oldLed = ledSelect;
  oldArm = armSelect;
  
  /*
  ledSelect++;
  if(ledSelect == 5){
    ledSelect = 0;
    armSelect++;
    if(armSelect == 4)
      armSelect = 0;
  }
  */
  
  // light random led
  if(ledSelect == 0)
    digitalWrite(LED1, HIGH);
  else if(ledSelect == 1)
    digitalWrite(LED2, HIGH);
  else if(ledSelect == 2)
    digitalWrite(LED3, HIGH);
  else if(ledSelect == 3)
    digitalWrite(LED4, HIGH);
  else
    digitalWrite(LED5, HIGH);
    
  if(armSelect == 0)
    digitalWrite(bottom, LOW);
  else if(armSelect == 1)
    digitalWrite(right, LOW);
  else if(armSelect == 2)
    digitalWrite(left, LOW);
  else
    digitalWrite(top, LOW);
}

void runFlashyStart(){
  lcd.setCursor(2,1);
  lcd.print("Welcome, friend");
  // flash all the leds
  //LED Position 1
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);
  
  //LED Position 2
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);
  
  //LED Position 3
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);
  
  //LED Position 4
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);
  
  //LED Position 5
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, HIGH);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);
  
  //LED Position 4
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);
  
  //LED Position 3
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);

  //LED Position 2
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  
  digitalWrite(bottom, LOW);
  digitalWrite(right, LOW);
  digitalWrite(left, LOW);
  digitalWrite(top, LOW);
  delay(500);
  
  //LED Position 1
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  delay(500);
  
  turnOffLEDs();
  

}
