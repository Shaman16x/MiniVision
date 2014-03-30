#include <TimerOne.h>
#include <TimerThree.h>
#include <SimpleTimer.h>

SimpleTimer timer;
int counter = 0;
const int lightPin = 46;   //digital

int counter1 = 0;
int counter3 = 0;
boolean light = false;

void miliCounter(){
  counter++;
  if(counter == 5000){
    light = !light;
    counter = 0;
  }
}

void timer1Handler(){
  counter1++;
  if(counter1 == 5){
    light = !light;
    counter1 = 0;
  }
}

void timer3Handler(){
  counter3++;
  if(counter3 == 10){
    light = !light;
    counter3 = 0;
  }
}

void setup(){
  //Timer1.initialize(1000000);              // sets timer1 for every second
  //Timer1.attachInterrupt(timer1Handler);    // sessionTimer updates timeLeft every timer1 interrupt
  //Timer3.initialize(1000000);              // sets timer3 for every second
  //Timer3.attachInterrupt(timer3Handler);   // reactionTimer updates reactionTimeLeft every timer3 interrupt
  timer.setInterval(1,miliCounter);
  
  pinMode(lightPin, OUTPUT);
}

void loop(){
  timer.run();
  if(light)
    digitalWrite(lightPin, HIGH);
  else
    digitalWrite(lightPin, LOW);
}
