# Makefile - Batalla Naval (C99)
#
# Requiere GCC o Clang y make. El objetivo de compilacion acordado es Linux;
# tambien se verifica en Windows con LLVM-MinGW (ver README_gian.md).
#
# Uso, desde la raiz del proyecto (donde esta data/):
#   make            -> compila el juego, la demo y las pruebas
#   make app        -> compila solo el juego completo
#   make test       -> ejecuta las pruebas del ranking
#   make demo       -> compila la demo de fin de partida
#   make run        -> compila y ejecuta el juego
#   make clean      -> borra binarios y temporales de prueba
#
# Estandar: -std=c99 -pedantic -Wall -Wextra. Se adopto C99 para todo el
# proyecto porque permite comentarios // y declaraciones dentro de for,
# que ya usan los modulos de interfaz y tablero. -pedantic mantiene el
# codigo dentro del estandar, sin extensiones del compilador.
#
# El archivo de datos del ranking es un volcado binario del struct Ranking
# (sin serializacion byte a byte): ver docs/contrato-ranking.md.

CC ?= gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Iinclude
BUILD = build

# Binarios
BIN_BATALLA = $(BUILD)/batalla-naval
BIN_DEMO = $(BUILD)/demo_ranking
BIN_TEST = $(BUILD)/test_ranking

# Fuentes del modulo de ranking (demo y pruebas).
SRC_DEMO = src/demo_ranking.c src/ranking.c
SRC_TEST = tests/test_ranking.c src/ranking.c

# Fuentes de la integracion completa.
SRC_APP = src/main.c src/app.c src/ui_cli.c src/tablero.c src/ranking.c

# Por defecto: todo lo que debe compilar la entrega.
all: $(BIN_BATALLA) $(BIN_TEST) $(BIN_DEMO)

$(BUILD):
	mkdir -p $(BUILD)

$(BIN_DEMO): $(SRC_DEMO) include/ranking.h include/config.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_DEMO) -o $(BIN_DEMO)

$(BIN_TEST): $(SRC_TEST) include/ranking.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_TEST) -o $(BIN_TEST)

$(BIN_BATALLA): $(SRC_APP) include/app.h include/ranking.h include/config.h include/ui_cli.h include/tablero.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_APP) -o $(BIN_BATALLA)

app: $(BIN_BATALLA)

test: $(BIN_TEST)
	./$(BIN_TEST)

demo: $(BIN_DEMO)
	@echo "Ejecute desde la raiz: ./$(BIN_DEMO) [NombreGanador]"

run: $(BIN_BATALLA)
	./$(BIN_BATALLA)

clean:
	rm -f $(BIN_BATALLA) $(BIN_DEMO) $(BIN_TEST)
	rm -f $(BUILD)/*.o $(BUILD)/*.exe
	rm -f data/ranking_test_*.dat

.PHONY: all app test demo run clean
