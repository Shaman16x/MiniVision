#include <EEPROM.h>
#include <LiquidCrystal.h>

int num;

//LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LiquidCrystal lcd(42, 43, 44, 45, 46, 47);  // pin setup for LCD

void setup(){
  lcd.begin(20,4);
  pinMode(7,INPUT_PULLUP);
  num = ((int)(((char)EEPROM.read(0)) - '0'))*100 + ((int)(((char)EEPROM.read(1)) - '0'))*10 + 
        ((int)(((char)EEPROM.read(2)) - '0'));
}

void loop(){
  delay(1000);
  while(digitalRead(7) == HIGH);
  num++;
  String n = String(num);
  if(num < 10)
    n = "00" + n;
  else if(num < 100)
    n = "0" + n;
  lcd.clear();
  lcd.print(n);
  EEPROM.write(0,n.charAt(0));
  EEPROM.write(1,n.charAt(1));
  EEPROM.write(2,n.charAt(2));
}
