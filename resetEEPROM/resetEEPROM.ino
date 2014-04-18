#include <EEPROM.h>

void setup(){
  for(int i=0; i<4096; i++)
    EEPROM.write(i,0);
}

void loop(){}
