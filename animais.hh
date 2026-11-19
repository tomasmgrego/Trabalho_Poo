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

    int gender = 0;

    double size = 0.1;
    double physicCapacity = 0.1;
    double agingEffect = 0.01;

    tempRange idealTemp{0.0f, 0.0f};
    double tempEffect = 0.05;

    double minHumid = 0.0;
    double humidEffect = 0.05;

    double metabolism = 0.1;

    double foodNeed = 0.1;
    double foodDigestion = 0.1;

    double mateAwareness = 0.1;
    double predAwareness = 0.1;
    double preyAwareness = 0.1;

    double foodOrientation = 0.1;
    double tempOrientation = 0.1;
    double humidOrientation = 0.1;
    double breedOrientation = 0.1;

    double matureAge = 1.0;
    double fertility = 0.1;

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

        // Métodos de apoio à persistência
        uint getID() const { return ID; }
        int getAge() const { return age; }
        double getEnergyReservatory() const { return energyReservatory; }
        double getVitalEnergy() const { return vitalEnergy; }
        void restoreState(int savedAge, double savedEnergy, double savedVital) {
            age = savedAge;
            energyReservatory = savedEnergy;
            vitalEnergy = savedVital;
        }

        int getTileX() const { return tileX; }
        int getTileY() const { return tileY; }

        void ageUpdate(int quantity = 10) { age += quantity; }

        virtual void eat(generator& world) = 0;
        virtual void move(generator& world) = 0;
        virtual Animal* reproduce(generator& world) = 0;

        virtual void breathe(generator& world);

        bool isAlive() const { return vitalEnergy > 0.0; }

        const Genes& getDNA() const { return DNA; }
        void kill() { vitalEnergy = 0.0; }
    
};

class Frog : public Animal {

    private:

    public:

        Frog(unsigned int id, float startX, float startY, Genes genes);
    
        void move(generator& world) override;
        void eat(generator& world) override;

        Animal* reproduce(generator& world) override;
        
        Frog* reproduce(generator& world, const Frog& partner);

};

class Snake : public Animal {

    private:

    public:

        Snake(unsigned int id, float startX, float startY, Genes genes);
    
        void move(generator& world) override;
        void eat(generator& world) override;

        Animal* reproduce(generator& world);
        
        Snake* reproduce(generator& world, const Snake& partner);

};


#endif