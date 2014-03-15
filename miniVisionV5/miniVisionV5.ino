#include <TimerOne.h>
#include <TimerThree.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

/*
  EEPROM address 0 is for the best session
  EEPROM address 1 is for session number
*/

const int bpin0 = 0;        // input button 0 (pin number?)
const int bpin1 = 0;        // input button 0 (pin number?)
const int bpin2 = 0;        // input button 0 (pin number?)
const int bpin3 = 0;        // input button 0 (pin number?)
const int outpin0 = 0;      // output pin to LED and Button (pin number?)
const int outpin1 = 0;      // output pin to LED and Button (pin number?)
const int outpin2 = 0;      // output pin to LED and Button (pin number?)
const int outpin3 = 0;      // output pin to LED and Button (pin number?)
const int outpin4 = 0;      // output pin to LED and Button (pin number?)
const int outpin5 = 0;      // output pin to LED and Button (pin number?)
const int outpin6 = 0;      // output pin to LED and Button (pin number?)
const int outpin7 = 0;      // output pin to LED and Button (pin number?)
const int timePin = 0;      // time button (pin number?)
const int modePin = 0;      // mode button (pin number?)
const int goPin = 0;        // go button   (pin number?)
const int quitPin = 0;      // quit button (pin number?)
const int speakerPin = 0;   // speaker  (pin number?)
const int chipSelect = 0;   // chipSelect for SD (pin number?)

int armSelect;                    // used to select active arm
int ledSelect;                    // used to select active led
int oldArm;                       // used to not light same led twice in a row
int oldLed;                       // used to not light same led twice in a row
volatile int timeLeft;            // used for session timer
volatile int reactionTimeLeft;    // used for reaction timer
int sessionNum;                   // session number
int hits;                         // number of correct hits
int time;                         // time setting
int mode;                         // mode setting
boolean started;                  // alerts that session has started
volatile boolean quit;            // alerts that session has been quit
boolean go;                       // alerts that session can begin
volatile boolean timeUp;          // alerts that time limit is reached
volatile boolean reactionTimeUp;  // alerts that reaction time limit is reached for reaction mode

LiquidCrystal lcd(12, 11, 9, 8, 7, 6, 5, 4, 3, 2);  // pin setup for LCD

void setup() {
  pinMode(timePin, INPUT_PULLUP);          // time pin as input with internal pullup
  pinMode(modePin, INPUT_PULLUP);          // mode pin as input with internal pullup
  pinMode(goPin, INPUT_PULLUP);            // go pin as input with internal pullup
  pinMode(quitPin, INPUT_PULLUP);          // quit pin as input with internal pullup
  Timer1.initialize(1000000);              // sets timer1 for every second
  Timer1.attachInterrupt(sessionTimer);    // sessionTimer updates timeLeft every timer1 interrupt
  Timer3.initialize(1000000);              // sets timer3 for every second
  Timer3.attachInterrupt(reactionTimer);   // reactionTimer updates reactionTimeLeft every timer3 interrupt
  pinMode(bpin0, INPUT_PULLUP);            // b pin 0 as input with internal pullup
  pinMode(bpin1, INPUT_PULLUP);            // b pin 1 as input with internal pullup
  pinMode(bpin2, INPUT_PULLUP);            // b pin 2 as input with internal pullup
  pinMode(bpin3, INPUT_PULLUP);            // b pin 3 as input with internal pullup
  pinMode(outpin0, OUTPUT);                // out pin 0 as output
  pinMode(outpin1, OUTPUT);                // out pin 1 as output
  pinMode(outpin2, OUTPUT);                // out pin 2 as output
  pinMode(outpin3, OUTPUT);                // out pin 3 as output
  pinMode(outpin4, OUTPUT);                // out pin 4 as output
  pinMode(outpin5, OUTPUT);                // out pin 5 as output
  pinMode(outpin6, OUTPUT);                // out pin 6 as output
  pinMode(outpin7, OUTPUT);                // out pin 7 as output
  pinMode(speakerPin, OUTPUT);             // set speaker pin to output
  randomSeed(analogRead(0));               // sets seed from random floating pin 0
  lcd.begin(16, 4);                        // set up the LCD for 16 columns and 4 rows
  
  sessionNum = int(EEPROM.read(0) + EEPROM.read(1) + EEPROM.read(2));
  Serial.begin(9600);
}

// timer1 interrupt handler
// updates session time and displays every second
// if session is over is the timer for viewing results
void sessionTimer(){
  timeLeft--;
  updateDisplay();
  if(timeLeft == 0){
    if(!timeUp){
      noInterrupts();
      timeUp = true;
    }
    else{
      quit = true;
    }
  }
}

// timer3 interrupt handler
// updates reaction time every second
void reactionTimer(){
  reactionTimeLeft--;
  if(reactionTimeLeft == 0)
    reactionTimeUp = true;
}

// restores default values for starting a session
void setDefaults(){
  noInterrupts();
  go = false;
  timeUp = false;
  reactionTimeUp = false;
  started = false;
  quit = false;
  time = 1;
  mode = 0;
  hits = 0;
  armSelect = 0;
  ledSelect = 0;
  oldArm = 4;
  oldLed = 4;
  Timer1.stop();
  Timer3.stop();
  sessionNum++;
}

// 1 loop is full session
void loop(){
  if (SD.begin(chipSelect))
    saveToSD();
  setDefaults();
  while(!go)
    checkSettings();
  while(!timeUp){
    if(!started){
      timeLeft = time*60;        // session time limit is time*60 seconds
      displayStartCounter();
      interrupts();
      Timer1.start();
    }
    if(mode == 0)
      runStandardMode();
    else{
      reactionTimeLeft = 3;      // reaction time limit is 3 seconds
      reactionTimeUp = false;
      runReactionMode();
    }
  }
  noInterrupts();
  Timer1.stop();
  saveSession((8*((sessionNum-1)%200))+11, sessionNum, time, hits, false);
  interrupts();
  displayEndSession();
  waitForQuit();
  Timer1.stop();
}

// checks the settings buttons and updates display
void checkSettings(){
  if(digitalRead(timePin) == LOW){
    time+=2;
    if(time > 5)
      time = 1;
    delay(20);
    while(digitalRead(timePin) == LOW)
      delay(20);
  }
  
  if(digitalRead(modePin) == LOW){
    mode = (-1)*mode + 1;
    delay(20);
    while(digitalRead(modePin) == LOW)
      delay(20);
  }
  
  if(digitalRead(goPin) == LOW){
    go = true;
    delay(20);
    while(digitalRead(goPin) == LOW)
      delay(20);
  }
  updateDisplay();
}

// update the lcd display
void updateDisplay(){
  lcd.clear();
  if(!started){
    if(mode == 0)
      lcd.print("Mode: Standard");
    else
      lcd.print("Mode: Reaction");
    lcd.setCursor(0,1);
    String s2 = "Time: ";
    String s3 = s2 + time + " min(s)";
    lcd.print(s3);
  }
  else
    lcd.print(timeLeft);
}

// display the starting count down
void displayStartCounter(){
  lcd.clear();
  delay(500);
  lcd.print("3");
  delay(1000);
  lcd.clear();
  lcd.print("2");
  delay(1000);
  lcd.clear();
  lcd.print("1");
  delay(1000);
  lcd.clear();
  lcd.print("start");
  delay(500);
  lcd.clear();
  lcd.print(timeLeft);
  started = true;
}

// display ending statistics
void displayEndSession(){
  int BSN = int(EEPROM.read(3) + EEPROM.read(4) + EEPROM.read(5));
  int BT = int(EEPROM.read(6))*60;
  int BH = int(EEPROM.read(7) + EEPROM.read(8) + EEPROM.read(9) + EEPROM.read(10));
  double BRT = 1.0*BH/BT;
  double RT = 1.0*hits/time*60;
  
  if(RT > BRT){
    saveSession(3,sessionNum, time, hits, true);
    BRT = RT;
  }
  
  int whole = int(RT);
  int dec = int((RT - int(RT))*100);
  int bwhole = int(BRT);
  int bdec = int((BRT - int(BRT))*100);
  String sn = String(sessionNum);
  String t = String(time);
  String h = String(hits);
  String rt = whole + "." + dec;
  String brt = bwhole + "." + bdec;
  
  lcd.clear();
  lcd.print("Session #: " + sn);
  lcd.setCursor(0,1);
  lcd.print("Hits: " + h);
  lcd.setCursor(0,2);
  lcd.print("Reaction: " + rt);
  lcd.setCursor(0,3);
  lcd.print("Best: " + brt);
}

// quits session after 2 minutes or when they press quit
void waitForQuit(){
  timeLeft = 120;
  Timer1.start();
  while(digitalRead(quitPin) == HIGH && !quit)
    delay(20);
  Timer1.stop();
  delay(20);
  lcd.clear();
  if(!quit)
    while(digitalRead(quitPin) == LOW)
      delay(20);
}

// saves session data to memory
void saveSession(int loc, int sNum, int time, int hits, boolean best){
  String sn = String(sNum);
  String t = String(time);
  String h = String(hits);
  
  if(sNum < 10)
    sn = "00" + sn;
  else if(sNum < 100)
    sn = "0" + sn;
  
  if(hits < 10)
    h = "000" + h;
  else if(hits < 100)
    h = "00" + h;
  else if(hits < 1000)
    h = "0" + h;
    
  if(!best){
    EEPROM.write(0,sn.charAt(0));
    EEPROM.write(1,sn.charAt(1));
    EEPROM.write(2,sn.charAt(2));
  }
  
  EEPROM.write(loc,sn.charAt(0));
  EEPROM.write(loc+1,sn.charAt(1));
  EEPROM.write(loc+2,sn.charAt(2));
  EEPROM.write(loc+3,t.charAt(0));
  EEPROM.write(loc+4,h.charAt(0));
  EEPROM.write(loc+5,h.charAt(1));
  EEPROM.write(loc+6,h.charAt(2));
  EEPROM.write(loc+10,h.charAt(3));
}

// save data to SD card
void saveToSD(){
  // do stuff
}

void playSound(){
  tone(speakerPin, 1000, 50);
}

// runs standard mode
// each iteration lights 1 led
void runStandardMode(){
  lightRandomLED();
  
  // check for correct button
  while(digitalRead(bpin0) == HIGH && digitalRead(bpin1) == HIGH &&
        digitalRead(bpin2) == HIGH && digitalRead(bpin3) == HIGH)
    delay(20);
  hits++;
  playSound();
  delay(20);
  while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW ||
        digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW)
    delay(20);
}

// runs reaction mode
// each iteration lights 1 led
void runReactionMode(){
  lightRandomLED();
  
  Timer3.start();
  // check for correct button
  while(digitalRead(bpin0) == HIGH && digitalRead(bpin1) == HIGH &&
        digitalRead(bpin2) == HIGH && digitalRead(bpin3) == HIGH && !reactionTimeUp)
    delay(20);
  Timer3.stop();
  if(!reactionTimeUp){
    hits++;
    playSound();
  }
  delay(20);
  while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW ||
        digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW)
    delay(20);
}

// determines a random LED to light and sets the appropriate pins
void lightRandomLED(){
  //there may be a better way for this
  digitalWrite(outpin0, HIGH);
  digitalWrite(outpin1, HIGH);
  digitalWrite(outpin2, HIGH);
  digitalWrite(outpin3, HIGH);
  digitalWrite(outpin4, LOW);
  digitalWrite(outpin5, LOW);
  digitalWrite(outpin6, LOW);
  digitalWrite(outpin7, LOW);
  
  // this makes sure a different led will light every time
  while(armSelect == oldArm && ledSelect == oldLed){
    armSelect = random(4);
    ledSelect = random(4);
  }
  oldArm = armSelect;
  oldLed = ledSelect;
  
  // light random led
  if(armSelect == 0)
    digitalWrite(outpin0, LOW);
  else if(armSelect == 1)
    digitalWrite(outpin1, LOW);
  else if(armSelect == 2)
    digitalWrite(outpin2, LOW);
  else
    digitalWrite(outpin3, LOW);
  
  if(ledSelect == 0)
    digitalWrite(outpin4, HIGH);
  else if(ledSelect == 1)
    digitalWrite(outpin5, HIGH);
  else if(ledSelect == 2)
    digitalWrite(outpin6, HIGH);
  else
    digitalWrite(outpin7, HIGH);
}
