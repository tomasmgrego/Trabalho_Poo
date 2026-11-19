# Define o compilador
CXX = g++

# Define as flags de compilação
# Adicionamos -MMD e -MP para gerar automaticamente as dependências dos arquivos .hh
CXXFLAGS = -std=c++17 -Wall -pthread -MMD -MP

# Nome do executável final
TARGET = simulacao

# Lista de arquivos fonte do projeto
SRCS = main.cc animais.cc ambiente.cc

# Arquivos objeto (.o) e arquivos de dependência (.d) gerados a partir dos fontes
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)

# Regra principal
all: $(TARGET)

# Como construir o executável a partir dos arquivos .o
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Como construir os arquivos .o a partir dos arquivos .cc
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para limpar os arquivos compilados e os arquivos de dependência
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

# Regra para rodar a simulação
run: $(TARGET)
	./$(TARGET)

# Inclui os arquivos de dependência gerados automaticamente pelo g++
-include $(DEPS)