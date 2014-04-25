#include <LiquidCrystal.h>

//LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LiquidCrystal lcd(37, 36, 43, 42, 45, 44);  // pin setup for LCD

void setup(){
  lcd.begin(20, 4);  // set up the LCD for 20 columns and 4 rows
  lcd.clear();
  lcd.print("line 1");
  lcd.setCursor(0,1);
  lcd.print("line 2");
  lcd.setCursor(0,2);
  lcd.print("line 3");
  lcd.setCursor(0,3);
  lcd.print("line 4");
  
}

void loop(){
  
}
