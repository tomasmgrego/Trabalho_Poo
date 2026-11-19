#ifndef ANALISADOR_GENETICO_HH
#define ANALISADOR_GENETICO_HH

#include <map>
#include <set>
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <type_traits>
#include "animais.hh"

// Chave ordenada para uso em Collections associativas (std::map e std::set).
// Agrupa indivíduos que compartilham o mesmo perfil de constantes genéticas.
struct AssinaturaGenetica {
    std::array<int, 20> constantesQuantizadas;

    static AssinaturaGenetica extrair(const Genes& g) {
        auto q = [](double valor) {
            return static_cast<int>(std::round(valor * 10.0));
        };

        AssinaturaGenetica sig;
        sig.constantesQuantizadas = {
            q(g.size),
            q(g.physicCapacity),
            q(g.agingEffect),
            q(g.idealTemp.minTemp),
            q(g.idealTemp.maxTemp),
            q(g.tempEffect),
            q(g.minHumid),
            q(g.humidEffect),
            q(g.metabolism),
            q(g.foodNeed),
            q(g.foodDigestion),
            q(g.mateAwareness),
            q(g.predAwareness),
            q(g.preyAwareness),
            q(g.foodOrientation),
            q(g.tempOrientation),
            q(g.humidOrientation),
            q(g.breedOrientation),
            q(g.matureAge),
            q(g.fertility)
        };
        return sig;
    }

    bool operator<(const AssinaturaGenetica& outra) const {
        return constantesQuantizadas < outra.constantesQuantizadas;
    }
};

struct GrupoGenetico {
    int quantidade = 0;
    Genes genesRepresentativos;
    std::set<unsigned int> idsIndividuos; // Collection std::set para IDs únicos da linhagem
};

// Tipo Genérico (Class Template) parametrizado para qualquer subclasse de Animal
template <typename T>
class AnalisadorGenetico {
    static_assert(std::is_base_of<Animal, T>::value, "T deve ser uma classe derivada de Animal");

private:
    // Collections da STL para agrupamento e contagem de perfis genéticos
    std::map<AssinaturaGenetica, GrupoGenetico> mapaPopulacional;
    std::set<AssinaturaGenetica> perfisDistintos;

public:
    void analisar(const std::vector<std::unique_ptr<T>>& populacao) {
        mapaPopulacional.clear();
        perfisDistintos.clear();

        for (const auto& individuo : populacao) {
            if (!individuo || !individuo->isAlive()) continue;

            const Genes& dna = individuo->getDNA();
            AssinaturaGenetica assinatura = AssinaturaGenetica::extrair(dna);

            // Insere no std::set (mantém apenas linhagens únicas)
            perfisDistintos.insert(assinatura);

            // Agrupa e contabiliza no std::map
            GrupoGenetico& grupo = mapaPopulacional[assinatura];
            if (grupo.quantidade == 0) {
                grupo.genesRepresentativos = dna;
            }
            grupo.quantidade++;
            grupo.idsIndividuos.insert(individuo->getID());
        }
    }

    void exibirResultado(const std::string& nomeEspecie) const {
        std::cout << "\n========================================\n";
        std::cout << "ANALISE GENETICA FINAL: " << nomeEspecie << "\n";
        std::cout << "========================================\n";

        if (mapaPopulacional.empty()) {
            std::cout << "Nenhum individuo sobrevivente para analise.\n";
            return;
        }

        // Localiza no std::map o perfil genético com o maior número de indivíduos
        auto predominante = std::max_element(
            mapaPopulacional.begin(),
            mapaPopulacional.end(),
            [](const auto& a, const auto& b) {
                return a.second.quantidade < b.second.quantidade;
            }
        );

        const GrupoGenetico& vencedor = predominante->second;
        const Genes& g = vencedor.genesRepresentativos;

        std::cout << "Linhagens geneticas distintas (std::set): " << perfisDistintos.size() << "\n";
        std::cout << "Individuos na genetica predominante: " << vencedor.quantidade << "\n";
        std::cout << "IDs dos individuos da linhagem: ";
        for (unsigned int id : vencedor.idsIndividuos) {
            std::cout << id << " ";
        }
        std::cout << "\n\nConstantes da genetica que prevaleceu:\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "  - Size: " << g.size
                  << " | PhysicCapacity: " << g.physicCapacity
                  << " | AgingEffect: " << g.agingEffect << "\n";
        std::cout << "  - IdealTemp: [" << g.idealTemp.minTemp << ", " << g.idealTemp.maxTemp << "]"
                  << " | TempEffect: " << g.tempEffect << "\n";
        std::cout << "  - MinHumid: " << g.minHumid
                  << " | HumidEffect: " << g.humidEffect << "\n";
        std::cout << "  - Metabolism: " << g.metabolism
                  << " | FoodNeed: " << g.foodNeed
                  << " | FoodDigestion: " << g.foodDigestion << "\n";
        std::cout << "  - Orientations -> Food: " << g.foodOrientation
                  << " | Temp: " << g.tempOrientation
                  << " | Humid: " << g.humidOrientation
                  << " | Breed: " << g.breedOrientation << "\n";
        std::cout << "  - MatureAge: " << g.matureAge
                  << " | Fertility: " << g.fertility << "\n";
    }
};

#endif