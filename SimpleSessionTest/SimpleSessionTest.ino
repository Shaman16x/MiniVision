#include <LiquidCrystal.h>
#include <SimpleTimer.h>
#include <TimerOne.h>
#include <TimerThree.h>

SimpleTimer timer;

const int bpin0 = 0;        // input button 0 (digital pin number?)
const int bpin1 = 0;        // input button 0 (digital pin number?)
const int bpin2 = 0;        // input button 0 (digital pin number?)
const int bpin3 = 0;        // input button 0 (digital pin number?)
const int outpin0 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin1 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin2 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin3 = 0;      // output pin to LED and Button (digital pin number?)
const int outpin4 = 0;      // output pin to LED and Button (digital pin number?)
const int timePin = 0;      // time button (digital pin number?)
const int modePin = 0;      // mode button (digital pin number?)
const int goPin = 0;        // go button   (digital pin number?)
const int quitPin = 0;      // quit button (digital pin number?)
const int reactionPin = 0;  // reaction timer buttom (digital pin number?)
const int speakerPin = 0;   // speaker  (PWM pin number?)

int TRT;                          // used to count reaction times
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
volatile boolean quit;            // alerts that session has been quit
boolean go;                       // alerts that session can begin
volatile boolean timeUp;          // alerts that time limit is reached
volatile boolean reactionTimeUp;  // alerts that reaction time limit is reached for reaction mode

boolean newBest;
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
  timer.setInterval(1,miliCounter);
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
  pinMode(speakerPin, OUTPUT);             // set speaker pin to output
  randomSeed(analogRead(0));               // sets seed from random floating pin 0
  lcd.begin(20, 4);                        // set up the LCD for 20 columns and 4 rows
  
  newBest = false;
  
  pinMode(53, OUTPUT);
  Serial.begin(9600);
}

void miliCounter(){
  counter++;
}

// timer1 interrupt handler
// updates session time and displays every second
// if session is over is the timer for viewing results
void sessionTimer(){
  timeLeft--;
  updateDisplay();
  if(timeLeft == 0){
    if(!timeUp){
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
  go = false;
  timeUp = false;
  reactionTimeLeft = 3;
  reactionTimeUp = false;
  started = false;
  quit = false;
  time = 1;
  mode = 0;
  hits = 0;
  misses = 0;
  ledSelect = 0;
  oldLed = 4;
  TRT = 0;
  Timer1.stop();
  Timer3.stop();
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
      Timer1.start();
    }
    timer.run();
    counter = 0;
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
  
  if(mode == 1){
    if(digitalRead(reactionPin) == LOW){
      reactionTimeLeft += 1;
      if(reactionTimeLeft > 3)
        reactionTimeLeft = 1;
      delay(20);
      while(digitalRead(reactionPin) == LOW)
        delay(20);
    }
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
    if(mode == 1){
      lcd.setCursor(0,2);
      String s4 = "Reaction Time: ";
      String s5 = s4 + reactionTimeLeft + " sec(s)";
      lcd.print(s5);
    }
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
  if(hits+misses != 0)
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
  String art;
  //String brt;
  if(ART > 0)
    art = whole + "." + dec;
  else
    art = "no hits";
  /*if(BRT > 0)
    brt = bwhole + "." + bdec;
  else
    brt = "no best data";*/
  
  lcd.clear();
  lcd.print("Session #: " + sn);
  lcd.setCursor(0,1);
  lcd.print("Hits: " + h);
  lcd.setCursor(0,2);
  lcd.print("Reaction: " + art);
  //lcd.setCursor(0,3);
  //lcd.print("Best: " + brt);
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
  
  Timer3.start();
  // check for correct button
  if(ledSelect == 0){
    while(digitalRead(bpin0) == HIGH && !reactionTimeUp && !timeUp)
      delay(20);
  }
  else if(ledSelect == 1){
    while(digitalRead(bpin1) == HIGH && !reactionTimeUp && !timeUp)
      delay(20);
  }
  else if(ledSelect == 2){
    while(digitalRead(bpin2) == HIGH && !reactionTimeUp && !timeUp)
      delay(20);
  }
  else{
    while(digitalRead(bpin3) == HIGH && !reactionTimeUp && !timeUp)
      delay(20);
  }
  if(!timeUp){
    TRT += counter;
    Timer3.stop();
    if(!reactionTimeUp){
      hits++;
      playSound();
    }
    else{
      misses++;
    }
    delay(20);
    while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW ||
          digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW)
      delay(20);
  }
}

// runs standard mode
// each iteration lights 1 led
void runStandardMode(){
  lightRandomLED();
  
  // check for correct button
  if(ledSelect == 0){
    while(digitalRead(bpin0) == HIGH && !timeUp)
      delay(20);
  }
  else if(ledSelect == 1){
    while(digitalRead(bpin1) == HIGH && !timeUp)
      delay(20);
  }
  else if(ledSelect == 2){
    while(digitalRead(bpin2) == HIGH && !timeUp)
      delay(20);
  }
  else{
    while(digitalRead(bpin3) == HIGH && !timeUp)
      delay(20);
  }
  if(!timeUp){
    TRT += counter;
    hits++;
    playSound();
    delay(20);
    while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW ||
          digitalRead(bpin2) == LOW || digitalRead(bpin3) == LOW)
      delay(20);
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
