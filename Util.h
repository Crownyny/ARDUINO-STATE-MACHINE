#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Keypad.h>



class Util 
{
public:
    Util();
    void print2lines(const char *line1, const char *line2, const int waitTime);
    void printLineValue(const byte spaces, const char *msg, const byte spacesValue, const int value);
    void clearLine(int line);
    LiquidCrystal lcd;
    Keypad keypad;
};

#endif