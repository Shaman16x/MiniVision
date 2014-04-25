const int outpin0 = 27;
const int outpin1 = 26;
const int outpin2 = 29;
const int outpin3 = 28;
const int outpin4 = 31;

const int outpin5 = 25;
const int outpin6 = 22;
const int outpin7 = 24;
const int outpin8 = 23;

const int bpin0 = 30;             // input button 0 (digital pin number?)
const int bpin1 = 33;             // input button 1 (digital pin number?)
const int bpin2 = 32;             // input button 2 (digital pin number?)
const int bpin3 = 35;             // input button 3 (digital pin number?)
const int bpin4 = 34;             // input button 4 (digital pin number?)
const int speakerPin = 8;         // speaker  (PWM pin number?)

void setup(){
  pinMode(outpin0, OUTPUT);                // out pin 0 as output
  pinMode(outpin1, OUTPUT);                // out pin 1 as output
  pinMode(outpin2, OUTPUT);                // out pin 2 as output
  pinMode(outpin3, OUTPUT);                // out pin 3 as output
  pinMode(outpin4, OUTPUT);                // out pin 4 as output
  
  pinMode(outpin5, OUTPUT);
  pinMode(outpin6, OUTPUT);
  pinMode(outpin7, OUTPUT);
  pinMode(outpin8, OUTPUT);
  
  pinMode(bpin0, INPUT_PULLUP);            // b pin 0 as input with internal pullup
  pinMode(bpin1, INPUT_PULLUP);            // b pin 1 as input with internal pullup
  pinMode(bpin2, INPUT_PULLUP);            // b pin 2 as input with internal pullup
  pinMode(bpin3, INPUT_PULLUP);            // b pin 3 as input with internal pullup
  pinMode(bpin4, INPUT_PULLUP);
  
  pinMode(speakerPin, OUTPUT);
  
  digitalWrite(outpin0,LOW);
  digitalWrite(outpin1,LOW);
  digitalWrite(outpin2,LOW);
  digitalWrite(outpin3,LOW);
  digitalWrite(outpin4,HIGH);
  
  digitalWrite(outpin5,LOW);
  digitalWrite(outpin6,HIGH);
  digitalWrite(outpin7,HIGH);
  digitalWrite(outpin8,HIGH);
  
  
}

void loop(){
  while(digitalRead(bpin4) == HIGH);
  playSound();
}

void playSound(){
  tone(speakerPin, 1000, 250);
}
