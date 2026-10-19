#include "ambiente.hh"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <fstream>
#include <iostream>

generator::generator() {}

square& generator::getSquare(int i, int j) {

    return *(ambienteCompleto[i][j]);

}

void generator::ambienteGenerator() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 5);

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){

            try {
                int numero = dist(gen);

                switch(numero){
                    case 1:
                        ambienteCompleto[i][j] = std::make_unique<water>();

                        break;
                    case 2:
                        ambienteCompleto[i][j] = std::make_unique<swamp>();

                        break;
                    case 3:
                        ambienteCompleto[i][j] = std::make_unique<dryDirt>();

                        break;
                    case 4:
                        ambienteCompleto[i][j] = std::make_unique<tallGrass>();

                        break;
                    case 5:
                        ambienteCompleto[i][j] = std::make_unique<shortGrass>();

                        break;
                }
                ambienteCompleto[i][j]->calcAttributes();

            }
            catch (const std::exception& e) {
                std::cout << "Exceção capturada: " << e.what() << '\n';
        
            }
        }
    }
}

square::square(int type) : squareType(type), gen(std::random_device{}()) {}

int square::getType() {

    return squareType;

}

double square::getHumidity() const {

    return humidity;

}

double square::getFood() const {

    return food;

}

double square::getSatPrey() const {

    return satPrey;

}

double square::getSatPred() const {

    return satPred;

}

double square::getTemperature() const {

    return temperature;

}

void square::enterFrog(Frog* f) {

    frogsInTile.push_back(f);
}

void square::leaveFrog(Frog* f) {

    frogsInTile.erase(std::remove(frogsInTile.begin(), frogsInTile.end(), f), frogsInTile.end());
}

int square::getFrogNum() const {

    return frogsInTile.size();
}

std::vector<Frog*>& square::getFrogs() {

    return frogsInTile;
}

void square::enterSnake(Snake* s) {

    snakesInTile.push_back(s);
}

void square::leaveSnake(Snake* s) {

    snakesInTile.erase(std::remove(snakesInTile.begin(), snakesInTile.end(), s), snakesInTile.end());
}

int square::getSnakeNum() const {

    return snakesInTile.size();
}

std::vector<Snake*>& square::getSnakes() {
    
    return snakesInTile;
}

water::water() : square(1) {}

void water::calcAttributes() {

    std::normal_distribution<double> gauss1(0.5, 0.1667);
    humidity = std::clamp(gauss1(gen), 0.0, 1.0);
    
    std::normal_distribution<double> gauss2(0.5, 0.1667);
    food = std::clamp(gauss2(gen), 0.0, 1.0);
    
    std::normal_distribution<double> gauss3(50.0, 16.67);
    satPrey = std::clamp(gauss3(gen), 0.0, 100.0);
    
    std::normal_distribution<double> gauss4(50.0, 16.67);
    satPred = std::clamp(gauss4(gen), 0.0, 100.0);
    
    std::normal_distribution<double> gauss5(0.0, 33.0);
    temperature = std::clamp(gauss5(gen), -100.0, 100.0);
}

swamp::swamp() : square(2) {}

void swamp::calcAttributes() {

    std::normal_distribution<double> gauss1(0.5, 0.1667);
    humidity = std::clamp(gauss1(gen), 0.0, 1.0);
    
    std::normal_distribution<double> gauss2(0.5, 0.1667);
    food = std::clamp(gauss2(gen), 0.0, 1.0);
    
    std::normal_distribution<double> gauss3(50.0, 16.67);
    satPrey = std::clamp(gauss3(gen), 0.0, 100.0);
    
    std::normal_distribution<double> gauss4(50.0, 16.67);
    satPred = std::clamp(gauss4(gen), 0.0, 100.0);
    
    std::normal_distribution<double> gauss5(0.0, 33.0);
    temperature = std::clamp(gauss5(gen), -100.0, 100.0);
}

dryDirt::dryDirt() : square(3) {}

void dryDirt::calcAttributes() {

    std::normal_distribution<double> gauss1(0.5, 0.1667);
    humidity = std::clamp(gauss1(gen), 0.0, 1.0);

    std::normal_distribution<double> gauss2(0.5, 0.1667);
    food = std::clamp(gauss2(gen), 0.0, 1.0);
    
    std::normal_distribution<double> gauss3(50.0, 16.67);
    satPrey = std::clamp(gauss3(gen), 0.0, 100.0);
    
    std::normal_distribution<double> gauss4(50.0, 16.67);
    satPred = std::clamp(gauss4(gen), 0.0, 100.0);
    
    std::normal_distribution<double> gauss5(0.0, 33.0);
    temperature = std::clamp(gauss5(gen), -100.0, 100.0);
}

tallGrass::tallGrass() : square(4) {}

void tallGrass::calcAttributes() {

    std::normal_distribution<double> gauss1(0.5, 0.1667);
    humidity = std::clamp(gauss1(gen), 0.0, 1.0);

    std::normal_distribution<double> gauss2(0.5, 0.1667);
    food = std::clamp(gauss2(gen), 0.0, 1.0);

    std::normal_distribution<double> gauss3(50.0, 16.67);
    satPrey = std::clamp(gauss3(gen), 0.0, 100.0);

    std::normal_distribution<double> gauss4(50.0, 16.67);
    satPred = std::clamp(gauss4(gen), 0.0, 100.0);

    std::normal_distribution<double> gauss5(0.0, 33.0);
    temperature = std::clamp(gauss5(gen), -100.0, 100.0);
}

shortGrass::shortGrass() : square(5) {}

void shortGrass::calcAttributes() {

    std::normal_distribution<double> gauss1(0.5, 0.1667);
    humidity = std::clamp(gauss1(gen), 0.0, 1.0);

    std::normal_distribution<double> gauss2(0.5, 0.1667);
    food = std::clamp(gauss2(gen), 0.0, 1.0);

    std::normal_distribution<double> gauss3(50.0, 16.67);
    satPrey = std::clamp(gauss3(gen), 0.0, 100.0);

    std::normal_distribution<double> gauss4(50.0, 16.67);
    satPred = std::clamp(gauss4(gen), 0.0, 100.0);

    std::normal_distribution<double> gauss5(0.0, 33.0);
    temperature = std::clamp(gauss5(gen), -100.0, 100.0);
}
