#include "persistencia.hh"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cmath>

namespace {
    double parseDouble(const std::string& s) {
        try {
            double val = std::stod(s);
            return std::isfinite(val) ? val : 0.0;
        } catch (const std::exception&) {
            return 0.0;
        }
    }

    float parseFloat(const std::string& s) {
        try {
            float val = std::stof(s);
            return std::isfinite(val) ? val : 0.0f;
        } catch (const std::exception&) {
            return 0.0f;
        }
    }
}

SimulacaoCsvDAO::SimulacaoCsvDAO(const std::string& caminho)
    : caminhoArquivo(caminho) {}

bool SimulacaoCsvDAO::existeDadosSalvos() const {
    std::ifstream file(caminhoArquivo);
    return file.good() && file.peek() != std::ifstream::traits_type::eof();
}

void SimulacaoCsvDAO::salvar(
    int turnoAtual,
    const generator& ambiente,
    const std::vector<std::unique_ptr<Frog>>& sapos,
    const std::vector<std::unique_ptr<Snake>>& cobras
) {
    std::string tempArquivo = caminhoArquivo + ".tmp";
    std::ofstream out(tempArquivo);
    if (!out.is_open()) return;

    // 1. Metadados da simulação
    out << "TURNO;" << turnoAtual << ";" << GRID_SIZE << "\n";

    // 2. Estado do ambiente (10x10 tiles)
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            const square& sq = ambiente.getSquare(i, j);
            out << "TILE;" << i << ";" << j << ";"
                << sq.getType() << ";"
                << sq.getHumidity() << ";"
                << sq.getFood() << ";"
                << sq.getSatPrey() << ";"
                << sq.getSatPred() << ";"
                << sq.getTemperature() << "\n";
        }
    }

    auto escreverAnimal = [&](const std::string& tipo, const Animal& a) {
        const Genes& g = a.getDNA();
        out << tipo << ";"
            << a.getID() << ";"
            << a.getAge() << ";"
            << a.getEnergyReservatory() << ";"
            << a.getVitalEnergy() << ";"
            << a.getTileX() << ";"
            << a.getTileY() << ";"
            << g.gender << ";"
            << g.size << ";"
            << g.physicCapacity << ";"
            << g.agingEffect << ";"
            << g.idealTemp.minTemp << ";"
            << g.idealTemp.maxTemp << ";"
            << g.tempEffect << ";"
            << g.minHumid << ";"
            << g.humidEffect << ";"
            << g.metabolism << ";"
            << g.foodNeed << ";"
            << g.foodDigestion << ";"
            << g.mateAwareness << ";"
            << g.predAwareness << ";"
            << g.preyAwareness << ";"
            << g.foodOrientation << ";"
            << g.tempOrientation << ";"
            << g.humidOrientation << ";"
            << g.breedOrientation << ";"
            << g.matureAge << ";"
            << g.fertility << "\n";
    };

    // 3. Populações de sapos e cobras
    for (const auto& frog : sapos) {
        if (frog && frog->isAlive()) {
            escreverAnimal("FROG", *frog);
        }
    }
    for (const auto& snake : cobras) {
        if (snake && snake->isAlive()) {
            escreverAnimal("SNAKE", *snake);
        }
    }

    out.close();
    std::remove(caminhoArquivo.c_str());
    std::rename(tempArquivo.c_str(), caminhoArquivo.c_str());
}

bool SimulacaoCsvDAO::carregar(
    int& turnoAtual,
    generator& ambiente,
    std::vector<std::unique_ptr<Frog>>& sapos,
    std::vector<std::unique_ptr<Snake>>& cobras
) {
    std::ifstream in(caminhoArquivo);
    if (!in.is_open()) return false;

    sapos.clear();
    cobras.clear();

    std::string linha;
    while (std::getline(in, linha)) {
        if (linha.empty()) continue;

        std::stringstream ss(linha);
        std::string item;
        std::vector<std::string> tokens;

        while (std::getline(ss, item, ';')) {
            tokens.push_back(item);
        }

        if (tokens.empty()) continue;

        if (tokens[0] == "TURNO" && tokens.size() >= 2) {
            turnoAtual = std::stoi(tokens[1]);
        }
        else if (tokens[0] == "TILE" && tokens.size() >= 9) {
            int i = std::stoi(tokens[1]);
            int j = std::stoi(tokens[2]);
            int type = std::stoi(tokens[3]);
            double hum = parseDouble(tokens[4]);
            double fd = parseDouble(tokens[5]);
            double sPrey = parseDouble(tokens[6]);
            double sPred = parseDouble(tokens[7]);
            double temp = parseDouble(tokens[8]);

            ambiente.setSquare(i, j, type, hum, fd, sPrey, sPred, temp);
        }
        else if ((tokens[0] == "FROG" || tokens[0] == "SNAKE") && tokens.size() >= 28) {
            unsigned int id = static_cast<unsigned int>(std::stoul(tokens[1]));
            int age = std::stoi(tokens[2]);
            double energy = parseDouble(tokens[3]);
            double vital = parseDouble(tokens[4]);
            int x = std::stoi(tokens[5]);
            int y = std::stoi(tokens[6]);

            Genes g;
            g.gender = std::stoi(tokens[7]);
            g.size = parseDouble(tokens[8]);
            g.physicCapacity = parseDouble(tokens[9]);
            g.agingEffect = parseDouble(tokens[10]);
            g.idealTemp.minTemp = parseFloat(tokens[11]);
            g.idealTemp.maxTemp = parseFloat(tokens[12]);
            g.tempEffect = parseDouble(tokens[13]);
            g.minHumid = parseDouble(tokens[14]);
            g.humidEffect = parseDouble(tokens[15]);
            g.metabolism = parseDouble(tokens[16]);
            g.foodNeed = parseDouble(tokens[17]);
            g.foodDigestion = parseDouble(tokens[18]);
            g.mateAwareness = parseDouble(tokens[19]);
            g.predAwareness = parseDouble(tokens[20]);
            g.preyAwareness = parseDouble(tokens[21]);
            g.foodOrientation = parseDouble(tokens[22]);
            g.tempOrientation = parseDouble(tokens[23]);
            g.humidOrientation = parseDouble(tokens[24]);
            g.breedOrientation = parseDouble(tokens[25]);
            g.matureAge = parseDouble(tokens[26]);
            g.fertility = parseDouble(tokens[27]);

            if (tokens[0] == "FROG") {
                auto frog = std::make_unique<Frog>(id, x, y, g);
                frog->restoreState(age, energy, vital);
                ambiente.getSquare(x, y).enterFrog(frog.get());
                sapos.push_back(std::move(frog));
            } else {
                auto snake = std::make_unique<Snake>(id, x, y, g);
                snake->restoreState(age, energy, vital);
                ambiente.getSquare(x, y).enterSnake(snake.get());
                cobras.push_back(std::move(snake));
            }
        }
    }

    return true;
}