/**
 * @file tablero.h
 * @brief Modelo del tablero y de la flota.
 *
 * Define los tipos de datos del tablero (celdas, orientacion, tamanio de
 * barco, jugador) y las operaciones de colocacion y dibujo. No pide datos
 * por teclado ni decide el flujo de la partida: eso es ui_cli.c / app.c.
 *
 * @note Convencion de coordenadas: `fila` y `columna` son indices
 *       0..CONFIG_FILAS-1 y 0..CONFIG_COLUMNAS-1. La interfaz muestra filas
 *       1..10 y columnas A..J; la traduccion se hace en la capa de interfaz.
 * @note Convencion de celdas: '~' agua, '#' barco intacto, 'X' impacto y
 *       '/' disparo al agua. El enum Estado usa exactamente esos caracteres
 *       para que imprimir una celda sea simplemente printf("%c", estado).
 * @note Estandar: C99 (-std=c99 -pedantic -Wall -Wextra). Los tamanios de
 *       tablero y flota salen de config.h; no se repiten literales 10 ni 5.
 */

#ifndef BATALLA_NAVAL_TABLERO_H
#define BATALLA_NAVAL_TABLERO_H

#include "config.h"

/**
 * @brief Estado de cada celda del tablero.
 * @note Los valores coinciden con los caracteres que se imprimen en consola.
 */
typedef enum {
    agua = '~',             /**< Casilla sin barco ni disparo. */
    barco = '#',            /**< Parte de un barco intacta. */
    impacto = 'X',          /**< Disparo que acerto a un barco. */
    disparo_a_agua = '/'    /**< Disparo que cayo al agua. */
} Estado;

/**
 * @brief Orientacion con la que se coloca un barco.
 */
typedef enum {
    horizontal = 0,         /**< Crece hacia la derecha (columnas). */
    vertical = 1            /**< Crece hacia abajo (filas). */
} Orientacion;

/**
 * @brief Tipo y longitud de cada barco.
 * @note Los valores salen de config.h para que cambiar la flota no obligue
 *       a tocar la logica. Cada enumerador representa la longitud del barco.
 */
typedef enum {
    porta_aviones = CONFIG_BARCO_LEN_0, /**< Longitud 5 por defecto. */
    buque = CONFIG_BARCO_LEN_1,         /**< Longitud 4 por defecto. */
    submarino = CONFIG_BARCO_LEN_2,     /**< Longitud 3 por defecto. */
    lancha = CONFIG_BARCO_LEN_3,        /**< Longitud 2 por defecto. */
    velero = CONFIG_BARCO_LEN_4         /**< Longitud 1 por defecto. */
} Tamanio;

/**
 * @brief Datos de un barco ya colocado (o que se quiere colocar).
 */
typedef struct {
    Tamanio tamanio;         /**< Longitud en casillas (1..5 por defecto). */
    Orientacion orientacion; /**< horizontal o vertical. */
    int fila;                /**< Fila de la proa (0..CONFIG_FILAS-1). */
    int columna;             /**< Columna de la proa (0..CONFIG_COLUMNAS-1). */
} Barco;

/**
 * @brief Datos de un jugador dentro de una partida.
 */
typedef struct {
    char nombre[CONFIG_NOMBRE_MAX]; /**< Nombre terminado en '\0'. */
    Barco barco[CONFIG_NUM_BARCOS]; /**< Flota completa del jugador. */
} Jugador;

/**
 * @brief Llena los dos tableros con agua ('~').
 *
 * @param posicion Tablero propio (barcos), de CONFIG_FILAS x CONFIG_COLUMNAS.
 * @param posicion_disparos Mapa de disparos contra el rival, del mismo tamano.
 * @return No devuelve valor.
 * @note Si cualquiera de los dos punteros es NULL, la funcion no modifica
 *       nada y retorna sin error.
 */
void inicializar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                         char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS]);

/**
 * @brief Verifica que un barco entre completo dentro del tablero.
 *
 * Comprueba que la casilla inicial exista, que la orientacion sea valida y
 * que la longitud no haga que el barco se salga por la derecha o por abajo.
 *
 * @param filas Fila de la proa (0..CONFIG_FILAS-1).
 * @param columnas Columna de la proa (0..CONFIG_COLUMNAS-1).
 * @param orientacion horizontal o vertical.
 * @param tamanio Longitud del barco.
 * @return 1 si la colocacion entra completa; 0 si no.
 * @note No recibe el tablero porque solo depende de los limites. Si en el
 *       futuro hay que validar terreno, se agrega un parametro nuevo y se
 *       actualizan las llamadas.
 */
int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio);

/**
 * @brief Verifica que el barco quepa y que el lugar este libre.
 *
 * Primero llama a rango() y luego recorre el cuerpo del barco y su anillo
 * exterior. La regla actual exige al menos una casilla de agua alrededor,
 * incluidas las diagonales.
 *
 * @param filas Fila de la proa.
 * @param columnas Columna de la proa.
 * @param orientacion horizontal o vertical.
 * @param tamanio Longitud del barco.
 * @param posicion Tablero propio que se quiere consultar.
 * @return 1 si el lugar esta libre; 0 si se superpone, toca otro barco o
 *         la colocacion no entra en el tablero.
 * @note No modifica `posicion`. El parametro no lleva const porque C99 no
 *       permite convertir implicitamente char[][N] a const char[][N] sin
 *       -Wpedantic.
 */
int libre(int filas, int columnas, Orientacion orientacion, Tamanio tamanio,
          char posicion[CONFIG_FILAS][CONFIG_COLUMNAS]);

/**
 * @brief Marca con '#' las casillas ocupadas por un barco.
 *
 * @param filas Fila de la proa.
 * @param columnas Columna de la proa.
 * @param orientacion horizontal o vertical.
 * @param tamanio Longitud del barco.
 * @param posicion Tablero propio que se modifica.
 * @return No devuelve valor.
 * @note Presupone que rango() y libre() ya fueron llamados y dieron un
 *       resultado valido; si no, no modifica nada.
 */
void ingresar_barco(int filas, int columnas, Orientacion orientacion,
                    Tamanio tamanio, char posicion[CONFIG_FILAS][CONFIG_COLUMNAS]);

/**
 * @brief Imprime los dos tableros en la consola.
 *
 * Muestra las filas 1..10 y las columnas A..J. El primer tablero es la flota
 * propia y el segundo el mapa de disparos contra el rival.
 *
 * @param posicion Tablero propio (flota).
 * @param posicion_disparos Mapa de disparos.
 * @return No devuelve valor.
 * @note No modifica las matrices. Si algun puntero es NULL, imprime un aviso
 *       en lugar de recorrer memoria invalida.
 */
void mostrar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                     char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS]);

#endif /* BATALLA_NAVAL_TABLERO_H */
