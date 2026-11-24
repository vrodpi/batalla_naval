#include "../include/ai.h"

#include <stdlib.h>

/* Intentos para ubicar un barco antes de reintentar la flota completa. */
#define AI_INTENTOS_POR_BARCO 1000

/* Reintentos de la flota completa antes de usar el respaldo fijo. */
#define AI_INTENTOS_POR_FLOTA 100

/* Devuelve 1 si la coordenada pertenece al tablero. */
static int dentro_del_tablero(int fila, int columna)
{
    return fila >= 0 && fila < FIL && columna >= 0 && columna < COL;
}

/* Devuelve 1 si la coordenada ya esta pendiente en la lista de objetivos. */
static int objetivo_repetido(const EstadoIA *estado, int fila, int columna)
{
    for (int i = 0; i < estado->num_objetivos; i++) {
        if (estado->objetivos[i][0] == fila &&
            estado->objetivos[i][1] == columna) {
            return 1;
        }
    }

    return 0;
}

/*
 * Agrega una coordenada a la lista de objetivos si esta dentro del tablero,
 * sigue sin disparar y no estaba ya pendiente.
 */
static void agregar_objetivo(EstadoIA *estado, const Jugador *maquina,
                             int fila, int columna)
{
    if (!dentro_del_tablero(fila, columna)) {
        return;
    }

    if (estado->num_objetivos >= FIL * COL) {
        return;
    }

    if (maquina->tableros.tablero_disparos[fila][columna] != CASILLA_AGUA) {
        return;
    }

    if (objetivo_repetido(estado, fila, columna)) {
        return;
    }

    estado->objetivos[estado->num_objetivos][0] = fila;
    estado->objetivos[estado->num_objetivos][1] = columna;
    estado->num_objetivos++;
}

/* Intenta ubicar un barco en una posicion y orientacion aleatorias. */
static int colocar_barco_aleatorio(Jugador *maquina, int indice)
{
    for (int intento = 0; intento < AI_INTENTOS_POR_BARCO; intento++) {
        Barco barco = maquina->barco[indice];

        barco.orientacion = (rand() % 2 == 0) ? horizontal : vertical;
        barco.fila = rand() % FIL;
        barco.columna = rand() % COL;
        barco.hundido = 0;

        /*
         * validar_ingreso_barco devuelve 1 solo cuando pudo colocar el
         * barco. Los valores negativos indican el motivo del rechazo.
         */
        if (validar_ingreso_barco(barco, maquina->tableros.tablero) == 1) {
            maquina->barco[indice] = barco;
            return 1;
        }
    }

    return 0;
}

/*
 * Respaldo deterministico: una fila por barco, separadas por una fila vacia.
 * Sirve para garantizar que la partida pueda comenzar aunque el azar falle.
 *
 * Requiere que el tablero llegue limpio. ai_colocar_flota se encarga de
 * limpiarlo antes de llamar a esta funcion.
 */
static int colocar_flota_de_respaldo(Jugador *maquina)
{
    for (int indice = 0; indice < BARCOS_MODO_NORMAL; indice++) {
        Barco barco = maquina->barco[indice];

        barco.orientacion = horizontal;
        barco.fila = (BARCOS_MODO_NORMAL - 1 - indice) * 2;
        barco.columna = 0;
        barco.hundido = 0;

        if (validar_ingreso_barco(barco, maquina->tableros.tablero) != 1) {
            return 0;
        }

        maquina->barco[indice] = barco;
    }

    return 1;
}

void ai_inicializar(EstadoIA *estado)
{
    if (estado != NULL) {
        estado->num_objetivos = 0;
    }
}

int ai_colocar_flota(Jugador *maquina)
{
    if (maquina == NULL) {
        return 0;
    }

    /*
     * Se intenta la flota completa varias veces porque una colocacion
     * aleatoria puede dejar sin espacio a los barcos mas chicos.
     */
    for (int intento = 0; intento < AI_INTENTOS_POR_FLOTA; intento++) {
        int flota_completa = 1;

        inicializar_tablero(maquina->tableros.tablero,
                            maquina->tableros.tablero_disparos);

        for (int indice = BARCOS_MODO_NORMAL - 1; indice >= 0; indice--) {
            if (!colocar_barco_aleatorio(maquina, indice)) {
                flota_completa = 0;
                break;
            }
        }

        if (flota_completa) {
            return 1;
        }
    }

    /*
     * El ultimo intento pudo dejar una flota parcial. Se limpia el tablero
     * para que el respaldo deterministico empiece desde cero.
     */
    inicializar_tablero(maquina->tableros.tablero,
                        maquina->tableros.tablero_disparos);

    return colocar_flota_de_respaldo(maquina);
}

/* Toma un objetivo pendiente que siga siendo una casilla desconocida. */
static int elegir_de_objetivos(const Jugador *maquina, EstadoIA *estado,
                               int *fila, int *columna)
{
    while (estado->num_objetivos > 0) {
        int indice = estado->num_objetivos - 1;
        int f = estado->objetivos[indice][0];
        int c = estado->objetivos[indice][1];

        estado->num_objetivos--;

        if (dentro_del_tablero(f, c) &&
            maquina->tableros.tablero_disparos[f][c] == CASILLA_AGUA) {
            *fila = f;
            *columna = c;
            return 1;
        }
    }

    return 0;
}

/* Elige al azar entre todas las casillas que todavia no recibieron disparo. */
static int elegir_al_azar(const Jugador *maquina, int *fila, int *columna)
{
    int candidatos[FIL * COL][2];
    int cantidad = 0;

    for (int f = 0; f < FIL; f++) {
        for (int c = 0; c < COL; c++) {
            if (maquina->tableros.tablero_disparos[f][c] == CASILLA_AGUA) {
                candidatos[cantidad][0] = f;
                candidatos[cantidad][1] = c;
                cantidad++;
            }
        }
    }

    if (cantidad == 0) {
        return 0;
    }

    int elegido = rand() % cantidad;

    *fila = candidatos[elegido][0];
    *columna = candidatos[elegido][1];

    return 1;
}

int ai_elegir_disparo(const Jugador *maquina, EstadoIA *estado,
                      int *fila, int *columna)
{
    if (maquina == NULL || estado == NULL || fila == NULL || columna == NULL) {
        return 0;
    }

    if (elegir_de_objetivos(maquina, estado, fila, columna)) {
        return 1;
    }

    return elegir_al_azar(maquina, fila, columna);
}

void ai_registrar_resultado(EstadoIA *estado, const Jugador *maquina,
                            int fila, int columna, Rdo_Disparo resultado)
{
    if (estado == NULL || maquina == NULL) {
        return;
    }

    /*
     * Solo se usan resultados publicos del disparo y el tablero_disparos
     * propio. No se consulta barcos_vivos ni el tablero rival.
     */
    if (resultado != disp_impacto) {
        return;
    }

    agregar_objetivo(estado, maquina, fila - 1, columna);
    agregar_objetivo(estado, maquina, fila + 1, columna);
    agregar_objetivo(estado, maquina, fila, columna - 1);
    agregar_objetivo(estado, maquina, fila, columna + 1);
}
