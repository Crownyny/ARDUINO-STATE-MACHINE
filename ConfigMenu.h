#ifndef CONFIGMENU_H
#define CONFIGMENU_H

#include <Arduino.h>
#include <LiquidMenu.h>
#include "Util.h"

class ConfigMenu
{
public:
    ConfigMenu();  // Constructor
    void myMenu(); // Método para manejar el menú
    void attachFunctions(LiquidLine &opt1, LiquidLine &opt2, LiquidLine &opt3, LiquidLine &opt4, LiquidLine &opt5, LiquidLine &opt6, LiquidLine &opt7, LiquidLine &opt8, LiquidLine &opt9, LiquidLine &opt10);
private:
    int tempHigh;
    int tempLow;
    int humHigh;
    int humLow;
    int lightHigh;
    int lightLow;
    int gasHigh;
    int gasLow;

    int newValue;
    byte pos;
    bool isNegative;

    void setSensorLimit(int &sensorValue, const byte posStart, const int inferiorLimit, const int superiorLimit);
    bool sensorLimitRepeatable(const int startNumber, const byte posStart, const int inferiorLimit, const int superiorLimit);

    void resetLimits();
    void setTempHigh();
    void setTempLow();
    void setHumHigh();
    void setHumLow();
    void setLightHigh();
    void setLightLow();
    void setGasHigh();
    void setGasLow();

    LiquidMenu menu;
};

#endif
