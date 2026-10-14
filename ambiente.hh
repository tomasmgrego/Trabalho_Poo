#ifndef AMBIENTE_HH
#define AMBIENTE_HH

#include <random>
#include <memory>
#include <algorithm>

class square {

    protected:

        double humidity;
        double food;
        double satPrey;
        double satPred;
        double temperature;

        std::mt19937 gen;

    public:

        square(int type);

        virtual ~square() = default;

        int getType();

        double getHumidity() const;
        double getFood() const;
        double getSatPrey() const;
        double getSatPred() const;
        double getTemperature() const;

        virtual void calcAttributes() = 0;

    private:

        int squareType;

};

class water : public square {

    public:

        water();

        void calcAttributes() override;

};

class swamp : public square {

    public:

        swamp();

        void calcAttributes() override;

};

class dryDirt : public square {

    public:
        
        dryDirt();

        void calcAttributes() override;

};

class tallGrass : public square {

    public:

        tallGrass();

        void calcAttributes() override;
};

class shortGrass : public square {

    public:

        shortGrass();

        void calcAttributes() override;

};

class generator {

    public:

        generator();

        void ambienteGenerator();

        square& getSquare(int i, int j);

    private:

        std::unique_ptr<square> ambienteCompleto[3][3];

};

#endif