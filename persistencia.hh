#ifndef PERSISTENCIA_HH
#define PERSISTENCIA_HH

#include <string>
#include <vector>
#include <memory>
#include "ambiente.hh"
#include "animais.hh"

// Padrão de Projeto: DAO (Data Access Object)
// Interface que define o contrato de persistência desacoplado da lógica de simulação
class ISimulacaoDAO {
public:
    virtual ~ISimulacaoDAO() = default;

    virtual bool existeDadosSalvos() const = 0;

    virtual void salvar(
        int turnoAtual,
        const generator& ambiente,
        const std::vector<std::unique_ptr<Frog>>& sapos,
        const std::vector<std::unique_ptr<Snake>>& cobras
    ) = 0;

    virtual bool carregar(
        int& turnoAtual,
        generator& ambiente,
        std::vector<std::unique_ptr<Frog>>& sapos,
        std::vector<std::unique_ptr<Snake>>& cobras
    ) = 0;
};

// Implementação concreta do DAO utilizando arquivo CSV
class SimulacaoCsvDAO : public ISimulacaoDAO {
private:
    std::string caminhoArquivo;

public:
    explicit SimulacaoCsvDAO(const std::string& caminho = "simulacao_save.csv");

    bool existeDadosSalvos() const override;

    void salvar(
        int turnoAtual,
        const generator& ambiente,
        const std::vector<std::unique_ptr<Frog>>& sapos,
        const std::vector<std::unique_ptr<Snake>>& cobras
    ) override;

    bool carregar(
        int& turnoAtual,
        generator& ambiente,
        std::vector<std::unique_ptr<Frog>>& sapos,
        std::vector<std::unique_ptr<Snake>>& cobras
    ) override;
};

#endif