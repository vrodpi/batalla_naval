# Makefile - Batalla Naval (ANSI C, portable Windows/Linux)
#
# Requiere GCC o Clang y `make`.
#  Linux:   sudo apt install gcc make   (o equivalente)
#  Windows: instalar LLVM-MinGW (o MinGW-w64) y usar `mingw32-make`.
#           Verificado con LLVM-MinGW 22 + `mingw32-make CC=clang`.
#           Ejecutar desde Git Bash/MSYS2. Ver README.md.
#
# Uso (desde la raiz del proyecto, donde esta data/):
#   make            -> compila todo en build/
#   make test       -> compila y ejecuta tests/test_ranking
#   make demo       -> compila demo y muestra ayuda de ejecucion
#   make clean      -> borra build/ y archivos temporales de prueba
#
# Estandar: -ansi -pedantic garantiza ANSI C (C89).

CC ?= gcc
CFLAGS = -ansi -pedantic -Wall -Wextra -Iinclude
BUILD = build

# Binarios
BIN_BATALLA = $(BUILD)/batalla-naval
BIN_DEMO = $(BUILD)/demo_ranking
BIN_TEST = $(BUILD)/test_ranking

# Fuentes actuales. Cuando existan app.c, ui_cli.c, game.c, board.c, ai.c,
# agregarlos a SRC_BATALLA.
SRC_BATALLA = src/main.c src/ranking.c
SRC_DEMO = src/demo_ranking.c src/ranking.c
SRC_TEST = tests/test_ranking.c src/ranking.c

all: $(BIN_BATALLA) $(BIN_DEMO) $(BIN_TEST)

$(BUILD):
	mkdir -p $(BUILD)

$(BIN_BATALLA): $(SRC_BATALLA) include/ranking.h include/config.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_BATALLA) -o $(BIN_BATALLA)

$(BIN_DEMO): $(SRC_DEMO) include/ranking.h include/config.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_DEMO) -o $(BIN_DEMO)

$(BIN_TEST): $(SRC_TEST) include/ranking.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_TEST) -o $(BIN_TEST)

test: $(BIN_TEST)
	./$(BIN_TEST)

demo: $(BIN_DEMO)
	@echo "Ejecute desde la raiz: ./$(BIN_DEMO) [NombreGanador]"

clean:
	rm -f $(BIN_BATALLA) $(BIN_DEMO) $(BIN_TEST)
	rm -f $(BUILD)/*.o $(BUILD)/*.exe
	rm -f data/ranking_test_*.dat

.PHONY: all test demo clean
