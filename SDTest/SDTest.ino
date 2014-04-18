#include <SD.h>

boolean card,initial;
File myFile;

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(53, OUTPUT);
  pinMode(7,INPUT_PULLUP);
  pinMode(3,INPUT_PULLUP);
  card = false;
  initial = false;
  
}

void loop()
{
  delay(1000);
  while(digitalRead(7) == HIGH);//{
    //if(card){
      //if(digitalRead(3) == HIGH)
        //card = false;
    //}
  //}
  
  
  
  if(!initial){
    Serial.print("Initializing SD card...");
  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  initial = true;
  card = true;
  Serial.println("initialization done.");
  }
  
  if(card){
  if(!SD.exists("test.txt")){
    Serial.println("no file");
  }
  myFile = SD.open("test.txt", FILE_WRITE);
  if(myFile){
    myFile.println("hello");
    myFile.close();
    Serial.println("wrote to card");
  }
  else
    Serial.println("file not found");
  }
  else
    Serial.println("do not remove card");
}
