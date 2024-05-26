
// include the library code:
#include <LiquidCrystal.h>
#include "StateMachineLib.h"
#include <Keypad.h>
#include "AsyncTaskLib.h"
#include <EasyBuzzer.h>
#include <LiquidMenu.h>
#include <DHT.h>

#define ledR 9
#define ledG 8
#define ledB 7

#define buttonPin 15

#define rs 12
#define en 11
#define d4 5
#define d5 4
#define d6 3
#define d7 2

#define enter 1

#define length 6

#define DHTPIN 10
#define DHTTYPE DHT11   

#define photocellPin A0
#define photocellPinGas A1

#define intervals 9

DHT dht(DHTPIN, DHTTYPE);

size_t timer = 10;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte ROWS = 4; // four rows
const byte COLS = 4; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {22, 24, 26, 28}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {30, 32, 34, 36}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); // create the keypad

AsyncTask TurnOffLedR(500, []()
                     { pinMode(ledR, LOW); }); // Turn off the led after 500ms

AsyncTask TurnOffLedB(800, []()
                    { pinMode(ledB,LOW);});

ImperialMarch march(14, 120); // Pin 11, tempo 120
AsyncTask stopMarch(10000, []()
                    { march.stop(); }); // Play the Imperial March after 10s

bool executeMenu;
bool executeAlarm;

bool flgCheckTLAlarm ;
bool flgCheckGasAlarm ;

const char password[length] = "A1234B";
char buffer[length];
int triesCounter = 0;
int failedAttempts = 0;
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

bool buttonState;

int outputValue = 0;


// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
float h;
// Read temperature as Celsius (the default)
float t;

int l;

int g;

float tempBuffer[intervals];
float lightBuffer[intervals];
int gasBuffer[intervals];

int tempLightIndex = 0;
int gasIndex = 0;

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

LiquidMenu menu(lcd);


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

AsyncTask BlockSequenceMsg(1000, true, []()
                   { 
    timer--;
    if(timer == 0)
    {
        timer = 10;
        input = Input::TimeOut10;
        return;
    }
    lcd.setCursor(0, 2);
    lcd.cursor(); 
    lcd.print("0");     
    lcd.print(timer); });

AsyncTask updateMonAmbiental(1000,true, []()
{
    timer++;

    if(timer == 5)
    {
        input = Input::TimeOut5;
        return;
    }

    buttonState = digitalRead(buttonPin);

    if (buttonState == HIGH){
        buttonState = LOW;
        input = Input::BtnPrs;
        return;
    } 

    h = dht.readHumidity();
    t = dht.readTemperature();

    l = analogRead(photocellPin) / 4;

    lcd.setCursor(4,0);
    lcd.cursor();
    lcd.print("    ");
    lcd.setCursor(4,0);
    lcd.cursor();
    lcd.print((int)h);
    
    lcd.setCursor(12,0);
    lcd.cursor();
    lcd.print("    ");
    lcd.setCursor(12,0);
    lcd.cursor();
    lcd.print((int)t);
    
    lcd.setCursor(4,1);
    lcd.cursor();
    lcd.print("    ");
    lcd.setCursor(4,1);
    lcd.cursor();
    lcd.print(l);

    // Actualizar los buffers
    tempBuffer[tempLightIndex] = t;
    lightBuffer[tempLightIndex] = l;

    if(tempLightIndex+1 == intervals)
    {
        flgCheckTLAlarm = true;
    }
    
    tempLightIndex = (tempLightIndex + 1) % intervals;

    if(!flgCheckTLAlarm)
    {
        return;
    }

    // Calcular promedios
    float tempSum = 0;
    float lightSum = 0;

    for (int i = 0; i < intervals; i++) {
        tempSum += tempBuffer[i];
        lightSum += lightBuffer[i];
    }
    float tempAvg = tempSum / intervals;
    float lightAvg = lightSum / intervals;

    if (tempAvg >= tempHigh && lightAvg >= lightHigh) {
        tempLightIndex = 0;
        flgCheckTLAlarm = false;
        input = Input::TempLightHigh;
        return;
    }
});

AsyncTask updateMonEventos(1000, true, []()
{
    timer++;

    if(timer == 2)
    {
        input = Input::TimeOut2;
        return;
    }

    buttonState = digitalRead(buttonPin);

    if (buttonState == HIGH){
        buttonState = LOW;
        input = Input::BtnPrs;
        return;
    } 

    g = analogRead(photocellPinGas);

    lcd.setCursor(4,0);
    lcd.cursor();
    lcd.print("    ");
    lcd.setCursor(4,0);
    lcd.cursor();
    lcd.print(g);

    gasBuffer[gasIndex] = g;

    if(gasIndex+1 == intervals)
    {
        flgCheckGasAlarm = true;
    }

    gasIndex = (gasIndex + 1) % intervals;

    if(!flgCheckGasAlarm)
    {
        return;
    }

    int gasSum = 0;

    for (int i = 0; i < intervals; i++) {
        gasSum += gasBuffer[i];
    }

    float gasAvg = gasSum / intervals;

    if (gasAvg >= gasHigh) {
        flgCheckGasAlarm = false;
        gasIndex = 0;
        input = Input::GasHigh;
        return;
    }
});

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
                               { return input == TempLightHigh; });


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
                              { Serial.println("Leaving B"); 
                                BlockSequenceMsg.Stop();});
    stateMachine.SetOnLeaving(S_CONFIG, []()
                              { Serial.println("Leaving C"); });
    stateMachine.SetOnLeaving(S_MEVENTOS, []()
                              { Serial.println("Leaving D"); 
                                updateMonEventos.Stop();});
    stateMachine.SetOnLeaving(S_MAMBIENTAL, []()
                              { Serial.println("Leaving E"); 
                                updateMonAmbiental.Stop();});
    stateMachine.SetOnLeaving(S_ALARMA, []()
                              { Serial.println("Leaving F"); 
                                EasyBuzzer.stopBeep();});
}

void setup()
{
    lcd.begin(16, 2);
    Serial.begin(9600);
    dht.begin();

    pinMode(ledR, LOW);
    pinMode(ledG, LOW);
    pinMode(ledB, LOW);

    pinMode(buttonPin, INPUT);  // Button pin as an input.

    TurnOffLedR.Start();
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

    opt1.attach_function(enter, gotoMonAmbiental);
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


void loop()
{

    // Update State Machine
    stateMachine.Update();

    TurnOffLedR.Update();
    TurnOffLedB.Update();

    EasyBuzzer.update();

    if(updateMonAmbiental.IsActive())
    {
        updateMonAmbiental.Update();
    }

    if(updateMonEventos.IsActive())
    {
        updateMonEventos.Update();
    }

    if (executeMenu)
    {
        myMenu();
    }

    if (executeAlarm)
    {
        turnOnAlarm();
    }

    if (stopMarch.IsActive())
    {
        stopMarch.Update();
    }

    if (BlockSequenceMsg.IsActive())
    {
        BlockSequenceMsg.Update();
    }
}

// Auxiliar output functions that show the state debug
void outputA()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("X            ");
    Serial.println();
    lcd.clear();
    lcd.print("Input Password:");
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
    showMonEventos();
}
void outputE()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("                X   ");
    Serial.println();
    input = Input::Unknown;
    showMonAmbiental();
}
void outputF()
{
    Serial.println("A   B   C   D   E   F");
    Serial.println("                    X");
    Serial.println();

    pinMode(ledB, HIGH);
    TurnOffLedB.Start();
    executeAlarm=true;
    EasyBuzzer.beep(15);
    print2lines("Alarm Activated", "Press Button", 100);
}

bool security()
{
    // Configuramos lcd para cada loop
    lcd.setCursor(charCounter, 2);
    char key = keypad.getKey();
    lcd.cursor();

    if (!key)
    {
        return true;
    }

    buffer[triesCounter] = key;
    Serial.print(key);
    lcd.print("*");
    triesCounter++;

    if (triesCounter < length)
    {
        charCounter++;
        return true;
    }

    buffer[triesCounter] = '\0';

    if (strcmp(password, buffer) == 0)
    {
        print2lines("Access Granted", "Welcome!", 500);
        lcd.clear();
        input = Input::CorrectPwd;
        return false;
    }

    failedAttempts++;

    if (failedAttempts >= 3)
    {
        lcd.clear();
        input = Input::SystBlock;
        return false;
    }

    print2lines("Access Denied", "Try Again", 100);
    triesCounter = 0;
    charCounter = 0;
    lcd.clear();
    lcd.print("Input Password:");
    return true;
}

void blocking()
{
    pinMode(ledR, HIGH); // Enciende el led y lo apgaa despues de 500ms
    TurnOffLedR.Reset();

    EasyBuzzer.singleBeep(120, 10000);

    BlockSequenceMsg.Start();
    lcd.clear();
    lcd.print("System Blocked");
    lcd.setCursor(0, 2);
    lcd.cursor();
    lcd.print(timer);
    lcd.setCursor(3, 2);
    lcd.cursor();
    lcd.print("Seconds left");

    failedAttempts = 0;
    triesCounter = 0;
    charCounter = 0;
}

void print2lines(const char *line1, const char *line2, const int waitTime)
{
    lcd.clear();
    lcd.print(line1);
    lcd.setCursor(0, 2);
    lcd.cursor();
    lcd.print(line2);
    delay(waitTime);
}

void myMenu()
{
    char key = keypad.getKey();

    buttonState = digitalRead(buttonPin);

    if (buttonState == HIGH){
        buttonState = LOW;
        gotoMonAmbiental();
        return;
    } 


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

void printLineValue(const byte spaces, const char *msg, const byte spacesValue, const int value)
{
    lcd.clear();
    lcd.setCursor(spaces, 0);
    lcd.cursor();
    lcd.print(msg);
    lcd.setCursor(spacesValue, 1);
    lcd.cursor();
    lcd.print(value);
}

void clearLine(int line)
{
    lcd.setCursor(0, line);
    lcd.cursor();
    for (int i = 0; i < 16; i++)
    {
        lcd.print(" ");
    }
    lcd.setCursor(0, line); // Vuelve a colocar el cursor al inicio de la línea
}

bool sensorLimitRepeatable(const int startNumber, const byte posStart, const int inferiorLimit, const int superiorLimit)
{
    byte digit = 0;

    char key = keypad.getKey();

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
        lcd.setCursor(pos, 2);
        lcd.cursor();
        lcd.print("0");
        return true;
    }

    // Check if the key is A(-) and the position is the start position and the number is negative
    if (key == 'A' && pos == posStart && inferiorLimit < 0)
    {
        isNegative = true;
        clearLine(1);
        lcd.setCursor(pos, 2);
        lcd.cursor();
        lcd.print("-");
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
            clearLine(1);
        }

        newValue = newValue * 10 + digit;

        lcd.setCursor(pos, 2);
        lcd.cursor();
        lcd.print(key);

        pos++;
        return true;
    }

    // Check if the key is B (Backspace) and the position is the start position (negative case)
    if (key == 'B' && isNegative && pos == posStart + 1)
    {
        isNegative = false;
        pos--;
        lcd.setCursor(pos, 2);
        lcd.cursor();
        lcd.print(" ");
        return true;
    }

    // Check if the key is B (Backspace) and the position is greater than the start position
    if (key == 'B' && pos > posStart)
    {
        pos--;

        newValue = newValue / 10;

        lcd.setCursor(pos, 2);
        lcd.cursor();
        lcd.print(" ");
        return true;
    }

    // Check if the key is # (Enter) and the position is greater than the start position
    if (key == '#' && ( (!isNegative && pos > posStart) || (isNegative && pos > posStart + 1) ))
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

bool setSensorLimit(int &sensorValue, const byte posStart, const int inferiorLimit, const int superiorLimit)
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

void setTempHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Temp High", posStart, tempHigh);
    setSensorLimit(tempHigh, posStart, tempLow, 50);
}

void setTempLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Temp Low", posStart, tempLow);
    setSensorLimit(tempLow, posStart, -50, tempHigh);
}

void setLightHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Light High", posStart, lightHigh);
    setSensorLimit(lightHigh, posStart, lightLow, 1000);
}

void setLightLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Light Low", posStart, lightLow);
    setSensorLimit(lightLow, posStart, 0, lightHigh);
}

void setHumHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Hum High", posStart, humHigh);
    setSensorLimit(humHigh, posStart, humLow, 100);
}

void setHumLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Hum Low", posStart, humLow);
    setSensorLimit(humLow, posStart, 0, humHigh);
}

void setGasHigh()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Gas High", posStart, gasHigh);
    setSensorLimit(gasHigh, posStart, gasLow, 2000);
}

void setGasLow()
{
    const byte posStart = 6;
    const byte spaces = 3;
    printLineValue(spaces, "Gas Low", posStart, gasLow);
    setSensorLimit(gasLow, posStart, 0, gasHigh);
}

void resetLimits()
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

void gotoMonAmbiental()
{
    input = Input::BtnPrs;
    executeMenu = false;
}

void showMonAmbiental()
{
    lcd.clear();
    lcd.print(F("TEM:"));
    lcd.setCursor(8, 0);
    lcd.print(F("HUM:"));
    lcd.setCursor(0, 1);
    lcd.print(F("LUZ:"));
    
    h = dht.readHumidity();
    t = dht.readTemperature();

    lcd.setCursor(4,0);
    lcd.cursor();
    lcd.print((int)h);
    
    lcd.setCursor(12,0);
    lcd.cursor();
    lcd.print((int)t);
    
    lcd.setCursor(4,1);
    lcd.cursor();
    lcd.print(analogRead(photocellPin) / 4);

    timer=0;
    updateMonAmbiental.Start();
}

void showMonEventos()
{
    lcd.clear();
    lcd.print("Gas:");
    lcd.print(analogRead(photocellPinGas));

    timer=0;
    updateMonEventos.Start();
}

void turnOnAlarm()
{
    buttonState = digitalRead(buttonPin);

    if (buttonState == HIGH){
        buttonState = LOW;
        input = Input::BtnPrs;
        executeAlarm = false;
    } 
}