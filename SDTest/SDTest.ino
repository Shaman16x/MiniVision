#include <SD.h>

File myFile;

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(53, OUTPUT);

  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");
  }

  // open a new file and immediately close it:
  myFile = myFile = SD.open("example.txt", FILE_WRITE);
  if(myFile){
    myFile.println("hello");
    myFile.close();
  }
  
  myFile = SD.open("example.txt");
  if(myFile){
  while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    myFile.close();
  }
  else{
    Serial.println("this did not work");
  }
  
  

  // Check to see if the file exists: 
  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");  
  }
}

void loop()
{
  // nothing happens after setup finishes.
}
