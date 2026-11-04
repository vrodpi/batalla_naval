/**
 * @file config.h
 * @brief Parametros configurables del juego Batalla Naval.
 *
 * El tamano del tablero, la flota, los nombres y las rutas de datos se
 * definen aqui para poder cambiarlos sin tocar la logica de tablero.c,
 * app.c, ui_cli.c ni ranking.c.
 *
 * @note Todos los modulos deben incluir este archivo en lugar de repetir
 *       literales como 10, 5 o "data/ranking.dat".
 * @note Estandar unico del proyecto: C99 (-std=c99 -pedantic -Wall -Wextra).
 */

#ifndef BATALLA_NAVAL_CONFIG_H
#define BATALLA_NAVAL_CONFIG_H

/**
 * @brief Cantidad de filas del tablero.
 * @note Las filas internas van de 0 a CONFIG_FILAS - 1.
 */
#define CONFIG_FILAS 10

/**
 * @brief Cantidad de columnas del tablero.
 * @note Las columnas internas van de 0 a CONFIG_COLUMNAS - 1.
 */
#define CONFIG_COLUMNAS 10

/**
 * @brief Cantidad de barcos que tiene cada jugador.
 * @note La flota se define con un barco de cada longitud: 5, 4, 3, 2 y 1.
 */
#define CONFIG_NUM_BARCOS 5

/**
 * @brief Longitud del portaaviones (primer barco de la flota).
 */
#define CONFIG_BARCO_LEN_0 5

/**
 * @brief Longitud del buque (segundo barco de la flota).
 */
#define CONFIG_BARCO_LEN_1 4

/**
 * @brief Longitud del submarino (tercer barco de la flota).
 */
#define CONFIG_BARCO_LEN_2 3

/**
 * @brief Longitud de la lancha (cuarto barco de la flota).
 */
#define CONFIG_BARCO_LEN_3 2

/**
 * @brief Longitud del velero (quinto barco de la flota).
 */
#define CONFIG_BARCO_LEN_4 1

/**
 * @brief Cantidad de jugadores humanos en una partida local.
 * @note En el modo jugador vs maquina solo se usa el primero.
 */
#define CONFIG_JUGADORES_PARTIDA 2

/**
 * @brief Largo maximo de un nombre de jugador, incluido el '\0'.
 * @note Debe coincidir con RANKING_NOMBRE_MAX (include/ranking.h): la
 *       interfaz captura el nombre en un buffer de este tamano y ranking.c
 *       lo valida y lo guarda en su propio arreglo de 32 bytes.
 */
#define CONFIG_NOMBRE_MAX 32

/**
 * @brief Ruta del reglamento de texto.
 * @note Es relativa a la raiz del proyecto: el programa debe ejecutarse
 *       desde la raiz para encontrarla.
 */
#define CONFIG_REGLAMENTO_RUTA "files/Reglamento.txt"

/**
 * @brief Ruta por defecto del ranking binario.
 * @note Es relativa a la raiz del proyecto. El archivo es un volcado del
 *       struct Ranking y no debe editarse a mano.
 */
#define CONFIG_RANKING_RUTA "data/ranking.dat"

#endif /* BATALLA_NAVAL_CONFIG_H */
