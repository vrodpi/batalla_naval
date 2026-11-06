/**
 * @file test_game.c
 * @brief Pruebas automaticas del motor de partida.
 *
 * Estrategia: sin frameworks externos, un contador de aserciones y una
 * funcion chequear() que imprime "ok" o "FALLO". No se pide entrada por
 * teclado: se prueban las funciones puras de inicializacion, colocacion,
 * disparo, hundimiento y fin de juego.
 *
 * @note Compilar y ejecutar con `make test-game` o `make test`; devuelve 0
 *       si todo pasa y 1 si algo falla.
 */

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "game.h"
#include "tablero.h"

/**
 * @brief Cantidad total de aserciones ejecutadas.
 */
static int pruebas = 0;

/**
 * @brief Cantidad de aserciones que fallaron.
 */
static int fallos = 0;

/**
 * @brief Registra una asercion y muestra el nombre del caso.
 *
 * @param condicion Resultado de la condicion verificada.
 * @param nombre Texto descriptivo del caso.
 * @return No devuelve valor.
 */
static void chequear(int condicion, const char *nombre)
{
    pruebas++;
    if (condicion) {
        printf("ok   - %s\n", nombre);
    } else {
        printf("FALLO- %s\n", nombre);
        fallos++;
    }
}

/**
 * @brief Comprueba que una partida nueva quede vacia y con la flota estandar.
 *
 * @return No devuelve valor.
 */
static void prueba_inicializacion(void)
{
    Partida partida; /* Partida recien inicializada. */
    int i;           /* Fila. */
    int j;           /* Columna. */
    int todo_agua;   /* Bandera de tableros vacios. */

    inicializar_partida(&partida);

    chequear(partida.turno == PJ1, "turno inicial PJ1");
    chequear(partida.cantidad_turnos == 0, "cantidad de turnos en cero");
    chequear(partida.victoria == 0 && partida.ganador == 0,
             "partida sin ganador");
    chequear(partida.jugador1.barcos_vivos == CONFIG_NUM_BARCOS,
             "jugador 1 empieza con 5 barcos");
    chequear(partida.jugador2.barcos_vivos == CONFIG_NUM_BARCOS,
             "jugador 2 empieza con 5 barcos");
    chequear(partida.jugador1.hits == 0 && partida.jugador2.hits == 0,
             "impactos iniciales en cero");
    chequear(partida.jugador1.nombre[0] == '\0'
                 && partida.jugador2.nombre[0] == '\0',
             "nombres iniciales vacios");

    todo_agua = 1;
    for (i = 0; i < CONFIG_FILAS; i++) {
        for (j = 0; j < CONFIG_COLUMNAS; j++) {
            if (partida.jugador1.tableros.tablero[i][j] != (char)agua
                || partida.jugador1.tableros.tablero_disparos[i][j]
                       != (char)agua
                || partida.jugador2.tableros.tablero[i][j] != (char)agua
                || partida.jugador2.tableros.tablero_disparos[i][j]
                       != (char)agua) {
                todo_agua = 0;
            }
        }
    }
    chequear(todo_agua, "los cuatro tableros empiezan en agua");

    chequear(partida.jugador1.barco[0].tamanio == porta_aviones
                 && partida.jugador1.barco[1].tamanio == buque
                 && partida.jugador1.barco[2].tamanio == submarino
                 && partida.jugador1.barco[3].tamanio == lancha
                 && partida.jugador1.barco[4].tamanio == velero,
             "flota estandar 5-4-3-2-1");
    chequear(partida.jugador1.barco[0].fila == -1
                 && partida.jugador1.barco[0].columna == -1,
             "los barcos empiezan sin colocar");
    chequear(partida.jugador1.barco[0].hundido == 0
                 && partida.jugador2.barco[4].hundido == 0,
             "los barcos empiezan a flote");
}

/**
 * @brief Comprueba la colocacion de barcos y las reglas de contacto.
 *
 * @return No devuelve valor.
 */
static void prueba_colocacion(void)
{
    Partida partida; /* Partida de prueba. */
    int i;           /* Contador auxiliar. */

    inicializar_partida(&partida);

    chequear(game_colocar_barco(&partida.jugador1, 0, 0, 0, horizontal) == 1,
             "coloca el portaaviones en 0,0");
    for (i = 0; i < CONFIG_BARCO_LEN_0; i++) {
        chequear(partida.jugador1.tableros.tablero[0][i] == (char)barco,
                 "el portaaviones marca sus 5 casillas");
    }
    chequear(partida.jugador1.barco[0].fila == 0
                 && partida.jugador1.barco[0].columna == 0
                 && partida.jugador1.barco[0].orientacion == horizontal,
             "guarda la posicion del portaaviones");

    chequear(game_colocar_barco(&partida.jugador1, 1, 0, 0, horizontal) == 0,
             "rechaza un barco superpuesto");
    chequear(game_colocar_barco(&partida.jugador1, 1, 1, 0, horizontal) == 0,
             "rechaza un barco que toca al anterior");
    chequear(game_colocar_barco(&partida.jugador1, 1, 2, 0, horizontal) == 1,
             "acepta un barco separado por una fila de agua");

    chequear(game_colocar_barco(&partida.jugador1, 2, 9, 8, horizontal) == 0,
             "rechaza un barco que se sale por la derecha");
    chequear(game_colocar_barco(&partida.jugador1, 2, 9, 9, vertical) == 0,
             "rechaza un barco que se sale por abajo");
    chequear(game_colocar_barco(&partida.jugador1, 4, 9, 9, horizontal) == 1,
             "coloca el velero en la esquina inferior derecha");
    chequear(game_colocar_barco(&partida.jugador1, 4, 9, 9, horizontal) == 0,
             "no permite recolocar un barco ya colocado");

    chequear(game_colocar_barco(&partida.jugador1, -1, 0, 0, horizontal) == 0,
             "rechaza un indice de barco negativo");
    chequear(game_colocar_barco(&partida.jugador1, CONFIG_NUM_BARCOS, 0, 0,
                                horizontal) == 0,
             "rechaza un indice de barco fuera de la flota");
    chequear(game_colocar_barco(NULL, 0, 0, 0, horizontal) == 0,
             "rechaza un jugador NULL");
    chequear(partida.jugador1.barcos_vivos == CONFIG_NUM_BARCOS,
             "colocar barcos no descuenta barcos vivos");
}

/**
 * @brief Comprueba el resultado de los disparos sobre un tablero.
 *
 * @return No devuelve valor.
 */
static void prueba_disparos(void)
{
    Partida partida; /* Partida de prueba. */

    inicializar_partida(&partida);

    /* El velero ocupa una sola casilla: 9,9. */
    chequear(game_colocar_barco(&partida.jugador2, 4, 9, 9, horizontal) == 1,
             "coloca el velero del jugador 2");

    chequear(recibir_disparo(0, 0, partida.jugador2.tableros.tablero)
                 == disp_agua,
             "disparo al agua devuelve disp_agua");
    chequear(partida.jugador2.tableros.tablero[0][0]
                 == (char)disparo_a_agua,
             "el agua disparada se marca con /");
    chequear(recibir_disparo(0, 0, partida.jugador2.tableros.tablero)
                 == disp_repetido,
             "repetir un disparo al agua devuelve disp_repetido");

    chequear(recibir_disparo(9, 9, partida.jugador2.tableros.tablero)
                 == disp_impacto,
             "disparo a un barco devuelve disp_impacto");
    chequear(partida.jugador2.tableros.tablero[9][9] == (char)impacto,
             "el impacto se marca con X");
    chequear(recibir_disparo(9, 9, partida.jugador2.tableros.tablero)
                 == disp_repetido,
             "repetir un impacto devuelve disp_repetido");

    chequear(recibir_disparo(-1, 0, partida.jugador2.tableros.tablero)
                 == disp_NO_valido,
             "fila negativa devuelve disp_NO_valido");
    chequear(recibir_disparo(0, CONFIG_COLUMNAS,
                             partida.jugador2.tableros.tablero)
                 == disp_NO_valido,
             "columna fuera del tablero devuelve disp_NO_valido");
    chequear(recibir_disparo(0, 0, NULL) == disp_NO_valido,
             "tablero NULL devuelve disp_NO_valido");
}

/**
 * @brief Comprueba el hundimiento de barcos completos e incompletos.
 *
 * @return No devuelve valor.
 */
static void prueba_hundido(void)
{
    Partida partida; /* Partida de prueba. */
    int i;           /* Contador de disparos. */

    inicializar_partida(&partida);
    chequear(game_colocar_barco(&partida.jugador1, 0, 0, 0, horizontal) == 1,
             "coloca el portaaviones para hundirlo");

    /* Impactar solo 3 de las 5 casillas: el barco sigue a flote. */
    for (i = 0; i < 3; i++) {
        (void)recibir_disparo(0, i, partida.jugador1.tableros.tablero);
    }
    barco_hundido(&partida.jugador1);
    chequear(partida.jugador1.barco[0].hundido == 0,
             "un barco con impactos parciales no se hunde");
    chequear(partida.jugador1.barcos_vivos == CONFIG_NUM_BARCOS,
             "los impactos parciales no descuentan barcos vivos");

    /* Completar los impactos: ahora si debe hundirse. */
    for (i = 3; i < CONFIG_BARCO_LEN_0; i++) {
        (void)recibir_disparo(0, i, partida.jugador1.tableros.tablero);
    }
    barco_hundido(&partida.jugador1);
    chequear(partida.jugador1.barco[0].hundido == 1,
             "el portaaviones queda hundido");
    chequear(partida.jugador1.barcos_vivos == CONFIG_NUM_BARCOS - 1,
             "hundir un barco descuenta un barco vivo");

    /* Llamar de nuevo no debe descontar dos veces. */
    barco_hundido(&partida.jugador1);
    chequear(partida.jugador1.barcos_vivos == CONFIG_NUM_BARCOS - 1,
             "un barco ya hundido no se descuenta dos veces");
    chequear(partida.jugador1.barco[1].hundido == 0,
             "los barcos no colocados siguen sin hundirse");
}

/**
 * @brief Comprueba la condicion de fin de juego y el ganador.
 *
 * @return No devuelve valor.
 */
static void prueba_fin_juego(void)
{
    Partida partida; /* Partida de prueba. */

    inicializar_partida(&partida);
    chequear(fin_juego(&partida) == 0, "la partida continua sin bajas");

    partida.jugador2.barcos_vivos = 0;
    chequear(fin_juego(&partida) == 1, "fin_juego detecta al jugador 2 sin barcos");
    chequear(partida.ganador == 1, "gana el jugador 1");
    chequear(partida.victoria == 1, "la partida queda marcada como terminada");

    inicializar_partida(&partida);
    partida.jugador1.barcos_vivos = 0;
    chequear(fin_juego(&partida) == 1, "fin_juego detecta al jugador 1 sin barcos");
    chequear(partida.ganador == 2, "gana el jugador 2");
    chequear(fin_juego(NULL) == 0, "fin_juego tolera una partida NULL");
}

/**
 * @brief Comprueba el cambio de turno y el registro de disparos.
 *
 * @return No devuelve valor.
 */
static void prueba_turnos(void)
{
    Partida partida; /* Partida de prueba. */

    inicializar_partida(&partida);
    chequear(game_colocar_barco(&partida.jugador2, 4, 9, 9, horizontal) == 1,
             "coloca el velero del jugador 2 para los turnos");

    chequear(game_realizar_disparo(&partida, 0, 0) == disp_agua,
             "el jugador 1 dispara al agua");
    chequear(partida.turno == PJ2, "el turno pasa al jugador 2");
    chequear(partida.cantidad_turnos == 1, "el disparo cuenta un turno");
    chequear(partida.jugador1.tableros.tablero_disparos[0][0]
                 == (char)disparo_a_agua,
             "el mapa del atacante marca el agua");

    chequear(game_realizar_disparo(&partida, 9, 9) == disp_agua,
             "el jugador 2 dispara al agua");
    chequear(partida.turno == PJ1, "el turno vuelve al jugador 1");
    chequear(partida.cantidad_turnos == 2, "el segundo disparo cuenta otro turno");

    chequear(game_realizar_disparo(&partida, 0, 0) == disp_repetido,
             "el jugador 1 repite un disparo");
    chequear(partida.turno == PJ1, "un disparo repetido no cambia el turno");
    chequear(partida.cantidad_turnos == 2,
             "un disparo repetido no cuenta un turno");

    chequear(game_realizar_disparo(&partida, 9, 9) == disp_impacto,
             "el jugador 1 acierta el velero");
    chequear(partida.turno == PJ2, "el impacto cambia el turno");
    chequear(partida.cantidad_turnos == 3, "el impacto cuenta un turno");
    chequear(partida.jugador1.hits == 1, "el impacto suma un hit al atacante");
    chequear(partida.jugador2.barco[4].hundido == 1,
             "el velero del jugador 2 queda hundido");
    chequear(partida.jugador2.barcos_vivos == CONFIG_NUM_BARCOS - 1,
             "el velero hundido descuenta un barco vivo");

    chequear(game_realizar_disparo(&partida, -1, 0) == disp_NO_valido,
             "un disparo invalido no se aplica");
    chequear(partida.turno == PJ2, "un disparo invalido no cambia el turno");
    chequear(partida.cantidad_turnos == 3,
             "un disparo invalido no cuenta un turno");
    chequear(game_realizar_disparo(NULL, 0, 0) == disp_NO_valido,
             "game_realizar_disparo tolera una partida NULL");
}

/**
 * @brief Ejecuta todas las baterias y resume el resultado.
 *
 * @return 0 si todas las aserciones pasaron; 1 si hubo fallos.
 */
int main(void)
{
    printf("== test_game ==\n");

    prueba_inicializacion();
    prueba_colocacion();
    prueba_disparos();
    prueba_hundido();
    prueba_fin_juego();
    prueba_turnos();

    printf("------------------\n");
    printf("Pruebas: %d, Fallos: %d\n", pruebas, fallos);
    if (fallos == 0) {
        printf("TODO OK\n");
        return 0;
    }
    printf("HAY FALLOS\n");
    return 1;
}
