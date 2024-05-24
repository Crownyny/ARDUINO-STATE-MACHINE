
// include the library code:
#include <LiquidCrystal.h>
#include "StateMachineLib.h"
#include "AsyncTaskLib.h"
#include "ImperialMarch.h"
#include <EasyBuzzer.h>
#include <LiquidMenu.h>
#include <Keypad.h>
#include "Util.h"

#define ledR 9
#define ledG 8
#define ledB 7


#define enter 1

#define length 6

size_t timer = 10;


Util util;


AsyncTask TurnOffLed(500, []()
                     { pinMode(ledR, LOW); }); // Turn off the led after 500ms

ImperialMarch march(14, 120); // Pin 11, tempo 120
AsyncTask stopMarch(10000, []()
                    { march.stop(); }); // Play the Imperial March after 10s

bool executeMenu = false;

const char password[length] = "A1234B";
char buffer[length];
int triesCounter = 0;
int failedAttempts = 3;
int charCounter = 0;

int tempHigh = 28;
int tempLow = -16;
int lightHigh = 200;
int lightLow = 800;
int humHigh = 100;
int humLow = 0;
int gasHigh = 1000;
int gasLow = 0;

int newValue = 0;
byte pos = 0;
bool isNegative = false;

// These are the char arrays stored in flash
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


void resetLimits();
void setTempHigh();
void setTempLow();
void setLightHigh();
void setLightLow();
void setHumHigh();
void setHumLow();
void setGasHigh();
void setGasLow();



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

LiquidMenu menu(util.lcd);



// State Alias
enum State
{
    S_INICIO = 0,
    S_BLOQUEADO = 1,
    S_CONFIG = 2,
    S_MEVENTOS = 3,
    S_MAMBIENTAL = 4,
    S_ALARMA = 5
};

// Input Alias
enum Input
{
    TimeOut10 = 0,
    SystBlock = 1,
    CorrectPwd = 2,
    BtnPrs = 3,
    TimeOut2 = 4,
    TimeOut5 = 5,
    GasHigh = 6,
    TempLightHigh = 7,
    Unknown = 8
};

// Create new StateMachine
StateMachine stateMachine(6, 12);

// Stores last user input
Input input;

// Setup the State Machine
void setupStateMachine()
{
    // Add transitions
    stateMachine.AddTransition(S_INICIO, S_BLOQUEADO, []()
                               { return input == SystBlock; });
    stateMachine.AddTransition(S_INICIO, S_CONFIG, []()
                               { return input == CorrectPwd; });

    stateMachine.AddTransition(S_BLOQUEADO, S_INICIO, []()
                               { return input == TimeOut10; });

    stateMachine.AddTransition(S_CONFIG, S_MAMBIENTAL, []()
                               { return input == BtnPrs; });

    stateMachine.AddTransition(S_MEVENTOS, S_CONFIG, []()
                               { return input == BtnPrs; });
    stateMachine.AddTransition(S_MEVENTOS, S_MAMBIENTAL, []()
                               { return input == TimeOut2; });
    stateMachine.AddTransition(S_MEVENTOS, S_ALARMA, []()
                               { return input == GasHigh; });

    stateMachine.AddTransition(S_MAMBIENTAL, S_CONFIG, []()
                               { return input == BtnPrs; });
    stateMachine.AddTransition(S_MAMBIENTAL, S_MEVENTOS, []()
                               { return input == TimeOut5; });
    stateMachine.AddTransition(S_MAMBIENTAL, S_ALARMA, []()
                               { return (input == TempLightHigh); });

    stateMachine.AddTransition(S_ALARMA, S_MAMBIENTAL, []()
                               { return input == TimeOut5; });
    stateMachine.AddTransition(S_ALARMA, S_INICIO, []()
                               { return input == BtnPrs; });

    // Add actions
    stateMachine.SetOnEntering(S_INICIO, outputA);
    stateMachine.SetOnEntering(S_BLOQUEADO, outputB);
    stateMachine.SetOnEntering(S_CONFIG, outputC);
    stateMachine.SetOnEntering(S_MEVENTOS, outputD);
    stateMachine.SetOnEntering(S_MAMBIENTAL, outputE);
    stateMachine.SetOnEntering(S_ALARMA, outputF);

    stateMachine.SetOnLeaving(S_INICIO, []()
                              { Serial.println("Leaving A"); });
    stateMachine.SetOnLeaving(S_BLOQUEADO, []()
                              { Serial.println("Leaving B"); });
    stateMachine.SetOnLeaving(S_CONFIG, []()
                              { Serial.println("Leaving C"); });
    stateMachine.SetOnLeaving(S_MEVENTOS, []()
                              { Serial.println("Leaving D"); });
    stateMachine.SetOnLeaving(S_MAMBIENTAL, []()
                              { Serial.println("Leaving E"); });
    stateMachine.SetOnLeaving(S_ALARMA, []()
                              { Serial.println("Leaving F"); });
}

AsyncTask BlockMsg(1000, true, []()
                   { 
    timer--;
    if(timer == 0)
    {
        timer = 10;
        BlockMsg.Stop();
        input = Input::TimeOut10;
        return;
    }
    util.lcd.setCursor(0, 2);
    util.lcd.cursor(); 
    util.lcd.print("0");     
    util.lcd.print(timer); });

void setup()
{
    Serial.begin(9600);

    pinMode(ledR, LOW);
    pinMode(ledG, LOW);
    pinMode(ledB, LOW);

    TurnOffLed.Start();
    EasyBuzzer.setPin(14);

    Serial.println("Starting State Machine...");
    setupStateMachine();
    Serial.println("Start Machine Started");

    // Initial state
    stateMachine.SetState(S_INICIO, false, true);

    // Set the variables as PROGMEM. The parameter is the consecutive
    // number of the PROGMEM variable for that LiquidLine object.
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

    opt1.attach_function(enter, func);
    opt2.attach_function(enter, resetLimits);
    opt3.attach_function(enter, setTempHigh);
    opt4.attach_function(enter, setTempLow);
    opt5.attach_function(enter, setLightHigh);
    opt6.attach_function(enter, setLightLow);
    opt7.attach_function(enter, setHumHigh);
    opt8.attach_function(enter, setHumLow);
    opt9.attach_function(enter, setGasHigh);
    opt10.attach_function(enter, setGasLow);


    menu.add_screen(scr1);
    menu.add_screen(scr2);
    menu.add_screen(scr3);
    menu.add_screen(scr4);
    menu.add_screen(scr5);

}

void func()
{
    return;
}   

void loop()
{

    // Update State Machine
    stateMachine.Update();

    TurnOffLed.Update();

    EasyBuzzer.update();

    if (executeMenu)
    {
        myMenu();
    }

    if (stopMarch.IsActive())
    {
        stopMarch.Update();
    }

    if (BlockMsg.IsActive())
    {
        BlockMsg.Update();
    }
}

// Auxiliar output functions that show the state debug
void outputA()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("X            ");
    Serial.println();
    util.lcd.clear();
    util.lcd.print("Input Password:");
    while (security())
    {
    }
}

void outputB()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("    x            ");
    Serial.println();
    input = Input::Unknown;
    blocking();
}
void outputC()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("        X    ");
    Serial.println();
    input = Input::Unknown;
    menu.update();
    executeMenu = true;
}
void outputD()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("            X    ");
    Serial.println();
    input = Input::Unknown;
}
void outputE()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("                X   ");
    Serial.println();
    input = Input::Unknown;
}
void outputF()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("                    X");
    Serial.println();
    input = Input::Unknown;
}

bool security()
{
    // Configuramos util.lcd para cada loop
    util.lcd.setCursor(charCounter, 2);
    char key = util.keypad.getKey();
    util.lcd.cursor();

    if (!key)
    {
        return true;
    }

    buffer[triesCounter] = key;
    Serial.print(key);
    util.lcd.print("*");
    triesCounter++;

    if (triesCounter < length)
    {
        charCounter++;
        return true;
    }

    buffer[triesCounter] = '\0';

    if (strcmp(password, buffer) == 0)
    {
        util.print2lines("Access Granted", "Welcome!", 500);
        util.lcd.clear();
        input = Input::CorrectPwd;
        return false;
    }

    failedAttempts++;

    if (failedAttempts >= 3)
    {
        util.lcd.clear();
        input = Input::SystBlock;
        return false;
    }

    util.print2lines("Access Denied", "Try Again", 100);
    triesCounter = 0;
    charCounter = 0;
    util.lcd.clear();
    util.lcd.print("Input Password:");
    return true;
}

void blocking()
{
    pinMode(ledR, HIGH); // Enciende el led y lo apgaa despues de 500ms
    TurnOffLed.Reset();

    EasyBuzzer.singleBeep(120, 10000);

    BlockMsg.Start();
    util.lcd.clear();
    util.lcd.print("System Blocked");
    util.lcd.setCursor(0, 2);
    util.lcd.cursor();
    util.lcd.print(timer);
    util.lcd.setCursor(3, 2);
    util.lcd.cursor();
    util.lcd.print("Seconds left");

    failedAttempts = 0;
    triesCounter = 0;
    charCounter = 0;
}


void myMenu()
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


bool setSensorLimit(const int startNumber, const byte posStart, const int inferiorLimit, const int superiorLimit)
{
    byte digit = 0;
    
    char key = util.keypad.getKey();

    if(!key)
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
    if (key == 'A' && pos == posStart && inferiorLimit <  0)
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

        if ( isNegative && (((newValue * -10) + digit) < inferiorLimit))
        {
            return true;
        }

        if ( !isNegative && ((newValue * 10) + digit) > superiorLimit)
        {
            return true;
        }

        if(pos==posStart)
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

void setTempHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    util.printLineValue(spaces, "Temp High", posStart, tempHigh);
    pos = posStart;
    newValue = 0;
    isNegative = false;
    while (setSensorLimit(tempHigh, posStart, tempLow, 80))
    {
    }
    tempHigh = newValue;
}

void setTempLow()
{
    const byte posStart = 7;
    const byte spaces = 6;
    util.printLineValue(spaces, "Temp Low", posStart, tempLow);
    while (setSensorLimit(tempLow, posStart, -60, tempHigh))
    {
    }
}

void setLightHigh()
{
    const byte posStart = 6;
    const byte spaces = 6;
    util.printLineValue(spaces, "Light High", posStart, lightHigh);
    while (setSensorLimit(lightHigh, posStart, lightLow, 1000))
    {
    }
}

void setLightLow()
{
    const byte posStart = 6;
    const byte spaces = 6;
    util.printLineValue(spaces, "Light Low", posStart, lightLow);
    while (setSensorLimit(lightLow, posStart, 0, lightHigh))
    {
    }
}

void setHumHigh()
{
    const byte posStart = 7;
    const byte spaces = 6;
    util.printLineValue(spaces, "Hum High", posStart, humHigh);
    while (setSensorLimit(humHigh, posStart, humLow, 100))
    {
    }
}

void setHumLow()
{
    const byte posStart = 7;
    const byte spaces = 6;
    util.printLineValue(spaces, "Hum Low", posStart, humLow);
    while (setSensorLimit(humLow, posStart, 0, humHigh))
    {
    }
}

void setGasHigh()
{
    const byte posStart = 5;
    const byte spaces = 6;
    util.printLineValue(spaces, "Gas High", posStart, gasHigh);
    while (setSensorLimit(gasHigh, posStart, gasLow, 10000))
    {
    }
}

void setGasLow()
{
    const byte posStart = 5;
    const byte spaces = 6;
    util.printLineValue(spaces, "Gas Low", posStart, gasLow);
    while (setSensorLimit(gasLow, posStart, 0, gasHigh))
    {
    }
}

void resetLimits()
{
    tempHigh = 80;
    tempLow = -60;
    lightHigh = 1000;
    lightLow = 0;
    humHigh = 100;
    humLow = 0;
    gasHigh = 10000;
    gasLow = 0;
    util.print2lines("Limits Reset", "Successfully", 1000);
    menu.update();
}