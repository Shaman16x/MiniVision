#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 9, 8, 7, 6, 5, 4, 3, 2);  // pin setup for LCD

void setup(){
  lcd.begin(20, 4);  // set up the LCD for 16 columns and 4 rows
  lcd.print("line 1");
  lcd.setCursor(0,1);
  lcd.print("line 2");
  lcd.setCursor(0,2);
  lcd.print("line 3");
  lcd.setCursor(0,3);
  lcd.print("line 4");
}

void loop(){}
