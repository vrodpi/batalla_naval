#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ai.h"
#include "game.h"
#include "tablero.h"

/* Cuenta cuantas casillas de barco quedan en el tablero. */
static int contar_casillas_barco(const char tablero[FIL][COL])
{
    int cantidad = 0;

    for (int fila = 0; fila < FIL; fila++) {
        for (int columna = 0; columna < COL; columna++) {
            if (tablero[fila][columna] == CASILLA_BARCO) {
                cantidad++;
            }
        }
    }

    return cantidad;
}

/* La IA debe colocar siempre los cinco barcos, sin salirse ni superponer. */
static void probar_colocacion(void)
{
    for (int semilla = 0; semilla < 200; semilla++) {
        Partida partida;
        int casillas_esperadas = 0;

        srand(semilla);
        inicializar_partida(&partida);

        assert(ai_colocar_flota(&partida.jugador2) == 1);
        assert(partida.jugador2.barcos_vivos == BARCOS_MODO_NORMAL);

        for (int i = 0; i < BARCOS_MODO_NORMAL; i++) {
            assert(partida.jugador2.barco[i].tamanio == i + 1);
            assert(partida.jugador2.barco[i].hundido == 0);
            casillas_esperadas += partida.jugador2.barco[i].tamanio;
        }

        assert(contar_casillas_barco(partida.jugador2.tableros.tablero) ==
               casillas_esperadas);
    }

    printf("  colocacion de flota: OK\n");
}

/* La IA no debe repetir disparos ni salirse del tablero. */
static void probar_disparos_unicos(void)
{
    Partida partida;
    EstadoIA estado;
    int vistos[FIL][COL] = {0};

    srand(1234);
    inicializar_partida(&partida);
    ai_inicializar(&estado);

    for (int i = 0; i < FIL * COL; i++) {
        int fila;
        int columna;

        assert(ai_elegir_disparo(&partida.jugador2, &estado,
                                 &fila, &columna) == 1);
        assert(fila >= 0 && fila < FIL);
        assert(columna >= 0 && columna < COL);
        assert(vistos[fila][columna] == 0);

        vistos[fila][columna] = 1;
        partida.jugador2.tableros.tablero_disparos[fila][columna] =
            CASILLA_DISP_AGUA;
    }

    {
        int fila;
        int columna;

        assert(ai_elegir_disparo(&partida.jugador2, &estado,
                                 &fila, &columna) == 0);
    }

    printf("  disparos unicos: OK\n");
}

/* Despues de un impacto se deben probar las cuatro casillas contiguas. */
static void probar_remate(void)
{
    Partida partida;
    EstadoIA estado;
    const int vecinos[4][2] = {{4, 5}, {6, 5}, {5, 4}, {5, 6}};

    inicializar_partida(&partida);
    ai_inicializar(&estado);

    partida.jugador2.tableros.tablero_disparos[5][5] = CASILLA_IMPACTO;
    ai_registrar_resultado(&estado, &partida.jugador2, 5, 5,
                           disp_impacto, 5, 5);
    assert(estado.num_objetivos == 4);

    for (int i = 0; i < 4; i++) {
        int fila;
        int columna;
        int es_vecino = 0;

        assert(ai_elegir_disparo(&partida.jugador2, &estado,
                                 &fila, &columna) == 1);

        for (int j = 0; j < 4; j++) {
            if (fila == vecinos[j][0] && columna == vecinos[j][1]) {
                es_vecino = 1;
            }
        }

        assert(es_vecino);
        partida.jugador2.tableros.tablero_disparos[fila][columna] =
            CASILLA_DISP_AGUA;
    }

    assert(estado.num_objetivos == 0);

    /* Si el impacto hunde un barco, se descartan los objetivos pendientes. */
    partida.jugador2.tableros.tablero_disparos[2][2] = CASILLA_IMPACTO;
    ai_registrar_resultado(&estado, &partida.jugador2, 2, 2,
                           disp_impacto, 5, 4);
    assert(estado.num_objetivos == 0);

    printf("  remate de impactos: OK\n");
}

/*
 * Simula una partida completa entre dos IA. Sirve para comprobar que la
 * estrategia siempre termina y que nunca repite un disparo.
 */
static void simular_partida(int semilla, Dificultad dificultad)
{
    Partida partida;
    EstadoIA estado1;
    EstadoIA estado2;
    int disparos = 0;

    srand(semilla);
    inicializar_partida(&partida);
    partida.dificultad = dificultad;

    ai_inicializar(&estado1);
    ai_inicializar(&estado2);

    assert(ai_colocar_flota(&partida.jugador1) == 1);
    assert(ai_colocar_flota(&partida.jugador2) == 1);

    while (comprobar_partida_terminada(&partida) == 0) {
        Jugador *atacante;
        Jugador *defensor;
        EstadoIA *estado;
        int fila;
        int columna;
        int barcos_antes;
        int barcos_despues;
        Rdo_Disparo disparo;

        if (partida.turno == PJ1) {
            atacante = &partida.jugador1;
            defensor = &partida.jugador2;
            estado = &estado1;
        } else {
            atacante = &partida.jugador2;
            defensor = &partida.jugador1;
            estado = &estado2;
        }

        assert(ai_elegir_disparo(atacante, estado, &fila, &columna) == 1);

        disparo = recibir_disparo(fila, columna, defensor->tableros.tablero);
        assert(disparo != disp_repetido);

        barcos_antes = defensor->barcos_vivos;
        aplicar_disparo(atacante, defensor, fila, columna, disparo);
        barcos_despues = defensor->barcos_vivos;

        ai_registrar_resultado(estado, atacante, fila, columna,
                               disparo, barcos_antes, barcos_despues);

        modo_tormenta(&partida);
        cambiar_turno(&partida);

        disparos++;
        assert(disparos <= FIL * COL * 2);
    }

    assert(partida.ganador == 1 || partida.ganador == 2);
}

int main(void)
{
    printf("test_ai:\n");

    probar_colocacion();
    probar_disparos_unicos();
    probar_remate();

    for (int semilla = 0; semilla < 100; semilla++) {
        simular_partida(semilla, NORMAL);
        simular_partida(semilla, TORMENTA);
    }

    printf("  simulaciones Normal/Tormenta: OK (200 partidas)\n");
    printf("test_ai: OK\n");

    return 0;
}
