#ifndef BATALLA_NAVAL_CONFIG_H
#define BATALLA_NAVAL_CONFIG_H

/*
 * config.h - Parametros configurables del juego.
 *
 * El tamano del tablero, la flota, los nombres y las rutas de datos se
 * definen aqui para poder cambiarlos sin tocar la logica de tablero.c,
 * app.c, ui_cli.c ni ranking.c.
 *
 * Todos los modulos deben incluir este archivo en lugar de repetir
 * literales como 10, 5 o "data/ranking.dat".
 *
 * Estandar del proyecto: C99 (-std=c99 -pedantic -Wall -Wextra).
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

/*
 * Cantidad de jugadores humanos en una partida local.
 * En el modo jugador vs maquina solo se usa el primero.
 */
#define CONFIG_JUGADORES_PARTIDA 2

/*
 * Largo maximo de un nombre de jugador, incluido el '\0'.
 * Debe coincidir con RANKING_NOMBRE_MAX (include/ranking.h): la interfaz
 * captura el nombre en un buffer de este tamano y ranking.c lo valida y
 * lo guarda en su propio arreglo de 32 bytes.
 */
#define CONFIG_NOMBRE_MAX 32

/*
 * Ruta del reglamento de texto. Es relativa a la raiz del proyecto:
 * el programa debe ejecutarse desde la raiz para encontrarla.
 */
#define CONFIG_REGLAMENTO_RUTA "files/Reglamento.txt"

/*
 * Ruta por defecto del ranking (relativa a la raiz del proyecto).
 * El programa debe ejecutarse desde la raiz para que exista ./data/.
 * El archivo es BINARIO (volcado del struct): no editarlo a mano.
 */
#define CONFIG_RANKING_RUTA "data/ranking.dat"

#endif
