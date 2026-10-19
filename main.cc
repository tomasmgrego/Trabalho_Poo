#include "ambiente.hh"
#include "animais.hh"

#include <iostream>

int main() {

    generator meuAmbiente;

    meuAmbiente.ambienteGenerator();

    std::random_device rd;
    std::mt19937 gen(rd());

    // ESPECÍFICO PARA O GRID 3X3
    std::uniform_int_distribution<int> posDist(0, 2);

    int numInitialFrogs = 10;

    std::vector<std::unique_ptr<Frog>> frogPopulation;

    for (int i = 0; i < numInitialFrogs; ++i) {
        try {

            int randX = posDist(gen);
            int randY = posDist(gen);

            Genes genesFrog = genRandomGenes(gen);

            auto newFrog = std::make_unique<Frog>(i, randX, randY, genesFrog);

            square& s = meuAmbiente.getSquare(randX, randY);
            s.enterFrog(newFrog.get());

            frogPopulation.push_back(std::move(newFrog));
            
        }
        
        catch (const std::exception& e) {

            std::cout << "Exceção capturada: " << e.what() << '\n';
        }
            
    }

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

    int totalTurns = 50;
    int turnsToAge = 10;

    std::cout << "\n simulação iniciada";

    for(int turn = 1; turn <= totalTurns; turn++) {

        for(auto &frog : frogPopulation) {

            if(!frog->isAlive()) continue;

            frog->breathe(meuAmbiente);

            int currentX = frog->getTileX();
            int currentY = frog->getTileY();

            if(!frog->isAlive()) {

                meuAmbiente.getSquare(currentX, currentY).leaveFrog(frog.get());
                continue;
            }

            frog->move(meuAmbiente);

            int newX = frog->getTileX();
            int newY = frog->getTileY();

            if(currentX != newX || currentY != newY) {

                meuAmbiente.getSquare(currentX, currentY).leaveFrog(frog.get());
                meuAmbiente.getSquare(newX, newY).enterFrog(frog.get());

            }

            frog->eat(meuAmbiente);

            frogPopulation.erase(

                std::remove_if(frogPopulation.begin(), frogPopulation.end(),

                    [](const std::unique_ptr<Frog>& frog) { return !frog->isAlive(); }),

                frogPopulation.end()
            );

            if(turn % turnsToAge == 0) {

                for(auto &frog : frogPopulation) {

                    frog->ageUpdate(10);

                }
            }

        }
    }

    std::cout << "simulacao concluida. sapos sobreviventes: " << frogPopulation.size() << "\n";

    return 0;
}
