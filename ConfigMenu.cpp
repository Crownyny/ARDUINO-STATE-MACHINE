#include "ConfigMenu.h"

#define enter 1

Util util;

LiquidMenu menu(util.lcd);

ConfigMenu::ConfigMenu()
{
    tempHigh = 28;
    tempLow = -16;
    lightHigh = 200;
    lightLow = 800;
    humHigh = 100;
    humLow = 0;
    gasHigh = 1000;
    gasLow = 0;

    const char text1[] PROGMEM = "Env Monitoring";
    const char text2[] PROGMEM = "Reset limits";
    const char text3[] PROGMEM = "TH TEMP HIGH";
    const char text4[] PROGMEM = "TH TEMP LOW";
    const char text5[] PROGMEM = "TH LIGHT HIGH";
    const char text6[] PROGMEM = "TH LIGHT LOW";
    const char text7[] PROGMEM = "TH HUM HIGH";
    const char text8[] PROGMEM = "TH HUM LOW";
    const char text9[] PROGMEM = "TH GAS HIGH";
    const char text10[] PROGMEM = "TH GAS LOW";

    LiquidLine opt1(0, 0, text1);
    LiquidLine opt2(0, 1, text2);
    LiquidScreen scr1(opt1, opt2);

    LiquidLine opt3(0, 0, text3);
    LiquidLine opt4(0, 1, text4);
    LiquidScreen scr2(opt3, opt4);

    LiquidLine opt5(0, 0, text5);
    LiquidLine opt6(0, 1, text6);
    LiquidScreen scr3(opt5, opt6);

    LiquidLine opt7(0, 0, text7);
    LiquidLine opt8(0, 1, text8);
    LiquidScreen scr4(opt7, opt8);

    LiquidLine opt9(0, 0, text9);
    LiquidLine opt10(0, 1, text10);
    LiquidScreen scr5(opt9, opt10);

    opt1.set_asProgmem(1);
    opt2.set_asProgmem(1);
    opt3.set_asProgmem(1);
    opt4.set_asProgmem(1);
    opt5.set_asProgmem(1);
    opt6.set_asProgmem(1);
    opt7.set_asProgmem(1);
    opt8.set_asProgmem(1);
    opt9.set_asProgmem(1);
    opt10.set_asProgmem(1);

    attachFunctions(opt1, opt2, opt3, opt4, opt5, opt6, opt7, opt8, opt9, opt10);

    menu.add_screen(scr1);
    menu.add_screen(scr2);
    menu.add_screen(scr3);
    menu.add_screen(scr4);
    menu.add_screen(scr5);
}

void ConfigMenu::attachFunctions(LiquidLine &opt1, LiquidLine &opt2, LiquidLine &opt3, LiquidLine &opt4, LiquidLine &opt5, LiquidLine &opt6, LiquidLine &opt7, LiquidLine &opt8, LiquidLine &opt9, LiquidLine &opt10) {
    opt1.attach_function(enter, [this]() { this->resetLimits(); });
    opt2.attach_function(enter, [this]() { this->resetLimits(); });
    opt3.attach_function(enter, [this]() { this->setTempHigh(); });
    opt4.attach_function(enter, [this]() { this->setTempLow(); });
    opt5.attach_function(enter, [this]() { this->setLightHigh(); });
    opt6.attach_function(enter, [this]() { this->setLightLow(); });
    opt7.attach_function(enter, [this]() { this->setHumHigh(); });
    opt8.attach_function(enter, [this]() { this->setHumLow(); });
    opt9.attach_function(enter, [this]() { this->setGasHigh(); });
    opt10.attach_function(enter, [this]() { this->setGasLow(); });
}

void ConfigMenu::myMenu()
{
    char key = util.keypad.getKey();

    if (key == 'B')
    {
        menu.next_screen();
        return;
    }

    if (key == 'A')
    {
        menu.previous_screen();
        return;
    }

    if (key == '#')
    {
        menu.call_function(enter);
        return;
    }

    if (key == '*')
    {
        menu.switch_focus();
        return;
    }
}

bool ConfigMenu::sensorLimitRepeatable(const int startNumber, const byte posStart, const int inferiorLimit, const int superiorLimit)
{
    byte digit = 0;

    char key = util.keypad.getKey();

    if (!key)
    {
        return true;
    }

    // Check if the key is 0 and the position is the start position
    if (key == '0' && (pos > posStart || (isNegative && pos > posStart + 1)))
    {
        return true;
    }

    if (key == '0')
    {
        util.lcd.setCursor(pos, 2);
        util.lcd.cursor();
        util.lcd.print("0");
        return true;
    }

    // Check if the key is A(-) and the position is the start position and the number is negative
    if (key == 'A' && pos == posStart && inferiorLimit < 0)
    {
        isNegative = true;
        util.clearLine(1);
        util.lcd.setCursor(pos, 2);
        util.lcd.cursor();
        util.lcd.print("-");
        pos++;
        return true;
    }

    // Check if the key is a number between 1 and 9
    if (key > '0' && key <= '9')
    {
        digit = key - '0';

        if (isNegative && (((newValue * -10) + digit) < inferiorLimit))
        {
            return true;
        }

        if (!isNegative && ((newValue * 10) + digit) > superiorLimit)
        {
            return true;
        }

        if (pos == posStart)
        {
            util.clearLine(1);
        }

        newValue = newValue * 10 + digit;

        util.lcd.setCursor(pos, 2);
        util.lcd.cursor();
        util.lcd.print(key);

        pos++;
        return true;
    }

    // Check if the key is B (Backspace) and the position is the start position (negative case)
    if (key == 'B' && isNegative && pos == posStart + 1)
    {
        isNegative = false;
        pos--;
        util.lcd.setCursor(pos, 2);
        util.lcd.cursor();
        util.lcd.print(" ");
        return true;
    }

    // Check if the key is B (Backspace) and the position is greater than the start position
    if (key == 'B' && pos > posStart)
    {
        pos--;

        newValue = newValue / 10;

        util.lcd.setCursor(pos, 2);
        util.lcd.cursor();
        util.lcd.print(" ");
        return true;
    }

    // Check if the key is # (Enter) and the position is greater than the start position
    if (key == '#' && pos > 7)
    {
        menu.update();
        if (isNegative)
        {
            newValue *= -1;
        }
        return false;
    }

    // Check if the key is * (Back) and the position is greater than the start position
    if (key == '*')
    {
        menu.update();
        newValue = startNumber;
        return false;
    }

    return true;
}

void ConfigMenu::setSensorLimit(int &sensorValue, const byte posStart, const int inferiorLimit, const int superiorLimit)
{
    if (superiorLimit < inferiorLimit)
    {
        return;
    }

    pos = posStart;
    newValue = 0;
    isNegative = false;

    while (sensorLimitRepeatable(sensorValue, posStart, inferiorLimit, superiorLimit))
    {
    }

    sensorValue = newValue;
}

void ConfigMenu::setTempHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Temp High", posStart, tempHigh);
    setSensorLimit(tempHigh, posStart, tempLow, 50);
}

void ConfigMenu::setTempLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Temp Low", posStart, tempLow);
    setSensorLimit(tempLow, posStart, -50, tempHigh);
}

void ConfigMenu::setLightHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Light High", posStart, lightHigh);
    setSensorLimit(lightHigh, posStart, lightLow, 1000);
}

void ConfigMenu::setLightLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Light Low", posStart, lightLow);
    setSensorLimit(lightLow, posStart, 0, lightHigh);
}

void ConfigMenu::setHumHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Hum High", posStart, humHigh);
    setSensorLimit(humHigh, posStart, humLow, 100);
}

void ConfigMenu::setHumLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Hum Low", posStart, humLow);
    setSensorLimit(humLow, posStart, 0, humHigh);
}

void ConfigMenu::setGasHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Gas High", posStart, gasHigh);
    setSensorLimit(gasHigh, posStart, gasLow, 2000);
}

void ConfigMenu::setGasLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Gas Low", posStart, gasLow);
    setSensorLimit(gasLow, posStart, 0, gasHigh);
}

void ConfigMenu::resetLimits()
{
    tempHigh = 28;
    tempLow = -16;
    lightHigh = 200;
    lightLow = 800;
    humHigh = 100;
    humLow = 0;
    gasHigh = 1000;
    gasLow = 0;
}   