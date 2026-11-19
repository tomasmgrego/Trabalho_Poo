#include "ambiente.hh"
#include "animais.hh"
#include "persistencia.hh"
#include "analisador_genetico.hh"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>

template<typename T>
void removeDead(std::vector<std::unique_ptr<T>>& population) {

    population.erase(

        std::remove_if(population.begin(), population.end(),
            [](const std::unique_ptr<T>& animal) { return !animal->isAlive(); }),
        population.end()
    );
}

template<typename T>
void agePopulation(std::vector<std::unique_ptr<T>>& population, int amount = 10) {

    for(auto &animal : population) {

        if (animal->isAlive()) {
            animal->ageUpdate(amount);
        }
    }
}

void renderSim(generator& meuAmbiente, int turn) {
    // Código ANSI para limpar o terminal e colocar o cursor no topo
    std::cout << "\033[2J\033[H"; 
    
    std::cout << "--- TURNO: " << turn << " ---\n\n";

    for (int i = 0; i < GRID_SIZE; i++) {
        
        for (int row = 0; row < 3; row++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                
                square& sq = meuAmbiente.getSquare(i, j);
                int type = sq.getType();
                
                // Define a cor de fundo com base no tipo de tile
                std::string bg;
                switch(type) {
                    case 1: bg = "\033[44m"; break;  // Água: Fundo Azul
                    case 2: bg = "\033[45m"; break;  // Pântano: Fundo Magenta/Roxo
                    case 3: bg = "\033[43m"; break;  // Terra Seca: Fundo Amarelo (marrom claro)
                    case 4: bg = "\033[42m"; break;  // Grama Alta: Fundo Verde
                    case 5: bg = "\033[102m"; break; // Grama Curta: Fundo Verde Claro
                    default: bg = "\033[40m"; break; // Padrão
                }
                
                std::string reset = "\033[0m"; // Reseta as cores
                
                // Na linha do meio do bloco (row == 1), imprimimos os animais
                if (row == 1) {
                    int numFrogs = sq.getFrogNum();
                    int numSnakes = sq.getSnakeNum();

                    // \033[92m = Texto Verde Claro (Sapos)
                    // \033[31m = Texto Vermelho/Marrom (Cobras)
                    std::string fStr = (numFrogs > 0) ? "\033[92m F:" + std::to_string(numFrogs) + " " : "     ";
                    std::string sStr = (numSnakes > 0) ? "\033[31m S:" + std::to_string(numSnakes) + " " : "     ";
                    
                    // Se não tiver animal, preenche com espaço para manter o quadrado alinhado
                    if (numFrogs == 0 && numSnakes == 0) {
                        std::cout << bg << "          " << reset;
                    } else {
                        // Imprime o fundo + os indicadores de animais + reseta
                        std::cout << bg << fStr << sStr << reset;
                    }
                } else {
                    // Nas outras linhas do bloco (topo e base), apenas espaços com a cor de fundo
                    std::cout << bg << "          " << reset; 
                }
            }
            std::cout << "\n";
        }
    }
    
    std::cout << "\nLegenda Terrenos:\n";
    std::cout << "\033[44m  \033[0m Agua  \033[45m  \033[0m Pantano  \033[43m  \033[0m Terra Seca  \033[42m  \033[0m Grama Alta  \033[102m  \033[0m Grama Curta\n";
    std::cout << "Animais: \033[92mF = Sapos\033[0m | \033[31mS = Cobras\033[0m\n";
    std::cout << "\033[90m[Auto-save ativo em segundo plano]\033[0m\n";
    
    // Pausa a thread para criar o efeito de animação (500 milissegundos)
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    generator meuAmbiente;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> posDist(0, GRID_SIZE - 1);

    int numInitialFrogs = 100;
    int numInitialSnakes = 10;

    std::vector<std::unique_ptr<Frog>> frogPopulation;
    std::vector<std::unique_ptr<Snake>> snakePopulation;

    // Instância do Padrão DAO para persistência
    std::unique_ptr<ISimulacaoDAO> dao = std::make_unique<SimulacaoCsvDAO>("simulacao_save.csv");

    int startTurn = 1;
    int totalTurns = 50;
    int turnsToAge = 10;
    bool carregouSave = false;

    // Carregamento na entrada do sistema caso exista arquivo salvo
    if (dao->existeDadosSalvos()) {
        int opcao = 0;
        std::cout << "Save encontrado (simulacao_save.csv). Deseja carregar? (1 - Sim / 0 - Nova simulacao): ";
        if (std::cin >> opcao && opcao == 1) {
            int savedTurn = 0;
            if (dao->carregar(savedTurn, meuAmbiente, frogPopulation, snakePopulation)) {
                startTurn = savedTurn + 1;
                if (startTurn > totalTurns) {
                    totalTurns = startTurn + 49;
                }
                carregouSave = true;
                std::cout << "Dados carregados com sucesso! Retomando do turno " << startTurn << "...\n";
            }
        }
    }

    if (!carregouSave) {
        meuAmbiente.ambienteGenerator();

        auto spawnFrogs = [&]() {
            for (int i = 0; i < numInitialFrogs; ++i) {
                int rX = posDist(gen), rY = posDist(gen);

                auto newFrog = std::make_unique<Frog>(i, rX, rY, genRandomGenes(gen));
                meuAmbiente.getSquare(rX, rY).enterFrog(newFrog.get());
                frogPopulation.push_back(std::move(newFrog));
            }
        };

        auto spawnSnakes = [&]() {
            for (int i = 0; i < numInitialSnakes; ++i) {
                int rX = posDist(gen), rY = posDist(gen);

                auto newSnake = std::make_unique<Snake>(i + 1000, rX, rY, genRandomGenes(gen));
                meuAmbiente.getSquare(rX, rY).enterSnake(newSnake.get());
                snakePopulation.push_back(std::move(newSnake));
            }
        };

        spawnFrogs();
        spawnSnakes();
    }

    std::cout << "\nSimulacao iniciada...\n";

    // Controle de sincronização e Thread de Auto-Save em segundo plano
    std::mutex simMutex;
    std::atomic<bool> autoSaveRunning{true};
    int currentTurn = startTurn - 1;

    std::thread autoSaveThread([&]() {
        while (autoSaveRunning.load()) {
            for (int i = 0; i < 50 && autoSaveRunning.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (!autoSaveRunning.load()) break;

            std::lock_guard<std::mutex> lock(simMutex);
            dao->salvar(currentTurn, meuAmbiente, frogPopulation, snakePopulation);
        }
    });

    for(int turn = startTurn; turn <= totalTurns; turn++) {
        {
            std::lock_guard<std::mutex> lock(simMutex);
            currentTurn = turn;

            std::vector<std::unique_ptr<Frog>> newFrogs;
            std::vector<std::unique_ptr<Snake>> newSnakes;


            for(auto &frog : frogPopulation) {
                if(!frog->isAlive()) continue;

                frog->breathe(meuAmbiente);
                if(!frog->isAlive()) {
                    meuAmbiente.getSquare(frog->getTileX(), frog->getTileY()).leaveFrog(frog.get());
                    continue;
                }

                int currentX = frog->getTileX();
                int currentY = frog->getTileY();
                
                frog->move(meuAmbiente);
                
                int newX = frog->getTileX();
                int newY = frog->getTileY();


                if(currentX != newX || currentY != newY) {
                    meuAmbiente.getSquare(currentX, currentY).leaveFrog(frog.get());
                    meuAmbiente.getSquare(newX, newY).enterFrog(frog.get());
                }

                frog->eat(meuAmbiente);


                Animal* baby = frog->reproduce(meuAmbiente); 
                if (baby) {
                    newFrogs.push_back(std::unique_ptr<Frog>(static_cast<Frog*>(baby)));
                }
            }


            for(auto &snake : snakePopulation) {
                if(!snake->isAlive()) continue;

                snake->breathe(meuAmbiente);
                if(!snake->isAlive()) {
                    meuAmbiente.getSquare(snake->getTileX(), snake->getTileY()).leaveSnake(snake.get());
                    continue;
                }

                snake->move(meuAmbiente);
                snake->eat(meuAmbiente);

                Animal* baby = snake->reproduce(meuAmbiente); 
                if (baby) {
                    newSnakes.push_back(std::unique_ptr<Snake>(static_cast<Snake*>(baby)));
                }
            }


            for (auto& baby : newFrogs) {
                frogPopulation.push_back(std::move(baby));
            }
            for (auto& baby : newSnakes) {
                snakePopulation.push_back(std::move(baby));
            }

            removeDead(frogPopulation);
            removeDead(snakePopulation);

            if(turn % turnsToAge == 0) {
                agePopulation(frogPopulation, 10);
                agePopulation(snakePopulation, 10);
            }
        }

        // CHAME A RENDERIZAÇÃO AQUI (final do loop do turno)
        renderSim(meuAmbiente, turn);
    } // fim do for(int turn = startTurn...)

    // Encerra a thread de auto-save de forma limpa e realiza o salvamento final na saída
    autoSaveRunning.store(false);
    if (autoSaveThread.joinable()) {
        autoSaveThread.join();
    }
    dao->salvar(currentTurn, meuAmbiente, frogPopulation, snakePopulation);

    std::cout << "Simulacao concluida. Estado salvo em 'simulacao_save.csv'.\n";

    std::cout << "Sapos sobreviventes: " << frogPopulation.size() << "\n";
    std::cout << "Cobras sobreviventes: " << snakePopulation.size() << "\n";

    // Uso do Tipo Genérico AnalisadorGenetico<T> e Collections (std::map e std::set)
    AnalisadorGenetico<Frog> analisadorSapos;
    analisadorSapos.analisar(frogPopulation);
    analisadorSapos.exibirResultado("SAPOS");

    AnalisadorGenetico<Snake> analisadorCobras;
    analisadorCobras.analisar(snakePopulation);
    analisadorCobras.exibirResultado("COBRAS");

    return 0;
}