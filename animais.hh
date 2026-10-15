#ifndef ANIMAIS_HH
#define ANIMAIS_HH

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <fstream>


struct tempRange {

    float minTemp;
    float maxTemp;

    bool isIdealTemp(float temp) {

        return temp >= minTemp && temp <= maxTemp;
    }

};


struct Genes {

    int gender;

    double size;
    double physicCapacity;
    double metabolism;

    double mateAwareness;
    double predAwareness;
    double preyAwareness;

    double foodOrientation;
    double tempOrientation;
    double breedOrientation;

    double matureAge;
    double fertility;

    tempRange idealTemp;

};


class Animal {

    protected:

        uint ID;
        int age;

        double energyReservatory;
        double vitalEnergy;
        
        int tileOccupated;
        Genes DNA;
    
    public:

        Animal(uint ID, int initialTile, Genes genes);
        virtual ~Animal() = default;

        virtual void breathe() = 0;
        virtual void eat() = 0;
        virtual void move() = 0;
        virtual void reproduce() = 0;

        virtual void update();

        bool isAlive() const {return vitalEnergy > 0.0};
    
};

#endif