#include <LiquidCrystal.h>
#include <TimerOne.h>
//#include <TimerThree.h>

/*
  This is where the user can program their own time limits
  for session time and reaction time
*/

int sessionTimes[] = {1, 3, 5};
int reactionTimes[] = {3, 2, 1};
int sIndex;
int rIndex;

const int bpin0 = 34;        // input button 0 (digital pin number?)
const int bpin1 = 35;        // input button 0 (digital pin number?)
const int bpin2 = 36;        // input button 0 (digital pin number?)
const int bpin3 = 37;        // input button 0 (digital pin number?)
const int outpin0 = 22;      // output pin to LED and Button (digital pin number?)
const int outpin1 = 23;      // output pin to LED and Button (digital pin number?)
const int outpin2 = 24;      // output pin to LED and Button (digital pin number?)
const int outpin3 = 25;      // output pin to LED and Button (digital pin number?)
const int outpin4 = 26;      // output pin to LED and Button (digital pin number?)
const int timePin = 6;      // time button (digital pin number?)
const int modePin = 7;      // mode button (digital pin number?)
const int goPin = 5;        // go button   (digital pin number?)
const int quitPin = 4;      // quit button (digital pin number?)
const int reactionPin = 8;  // reaction timer buttom (digital pin number?)
const int speakerPin = 2;   // speaker  (PWM pin number?)

long TRT;                          // used to count reaction times
int maxReactionTime = 3000;       // max reaction reaction time
int ledSelect;                    // used to select active led
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
boolean update;
volatile boolean quit;            // alerts that session has been quit
volatile boolean go;                       // alerts that session can begin
volatile boolean timeUp;          // alerts that time limit is reached
volatile boolean reactionTimeUp;  // alerts that reaction time limit is reached for reaction mode

int second;
boolean newBest;
boolean change;

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
LiquidCrystal lcd(42, 43, 44, 45, 46, 47);  // digital pins?

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
  pinMode(outpin0, OUTPUT);                // out pin 0 as output
  pinMode(outpin1, OUTPUT);                // out pin 1 as output
  pinMode(outpin2, OUTPUT);                // out pin 2 as output
  pinMode(outpin3, OUTPUT);                // out pin 3 as output
  pinMode(outpin4, OUTPUT);                // out pin 4 as output
  pinMode(speakerPin, OUTPUT);             // set speaker pin to output
  randomSeed(analogRead(0));               // sets seed from random floating pin 0
  lcd.begin(20, 4);                        // set up the LCD for 20 columns and 4 rows
  
  newBest = false;
  
  pinMode(53, OUTPUT);
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
  go = false;
  change = true;
  timeUp = false;
  reactionTimeUp = false;
  started = false;
  quit = false;
  mode = 0;
  hits = 0;
  misses = 0;
  ledSelect = 0;
  oldLed = 4;
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
      timeLeft = time*60;        // session time limit is time*60 seconds
      displayStartCounter();
    }
    //change = false;
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
  /*if(newBest){
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
    BRT = BT/(BH+BM);*/
  int ART = 0;
  if(hits != 0)
    ART = TRT/(hits+misses);
  
  /*if(ART > 0 && BRT > 0 && ART < BRT){
    saveSession(3,sessionNum, TRT, hits, misses, true);
    BRT = ART;
    newBest = true;
  }*/
  
  int whole = ART/1000;
  int dec = ART%1000;
  //int bwhole = BRT/1000;
  //int bdec = BRT%1000;
  String sn = String(sessionNum);
  String h = String(hits);
  String m = String(misses);
  String art;
  //String brt;
  if(ART > 0){
    art = String(whole);
    art += ".";
    art += String(dec);
    art += " s";
  }
  else
    art = "no hits";
  /*if(BRT > 0)
    brt = String(bwhole);
    brt += ".";
    brt += String(bdec);
  else
    brt = "no best data";*/
  
  lcd.clear();
  lcd.print("Session #: " + sn);
  lcd.setCursor(0,1);
  lcd.print("Hits: " + h);
  lcd.print(", Miss: " + m);
  lcd.setCursor(0,2);
  lcd.print("Reaction: " + art);
  //lcd.setCursor(0,3);
  //lcd.print("Best: " + brt);
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

void turnOffLEDs(){
  digitalWrite(outpin0, HIGH);
  digitalWrite(outpin1, LOW);
  digitalWrite(outpin2, LOW);
  digitalWrite(outpin3, LOW);
  digitalWrite(outpin4, LOW);
}

void playSound(){
  tone(speakerPin, 1000, 250);
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
  else{
    while(digitalRead(bpin3) == HIGH && !reactionTimeUp && !timeUp){
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
          digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW);
    
  }
}

// runs standard mode
// each iteration lights 1 led
void runStandardMode(){
  lightRandomLED();
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
  else{
    while(digitalRead(bpin3) == HIGH && !timeUp){
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
          digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW);
  }
}

void lightRandomLED(){
  //there may be a better way for this
  turnOffLEDs();
  
  // this makes sure a different led will light every time
  //while(ledSelect == oldLed){
  //  ledSelect = random(4);
  //}
  //oldLed = ledSelect;
  ledSelect= (ledSelect+1)%4;
  // light random led
  digitalWrite(outpin0, LOW);
  
  if(ledSelect == 0)
    digitalWrite(outpin1, HIGH);
  else if(ledSelect == 1)
    digitalWrite(outpin2, HIGH);
  else if(ledSelect == 2)
    digitalWrite(outpin3, HIGH);
  else
    digitalWrite(outpin4, HIGH);
}
