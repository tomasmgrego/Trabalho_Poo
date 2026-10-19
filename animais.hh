#ifndef ANIMAIS_HH
#define ANIMAIS_HH

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <fstream>
#include <random>
#include <vector>
#include <algorithm>
#include <functional>

#include "ambiente.hh"

struct tempRange {

    float minTemp;
    float maxTemp;

    bool isIdealTemp(float temp) {

        return temp >= minTemp && temp <= maxTemp;
    }

};

enum class OrientationIntent {

    Food,
    Breed,
    Humidity,
    Temperature

};

struct Genes {

    int gender;

    double size;
    double physicCapacity;
    double agingEffect;

    tempRange idealTemp;
    double tempEffect;

    double minHumid;
    double humidEffect;

    double metabolism;

    double foodNeed;
    double foodDigestion;

    double mateAwareness;
    double predAwareness;
    double preyAwareness;

    double foodOrientation;
    double tempOrientation;
    double humidOrientation;
    double breedOrientation;

    double matureAge;
    double fertility;

};

Genes genRandomGenes(std::mt19937& gen);

class generator;

class Animal {

    protected:

        uint ID;
        int age;

        double energyReservatory;
        double vitalEnergy;
        
        int tileX;
        int tileY;

        Genes DNA;
    
    public:

        Animal(uint ID, int initX, int initY, Genes genes);
        virtual ~Animal() = default;

        int getTileX() const { return tileX; }
        int getTileY() const { return tileY; }

        void ageUpdate(int quantity = 10) { age += quantity; }

        virtual void eat(generator& world) = 0;
        virtual void move(generator& world) = 0;
        virtual void reproduce(generator& world) = 0;

        virtual void breathe(generator& world);

        virtual void update(generator& world);

        bool isAlive() const { return vitalEnergy > 0.0; }
    
};

class Frog : public Animal {

    private:

    public:

        Frog(unsigned int id, float startX, float startY, Genes genes);
    
        void move(generator& world) override;
        void eat(generator& world) override;
        
        Frog* reproduce(const Frog& partner);

};

class Snake : public Animal {

    private:

    public:

        Snake(unsigned int id, float startX, float startY, Genes genes);
    
        void move(generator& world) override;
        void eat(generator& world) override;
        
        Snake* reproduce(const Snake& partner);

};


#endif