#include <TimerOne.h>
#include <TimerThree.h>

const int bpin0 = 0;        // input button 0 (pin number?)
const int bpin1 = 0;        // input button 0 (pin number?)
const int outpin0 = 0;      // output pin to LED and Button (pin number?)
const int outpin1 = 0;      // output pin to LED and Button (pin number?)
const int outpin2 = 0;      // output pin to LED and Button (pin number?)
const int outpin3 = 0;      // output pin to LED and Button (pin number?)
const int speakerPin = 0;   // speaker  (pin number?)

int armSelect;                    // used to select active arm
int ledSelect;                    // used to select active led
int oldArm;                       // used to not light same led twice in a row
int oldLed;                       // used to not light same led twice in a row
volatile int timeLeft;            // used for session timer
volatile int reactionTimeLeft;    // used for reaction timer
int hits;                         // number of correct hits
int time;                         // time setting
int mode;                         // mode setting
boolean started;                  // alerts that session has started
volatile boolean timeUp;          // alerts that time limit is reached
volatile boolean reactionTimeUp;  // alerts that reaction time limit is reached for reaction mode

void sessionTimer(){
  timeLeft--;
  //updateDisplay();
  if(timeLeft == 0){
    if(!timeUp){
      noInterrupts();
      timeUp = true;
    }
  }
}

void reactionTimer(){
  reactionTimeLeft--;
  if(reactionTimeLeft == 0)
    reactionTimeUp = true;
}
void setup() {
  Timer1.initialize(1000000);              // sets timer1 for every second
  Timer1.attachInterrupt(sessionTimer);    // sessionTimer updates timeLeft every timer1 interrupt
  Timer3.initialize(1000000);              // sets timer3 for every second
  Timer3.attachInterrupt(reactionTimer);   // reactionTimer updates reactionTimeLeft every timer3 interrupt
  pinMode(bpin0, INPUT_PULLUP);            // b pin 0 as input with internal pullup
  pinMode(bpin1, INPUT_PULLUP);            // b pin 1 as input with internal pullup
  pinMode(outpin0, OUTPUT);                // out pin 0 as output
  pinMode(outpin1, OUTPUT);                // out pin 1 as output
  pinMode(outpin2, OUTPUT);                // out pin 2 as output
  pinMode(outpin3, OUTPUT);                // out pin 3 as output
  pinMode(speakerPin, OUTPUT);             // set speaker pin to output
  randomSeed(analogRead(0));               // sets seed from random floating pin 0
  
  Serial.begin(9600);
}

void setDefaults(){
  noInterrupts();
  timeUp = false;
  reactionTimeUp = false;
  started = false;
  time = 1;
  mode = 0;
  hits = 0;
  armSelect = 0;
  ledSelect = 0;
  oldArm = 2;
  oldLed = 2;
  Timer1.stop();
  Timer3.stop();
}

// 1 loop is full session
void loop(){
  setDefaults();
  while(!timeUp){
    if(!started){
      started = true;
      timeLeft = time*60;        // session time limit is time*60 seconds
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
}

void playSound(){
  tone(speakerPin, 1000, 50);
}


void runReactionMode(){
  lightRandomLED();
  
  Timer3.start();
  // check for correct button
  while(digitalRead(bpin0) == HIGH && digitalRead(bpin1) == HIGH && !reactionTimeUp)
    delay(20);
  Timer3.stop();
  if(!reactionTimeUp){
    hits++;
    playSound();
  }
  delay(20);
  while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW)
    delay(20);
}

void runStandardMode(){
  lightRandomLED();
  
  // check for correct button
  while(digitalRead(bpin0) == HIGH && digitalRead(bpin1) == HIGH)
    delay(20);
  hits++;
  playSound();
  delay(20);
  while(digitalRead(bpin0) == LOW || digitalRead(bpin1) == LOW)
    delay(20);
}

void lightRandomLED(){
  //there may be a better way for this
  digitalWrite(outpin0, HIGH);
  digitalWrite(outpin1, HIGH);
  digitalWrite(outpin2, LOW);
  digitalWrite(outpin3, LOW);
  
  // this makes sure a different led will light every time
  while(armSelect == oldArm && ledSelect == oldLed){
    armSelect = random(2);
    ledSelect = random(2);
  }
  oldArm = armSelect;
  oldLed = ledSelect;
  
  // light random led
  if(armSelect == 0)
    digitalWrite(outpin0, LOW);
  else if(armSelect == 1)
    digitalWrite(outpin1, LOW);
  
  if(ledSelect == 0)
    digitalWrite(outpin2, HIGH);
  else if(ledSelect == 1)
    digitalWrite(outpin3, HIGH);
}