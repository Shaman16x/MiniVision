#include <TimerOne.h>
#include <TimerThree.h>

const int lightPin = 0;
const int soundPin = 0;

int counter1 = 0;
int counter3 = 0;
boolean light = false;
boolean sound = false;

void timer1Handler(){
  counter1++;
  if(counter1 == 30){
    light = !light;
    counter1 = 0;
  }
}

void timer3Handler(){
  counter3++;
  if(counter3 == 10){
    sound = true;
    counter3 = 0;
  }
}

void setup(){
  Timer1.initialize(1000000);              // sets timer1 for every second
  Timer1.attachInterrupt(timer1Handler);    // sessionTimer updates timeLeft every timer1 interrupt
  Timer3.initialize(1000000);              // sets timer3 for every second
  Timer3.attachInterrupt(timer3Handler);   // reactionTimer updates reactionTimeLeft every timer3 interrupt
}

void loop(){
  if(light);
    // light
  else;
    // turn off
  if(sound){
    // play sound .5 sec
    sound = false;
  }
}
