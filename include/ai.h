#ifndef BATALLA_NAVAL_AI_H
#define BATALLA_NAVAL_AI_H

#include "game.h"

/*
 * ai.h - Estrategia de la maquina para el modo Jugador vs IA.
 *
 * La IA solo propone coordenadas. Las reglas y los cambios de estado se
 * aplican con las funciones de game.c/tablero.c, igual que en PvP.
 *
 * La IA no recibe el tablero rival ni contadores de barcos vivos. Solo
 * usa su propio tablero_disparos y el resultado publico de cada disparo.
 *
 * Estrategia:
 * 1. Caza: elige al azar una casilla que todavia no haya recibido un disparo.
 * 2. Remate: si impacta, prueba las casillas contiguas desconocidas.
 * 3. Si no quedan objetivos pendientes, vuelve a cazar.
 */

typedef struct {
    int objetivos[FIL * COL][2];
    int num_objetivos;
} EstadoIA;

/* Deja el estado de la IA listo para una partida nueva. */
void ai_inicializar(EstadoIA *estado);

/*
 * Coloca la flota de la maquina de mayor a menor longitud.
 * Devuelve 1 si pudo ubicar todos los barcos y 0 si no fue posible.
 */
int ai_colocar_flota(Jugador *maquina);

/*
 * Elige una coordenada valida para disparar.
 * Devuelve 1 si encontro una casilla y 0 si no quedan casillas disponibles.
 */
int ai_elegir_disparo(const Jugador *maquina, EstadoIA *estado,
                      int *fila, int *columna);

/*
 * Actualiza los objetivos de la IA despues de un disparo.
 * Solo recibe el resultado publico del disparo; nunca la cantidad de
 * barcos vivos del oponente ni su tablero.
 */
void ai_registrar_resultado(EstadoIA *estado, const Jugador *maquina,
                            int fila, int columna, Rdo_Disparo resultado);

#endif
