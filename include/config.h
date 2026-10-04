#ifndef BATALLA_NAVAL_CONFIG_H
#define BATALLA_NAVAL_CONFIG_H

/*
 * config.h - Parametros configurables del juego.
 *
 * El tamano del tablero y la flota se definen aqui para poder
 * cambiarlos sin tocar la logica de board.c / game.c / ai.c.
 * Todos los modulos deben incluir este archivo en lugar de
 * repetir literales 10 o 5 en el codigo.
 */

/* Tablero cuadrado inicial de 10 x 10. */
#define CONFIG_FILAS 10
#define CONFIG_COLUMNAS 10

/* Flota: cantidad de barcos y sus longitudes (mayor a menor). */
#define CONFIG_NUM_BARCOS 5

#define CONFIG_BARCO_LEN_0 5
#define CONFIG_BARCO_LEN_1 4
#define CONFIG_BARCO_LEN_2 3
#define CONFIG_BARCO_LEN_3 2
#define CONFIG_BARCO_LEN_4 1

/* Ruta por defecto del ranking (relativa a la raiz del proyecto). */
/* El programa debe ejecutarse desde la raiz para que exista ./data/. */
#define CONFIG_RANKING_RUTA "data/ranking.dat"

#endif
