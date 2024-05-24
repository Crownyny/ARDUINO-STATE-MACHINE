#ifndef IMPERIALMARCH_H
#define IMPERIALMARCH_H

#include <Arduino.h>

class ImperialMarch {
public:
    ImperialMarch(int pin, int tempo = 120);
    void play();
    void stop();

private:
    int buzzer;
    int tempo;
    int notes;
    int wholenote;
    int melody[208]; // Ajusta el tamaño si cambias la melodía
    bool isPlaying;
    

    void playNote(int note, int duration);
};

#endif
