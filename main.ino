
// include the library code:
#include "StateMachineLib.h"
#include "AsyncTaskLib.h"
#include "ImperialMarch.h"
#include <EasyBuzzer.h>
#include "ConfigMenu.h"
#include "Util.h"

#define ledR 9
#define ledG 8
#define ledB 7

#define length 6

size_t timer = 10;

Util util;
ConfigMenu menu();

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
}

void loop()
{
    // Update State Machine
    stateMachine.Update();

    TurnOffLed.Update();

    EasyBuzzer.update();

    if (executeMenu)
    {
        menu.myMenu();
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
