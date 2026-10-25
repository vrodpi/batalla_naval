# Makefile - Batalla Naval (modulo ranking con formato binario, Opcion A)
#
# Requiere GCC o Clang y make. El objetivo de compilacion acordado es Linux.
#
# Uso, desde la raiz del proyecto (donde esta data/):
#   make            -> compila y deja listos demo y tests del ranking
#   make test       -> ejecuta las pruebas del ranking
#   make demo       -> compila la demo de fin de partida
#   make app        -> compila el juego completo (pendiente de integracion
#                      de app.c/ui_cli.c/tablero.c; ver nota abajo)
#   make clean      -> borra binarios y temporales de prueba
#
# Estandar: -ansi -pedantic -Wall -Wextra garantiza ANSI C (C89) en el
# modulo de ranking. El archivo de datos es un volcado binario del struct
# Ranking (sin serializacion byte a byte): ver docs/contrato-ranking.md.
#
# NOTA DE INTEGRACION: src/main.c, src/app.c, src/ui_cli.c y src/tablero.c
# pertenecen a otros integrantes y a la fecha de este commit todavia no
# compilan en -ansi (usan comentarios //, declaraciones dentro de for y
# tienen errores de sintaxis). Por eso `app` no forma parte de `all`:
# no bloquea el modulo de ranking, que es el que se prueba con `make test`.

CC ?= gcc
CFLAGS = -ansi -pedantic -Wall -Wextra -Iinclude
BUILD = build

# Binarios
BIN_BATALLA = $(BUILD)/batalla-naval
BIN_DEMO = $(BUILD)/demo_ranking
BIN_TEST = $(BUILD)/test_ranking

# Fuentes del modulo de ranking (las que se compilan y prueban aca).
SRC_DEMO = src/demo_ranking.c src/ranking.c
SRC_TEST = tests/test_ranking.c src/ranking.c

# Fuentes de la integracion completa. Se usan solo en `make app`.
SRC_APP = src/main.c src/app.c src/ui_cli.c src/tablero.c src/ranking.c

# Por defecto: entregables del ranking (no bloquean por modulos ajenos).
all: $(BIN_TEST) $(BIN_DEMO)

$(BUILD):
	mkdir -p $(BUILD)

$(BIN_DEMO): $(SRC_DEMO) include/ranking.h include/config.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_DEMO) -o $(BIN_DEMO)

$(BIN_TEST): $(SRC_TEST) include/ranking.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_TEST) -o $(BIN_TEST)

# Integracion completa. Requiere que app/ui/tablero compilen; ver nota.
$(BIN_BATALLA): $(SRC_APP) include/ranking.h include/config.h include/ui_cli.h include/tablero.h | $(BUILD)
	$(CC) $(CFLAGS) $(SRC_APP) -o $(BIN_BATALLA)

app: $(BIN_BATALLA)

test: $(BIN_TEST)
	./$(BIN_TEST)

demo: $(BIN_DEMO)
	@echo "Ejecute desde la raiz: ./$(BIN_DEMO) [NombreGanador]"

clean:
	rm -f $(BIN_BATALLA) $(BIN_DEMO) $(BIN_TEST)
	rm -f $(BUILD)/*.o $(BUILD)/*.exe
	rm -f data/ranking_test_*.dat

.PHONY: all app test demo clean
