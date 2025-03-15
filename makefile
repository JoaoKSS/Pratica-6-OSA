##############################################################################
# Exemplo de makefile para um projeto em C++ com arquivos separados
##############################################################################
# Arquivo principal: main.cpp
# Arquivos auxiliares: BTree.cpp, BTree.h
##############################################################################

# Nome do arquivo principal (sem a extensão)
MAIN    := main
# Objetos a serem gerados
OBJECTS := $(MAIN).o BTree.o

# Flags de compilação
FLAGS   := -Wall -Wextra -std=c++17 -pedantic-errors
DEBUG   := -g
MATH    :=
CC      := g++

# Define o nome do executável conforme o sistema
ifeq ($(OS), Windows_NT)
OUTPUTMAIN := $(MAIN).exe
else
OUTPUTMAIN := $(MAIN).out
endif

all: $(OUTPUTMAIN)
	@echo Compilação completa!

$(OUTPUTMAIN): $(OBJECTS)
	$(CC) $(FLAGS) $(DEBUG) $(OBJECTS) -o $(OUTPUTMAIN) $(MATH)

main.o: main.cpp BTree.h
	$(CC) $(FLAGS) $(DEBUG) -c main.cpp

BTree.o: BTree.cpp BTree.h
	$(CC) $(FLAGS) $(DEBUG) -c BTree.cpp

clean:
ifeq ($(OS), Windows_NT)
	del /Q *.o
	del /Q $(OUTPUTMAIN)
else
	rm -rf *.o
	rm -rf $(OUTPUTMAIN)
endif
	@echo Cleanup completo!!!

run: all
	./$(OUTPUTMAIN)
	@echo Execução completa!
