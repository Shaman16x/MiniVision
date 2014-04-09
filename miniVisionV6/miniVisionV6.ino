#include <TimerOne.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

/*
  This is where the user can program their own time limits
  for session time and reaction time
*/

int sessionTimes[] = {1, 3, 5};
int reactionTimes[] = {3, 2, 1};
int sIndex;
int rIndex;

/*
  EEPROM address 0-2 is for session number
  EEPROM address 3-15 is for the best session
*/

const int bpin0 = 0;        // input button 0 (digital pin number?)
const int bpin1 = 0;        // input button 1 (digital pin number?)
const int bpin2 = 0;        // input button 2 (digital pin number?)
const int bpin3 = 0;        // input button 3 (digital pin number?)
const int bpin4 = 0;        // input button 4 (digital pin number?)
const int outpin0 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin1 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin2 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin3 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin4 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin5 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin6 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin7 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin8 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin9 = 0;      // output pin to LED and Button (digital pin number?)
const int timePin = 0;      // time button (digital pin number?)
const int modePin = 0;      // mode button (digital pin number?)
const int goPin = 0;        // go button   (digital pin number?)
const int quitPin = 0;      // quit button (digital pin number?)
const int reactionPin = 0;  // reaction timer buttom (digital pin number?)
const int speakerPin = 0;   // speaker  (PWM pin number?)

/*
SD card attached to SPI bus as follows:
MOSI - pin 11
MISO - pin 12
CLK - pin 13
CS - pin 4
*/
const int chipSelect = 0;   // chipSelect for SD (digital pin number?)

int TRT;                          // used to count reaction times
int maxReactionTime = 3000;       // max reaction reaction time
int armSelect;                    // used to select active arm
int ledSelect;                    // used to select active led
int oldArm;                       // used to not light same led twice in a row
int oldLed;                       // used to not light same led twice in a row
volatile int timeLeft;            // used for session timer
volatile int reactionTimeLeft;    // used for reaction mode timer
volatile int counter;             // used for each reaction time
int sessionNum;                   // session number
int misses;                       // number of misses in reaction mode
int hits;                         // number of correct hits
int time;                         // time setting
int mode;                         // mode setting
boolean started;                  // alerts that session has started
volatile boolean quit;            // alerts that session has been quit
boolean go;                       // alerts that session can begin
volatile boolean timeUp;          // alerts that time limit is reached
volatile boolean reactionTimeUp;  // alerts that reaction time limit is reached for reaction mode

boolean update;
boolean newBest;
boolean change;
int second;
int BSN;
int BT;
int BH;
int BM;
int BRT;

/*
LCD RS pin to digital pin 12
LCD Enable pin to digital pin 11
LCD D4 pin to digital pin 5
LCD D5 pin to digital pin 4
LCD D6 pin to digital pin 3
LCD D7 pin to digital pin 2
LCD R/W pin to ground

10K resistor:
ends to +5V and ground
wiper to LCD VO pin (pin 3)
*/
//LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LiquidCrystal lcd(0, 0, 0, 0, 0, 0);  // digital pins?

void setup() {
  pinMode(timePin, INPUT_PULLUP);          // time pin as input with internal pullup
  pinMode(modePin, INPUT_PULLUP);          // mode pin as input with internal pullup
  pinMode(goPin, INPUT_PULLUP);            // go pin as input with internal pullup
  pinMode(quitPin, INPUT_PULLUP);          // quit pin as input with internal pullup
  pinMode(reactionPin, INPUT_PULLUP);      // reaction pin as input with internal pullup
  Timer1.initialize(1000);                 // sets timer1 for every ms
  Timer1.attachInterrupt(sessionTimer);    // sessionTimer updates timeLeft every timer1 interrupt
  pinMode(bpin0, INPUT_PULLUP);            // b pin 0 as input with internal pullup
  pinMode(bpin1, INPUT_PULLUP);            // b pin 1 as input with internal pullup
  pinMode(bpin2, INPUT_PULLUP);            // b pin 2 as input with internal pullup
  pinMode(bpin3, INPUT_PULLUP);            // b pin 3 as input with internal pullup
  pinMode(bpin4, INPUT_PULLUP);            // b pin 3 as input with internal pullup
  pinMode(outpin0, OUTPUT);                // out pin 0 as output
  pinMode(outpin1, OUTPUT);                // out pin 1 as output
  pinMode(outpin2, OUTPUT);                // out pin 2 as output
  pinMode(outpin3, OUTPUT);                // out pin 3 as output
  pinMode(outpin4, OUTPUT);                // out pin 4 as output
  pinMode(outpin5, OUTPUT);                // out pin 5 as output
  pinMode(outpin6, OUTPUT);                // out pin 6 as output
  pinMode(outpin7, OUTPUT);                // out pin 7 as output
  pinMode(outpin8, OUTPUT);                // out pin 8 as output
  pinMode(outpin9, OUTPUT);                // out pin 9 as output
  pinMode(speakerPin, OUTPUT);             // set speaker pin to output
  randomSeed(analogRead(0));               // sets seed from random floating pin 0
  lcd.begin(20, 4);                        // set up the LCD for 20 columns and 4 rows
  
  newBest = false;
  
  pinMode(53, OUTPUT);
  
  sessionNum = ((int)(EEPROM.read(0)-'0'))*100 + ((int)(EEPROM.read(1)-'0'))*10 + (int)(EEPROM.read(2)-'0');
  Serial.begin(9600);
}

// timer1 interrupt handler
// updates session time and displays every second
// if session is over is the timer for viewing results
void sessionTimer(){
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
  sIndex = 0;
  rIndex = 0;
  time = sessionTimes[0];
  reactionTimeLeft = reactionTimes[0];
  change = true;
  go = false;
  timeUp = false;
  reactionTimeUp = false;
  started = false;
  quit = false;
  mode = 0;
  hits = 0;
  misses = 0;
  armSelect = 0;
  ledSelect = 0;
  oldArm = 5;
  oldLed = 5;
  TRT = 0;
  Timer1.stop();
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
    saveSession((16*((sessionNum-1)%200))+19, sessionNum, time, hits, misses, false);
    displayEndSession();
    waitForQuit();
    Timer1.stop();
  }
}

// checks the settings buttons and updates display
void checkSettings(){
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
    lcd.print(timeLeft);
    lcd.setCursor(0,1);
    lcd.print(String(TRT));
  }
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
  if(newBest){
    BSN = ((int)(EEPROM.read(3)-'0'))*100 + ((int)(EEPROM.read(4)-'0'))*10 + (int)(EEPROM.read(5)-'0');
    BT = ((int)(EEPROM.read(6)-'0'))*100000 + ((int)(EEPROM.read(7)-'0'))*10000 + 
         ((int)(EEPROM.read(8)-'0'))*1000 + ((int)(EEPROM.read(9)-'0'))*100 + 
         ((int)(EEPROM.read(10)-'0'))*10 + (int)(EEPROM.read(11)-'0');
    BH = ((int)(EEPROM.read(12)-'0'))*1000 + ((int)(EEPROM.read(13)-'0'))*100 +
         ((int)(EEPROM.read(14)-'0'))*10 + (int)(EEPROM.read(15)-'0');
    BM = ((int)(EEPROM.read(16)-'0'))*100 + ((int)(EEPROM.read(17)-'0'))*10 + (int)(EEPROM.read(18)-'0');
    newBest = false;
  }
  
  int BRT = 0;
  if(BH+BM != 0)
    BRT = BT/(BH+BM);
    
  int ART = 0;
  if(hits != 0)
    ART = TRT/(hits+misses);
  
  if(ART > 0 && BRT > 0 && ART < BRT){
    saveSession(3,sessionNum, TRT, hits, misses, true);
    BRT = ART;
    newBest = true;
  }
  
  int whole = ART/1000;
  int dec = ART%1000;
  int bwhole = BRT/1000;
  int bdec = BRT%1000;
  String sn = String(sessionNum);
  String h = String(hits);
  String m = String(misses);
  String art;
  String brt;
  
  if(ART > 0){
    art = String(whole);
    art += ".";
    art += String(dec);
    art += " s";
  }
  else
    art = "no hits";
    
  if(BRT > 0){
    brt = String(bwhole);
    brt += ".";
    brt += String(bdec);
  }
  else
    brt = "no best data";
  
  lcd.clear();
  lcd.print("Session #: " + sn);
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
  while(digitalRead(quitPin) == HIGH && !quit);
  Timer1.stop();
  delay(20);
  lcd.clear();
  if(!quit)
    while(digitalRead(quitPin) == LOW);
}

// saves session data to memory
// 1 set of session data takes 16 bytes
// sNum takes 3 bytes
// time takes 6 bytes
// hits takes 4 bytes
// misses takes 3 bytes
void saveSession(int loc, int sNum, int time, int hits, int misses, boolean best){
  String sn = String(sNum);
  String t = String(time);
  String h = String(hits);
  String m = String(misses);
  
  if(sNum < 10)
    sn = "00" + sn;
  else if(sNum < 100)
    sn = "0" + sn;
    
  if(time < 10)
    t = "00000" + t;
  else if(time < 100)
    t = "0000" + t;
  else if(time < 1000)
    t = "000" + t;
  else if(time < 10000)
    t = "00" + t;
  else if(time < 100000)
    t = "0" + t;
  
  if(hits < 10)
    h = "000" + h;
  else if(hits < 100)
    h = "00" + h;
  else if(hits < 1000)
    h = "0" + h;
    
  if(misses < 10)
    m = "00" + m;
  else if(misses < 100)
    m = "0" + m;
  
  // save the session # if not saving best session
  if(!best){
    EEPROM.write(0,sn.charAt(0));
    EEPROM.write(1,sn.charAt(1));
    EEPROM.write(2,sn.charAt(2));
  }
  
  // snum 3 bytes
  EEPROM.write(loc,sn.charAt(0));
  EEPROM.write(loc+1,sn.charAt(1));
  EEPROM.write(loc+2,sn.charAt(2));
  // time 6 bytes
  EEPROM.write(loc+3,t.charAt(0));
  EEPROM.write(loc+4,t.charAt(1));
  EEPROM.write(loc+5,t.charAt(2));
  EEPROM.write(loc+6,t.charAt(3));
  EEPROM.write(loc+7,t.charAt(4));
  EEPROM.write(loc+8,t.charAt(5));
  // hits 4 bytes
  EEPROM.write(loc+9,h.charAt(0));
  EEPROM.write(loc+10,h.charAt(1));
  EEPROM.write(loc+11,h.charAt(2));
  EEPROM.write(loc+12,h.charAt(3));
  // misses 3 bytes
  EEPROM.write(loc+13,h.charAt(0));
  EEPROM.write(loc+14,h.charAt(1));
  EEPROM.write(loc+15,h.charAt(2));
}

// save data to SD card
void saveToSD(){
  // do stuff
}

void playSound(){
  tone(speakerPin, 1000, 250);
}

void turnOffLEDs(){
  digitalWrite(outpin0, HIGH);
  digitalWrite(outpin1, HIGH);
  digitalWrite(outpin2, HIGH);
  digitalWrite(outpin3, HIGH);
  digitalWrite(outpin4, LOW);
  digitalWrite(outpin5, LOW);
  digitalWrite(outpin6, LOW);
  digitalWrite(outpin7, LOW);
}

// runs standard mode
// each iteration lights 1 led
void runStandardMode(){
  lightRandomLED();
  reactionTimeLeft = reactionTimes[rIndex];
  second = 0;
  counter = -1;
  Timer1.start();
  // check for correct button
  if(ledSelect == 0){
    while(digitalRead(bpin0) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 1){
    while(digitalRead(bpin1) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 2){
    while(digitalRead(bpin2) == HIGH && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 3){
    while(digitalRead(bpin3) == HIGH && !timeUp)
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
  }
  else{
    while(digitalRead(bpin4) == HIGH && !timeUp){
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
    delay(20);
    while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW ||
          digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW || digitalRead(bpin4) == LOW);
  }
}

// runs reaction mode
// each iteration lights 1 led
void runReactionMode(){
  lightRandomLED();
  reactionTimeLeft = reactionTimes[rIndex];
  second = 0;
  counter = -1;
  Timer1.start();
  // check for correct button
  if(ledSelect == 0){
    while(digitalRead(bpin0) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 1){
    while(digitalRead(bpin1) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 2){
    while(digitalRead(bpin2) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else if(ledSelect == 3){
    while(digitalRead(bpin3) == HIGH && !reactionTimeUp && !timeUp){
      if(digitalRead(quitPin) == LOW){
        quit = true;
        break;
      }
    }
  }
  else{
    while(digitalRead(bpin4) == HIGH && !reactionTimeUp && !timeUp){
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
    if(!reactionTimeUp){
      hits++;
      playSound();
    }
    else{
      misses++;
    }
    delay(20);
    while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW ||
          digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW || digitalRead(bpin4) == LOW);
  }
}

// determines a random LED to light and sets the appropriate pins
void lightRandomLED(){
  turnOffLEDs();
  
  // this makes sure a different led will light every time
  while(armSelect == oldArm && ledSelect == oldLed){
    armSelect = random(5);
    ledSelect = random(5);
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
  else if(armSelect == 3)
    digitalWrite(outpin3, LOW);
  else
    digitalWrite(outpin4, LOW);
  
  if(ledSelect == 0)
    digitalWrite(outpin5, HIGH);
  else if(ledSelect == 1)
    digitalWrite(outpin6, HIGH);
  else if(ledSelect == 2)
    digitalWrite(outpin7, HIGH);
  else if(ledSelect == 3)
    digitalWrite(outpin8, HIGH);
  else
    digitalWrite(outpin9, HIGH);
}
