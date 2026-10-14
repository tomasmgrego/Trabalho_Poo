#include "ambiente.hh"

#include <iostream>

int main() {

    generator meuAmbiente;

    meuAmbiente.ambienteGenerator();

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {

            square& s = meuAmbiente.getSquare(i, j);

            std::cout << s.getType() << "\n";

            std::cout << "Humidity[" << i << "][" << j << "]: " << s.getHumidity() << "\n";
            std::cout << "Food[" << i << "][" << j << "]: " << s.getFood() << "\n";
            std::cout << "SatPrey[" << i << "][" << j << "]: " << s.getSatPrey() << "\n";
            std::cout << "SatPred[" << i << "][" << j << "]: " << s.getSatPred() << "\n";
            std::cout << "Temperature[" << i << "][" << j << "]: " << s.getTemperature() << "\n";
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {

            square& s = meuAmbiente.getSquare(i, j);

            std::cout << s.getType() << " ";
        }
        std::cout << std::endl;
    }
}
