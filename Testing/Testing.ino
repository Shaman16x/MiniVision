#include <TimerOne.h>
#include <TimerThree.h>

const int bpin0 = 40;        // input button 0 (pin number?)
const int bpin1 = 41;        // input button 0 (pin number?)
const int bpin2 = 42;        // input button 0 (pin number?)
const int bpin3 = 43;        // input button 0 (pin number?)
const int outpin0 = 22;      // output pin to LED and Button (pin number?)
const int outpin1 = 23;      // output pin to LED and Button (pin number?)
const int outpin2 = 24;      // output pin to LED and Button (pin number?)
const int outpin3 = 25;      // output pin to LED and Button (pin number?)
const int outpin4 = 26;      // output pin to LED and Button
const int speakerPin = 2;   // speaker  (pin number?)

int ledSelect;                    // used to select active led
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
      timeUp = true;
      tone(speakerPin, 500, 1000);
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
  pinMode(bpin2, INPUT_PULLUP);            // b pin 2 as input with internal pullup
  pinMode(bpin3, INPUT_PULLUP);            // b pin 3 as input with internal pullup
  pinMode(outpin0, OUTPUT);                // out pin 0 as output
  pinMode(outpin1, OUTPUT);                // out pin 1 as output
  pinMode(outpin2, OUTPUT);                // out pin 2 as output
  pinMode(outpin3, OUTPUT);                // out pin 3 as output
  pinMode(outpin4, OUTPUT);                // out pin 4 as output
  pinMode(speakerPin, OUTPUT);             // set speaker pin to output
  randomSeed(analogRead(0));               // sets seed from random floating pin 0
  
  timeUp = false;
}

void setDefaults(){
  Timer1.stop();
  Timer3.stop();
  //timeUp = false;
  reactionTimeUp = false;
  started = false;
  time = 1;
  mode = 0;
  hits = 0;
  ledSelect = 0;
  oldLed = 4;
}

// 1 loop is full session
void loop(){
  setDefaults();
  while(!timeUp){
    if(!started){
      started = true;
      timeLeft = 30;
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
  Timer1.stop();
  turnOffLEDs();
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
    //TRT += counter;
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
