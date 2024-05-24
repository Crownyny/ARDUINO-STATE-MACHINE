#include "Util.h"

// Pin mapping for the display
#define rs 12
#define en 11
#define d4 5
#define d5 4
#define d6 3
#define d7 2

// Pin mapping for the keypad
const byte ROWS = 4; // four rows
const byte COLS = 4; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {22, 24, 26, 28}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {30, 32, 34, 36}; // connect to the column pinouts of the keypad


Util::Util() : lcd(rs, en, d4, d5, d6, d7), Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS)
{
    lcd.begin(16, 2);
}

void Util::print2lines(const char *line1, const char *line2, const int waitTime)
{
    lcd.clear();
    lcd.print(line1);
    lcd.setCursor(0, 2);
    lcd.cursor();
    lcd.print(line2);
    delay(waitTime);
}

void Util::printLineValue(const byte spaces, const char *msg, const byte spacesValue, const int value)
{
    lcd.clear();
    lcd.setCursor(spaces, 0);
    lcd.cursor();
    lcd.print(msg);
    lcd.setCursor(spacesValue, 1);
    lcd.cursor();
    lcd.print(value);
}

void Util::clearLine(int line)
{
    lcd.setCursor(0, line);
    lcd.cursor();
    for (int i = 0; i < 16; i++)
    {
        lcd.print(" ");
    }
    lcd.setCursor(0, line); 
}