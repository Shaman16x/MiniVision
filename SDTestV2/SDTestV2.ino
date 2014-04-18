#include <SD.h>

boolean card;

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
  card = false;
  pinMode(3,INPUT_PULLUP);
  
}

void loop()
{
  delay(1000);
  while(digitalRead(7) == HIGH);
  
  if(digitalRead(3) == HIGH)
    card = false;
  else
    card = true;
  if(card)
    Serial.println("card is there");
  else
    Serial.println("no card");
}
  
