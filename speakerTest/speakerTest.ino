int speakerPin = 11;

void setup(){
  pinMode(speakerPin, OUTPUT);
}

void loop(){
  tone(speakerPin, 1000, 1000);
}
