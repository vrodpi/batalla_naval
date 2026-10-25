#ifndef BATALLA_NAVAL_TABLERO_H
#define BATALLA_NAVAL_TABLERO_H

/*
 * tablero.h - Modelo del tablero y de la flota.
 *
 * ---------------------------------------------------------------------------
 * RESPONSABILIDAD
 * ---------------------------------------------------------------------------
 * Define los tipos de datos del tablero (celdas, orientacion, tamanio de
 * barco, jugador) y las operaciones de colocacion y dibujo. NO pide datos
 * por teclado ni decide el flujo de la partida: eso es ui_cli.c / app.c.
 *
 * Convencion de coordenadas:
 *   - `fila` y `columna` son indices 0..CONFIG_FILAS-1 / 0..CONFIG_COLUMNAS-1.
 *   - La interfaz muestra filas 1..10 y columnas A..J; la traduccion se
 *     hace en la capa de interfaz, no aqui.
 *
 * Convencion de celdas:
 *   '~' agua, '#' barco intacto, 'X' impacto, '/' disparo al agua.
 *   El enum Estado usa exactamente esos caracteres para que imprimir una
 *   celda sea simplemente printf("%c", (char)estado).
 *
 * ---------------------------------------------------------------------------
 * ESTANDAR
 * ---------------------------------------------------------------------------
 * C99 (-std=c99 -pedantic -Wall -Wextra). Los tamanios de tablero y flota
 * salen de config.h; no se repiten literales 10 ni 5 en este archivo.
 */

#include "config.h"

/* Estado de cada celda del tablero. */
typedef enum {
    agua = '~',             /* casilla sin barco ni disparo */
    barco = '#',            /* parte de un barco intacta */
    impacto = 'X',          /* disparo que acerto a un barco */
    disparo_a_agua = '/'    /* disparo que cayo al agua */
} Estado;

/* Orientacion con la que se coloca un barco. */
typedef enum {
    horizontal = 0,         /* crece hacia la derecha (columnas) */
    vertical = 1            /* crece hacia abajo (filas) */
} Orientacion;

/*
 * Tipo y longitud de cada barco. Los valores salen de config.h para que
 * cambiar la flota no obligue a tocar la logica.
 */
typedef enum {
    porta_aviones = CONFIG_BARCO_LEN_0,
    buque = CONFIG_BARCO_LEN_1,
    submarino = CONFIG_BARCO_LEN_2,
    lancha = CONFIG_BARCO_LEN_3,
    velero = CONFIG_BARCO_LEN_4
} Tamanio;

/* Datos de un barco ya colocado (o que se quiere colocar). */
typedef struct {
    Tamanio tamanio;        /* longitud en casillas (1..5 por defecto) */
    Orientacion orientacion;/* horizontal o vertical */
    int fila;               /* fila de la proa (0..CONFIG_FILAS-1) */
    int columna;            /* columna de la proa (0..CONFIG_COLUMNAS-1) */
} Barco;

/* Datos de un jugador dentro de una partida. */
typedef struct {
    char nombre[CONFIG_NOMBRE_MAX];
    Barco barco[CONFIG_NUM_BARCOS];
} Jugador;

/*
 * Llena los dos tableros con agua ('~').
 * `posicion` es el tablero propio (barcos) y `posicion_disparos` el mapa
 * de disparos contra el rival. Ambos miden CONFIG_FILAS x CONFIG_COLUMNAS.
 */
void inicializar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                         char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS]);

/*
 * Verifica que un barco de longitud `tamanio` colocado en (filas, columnas)
 * con la orientacion indicada entre completo dentro del tablero.
 *
 * Devuelve 1 si es valido, 0 si no. No recibe el tablero porque solo
 * depende de los limites; si en el futuro hay que validar terreno, se
 * agrega un parametro nuevo y se actualizan las llamadas.
 */
int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio);

/*
 * Verifica que el barco quepa (mediante rango) y que no se superponga ni
 * toque a otro barco. La regla actual exige al menos una casilla de agua
 * alrededor del barco, incluidos los laterales y las diagonales.
 *
 * No modifica `posicion` (el parametro no lleva const porque C99 no permite
 * convertir implicitamente char[][N] a const char[][N] sin -Wpedantic).
 *
 * Devuelve 1 si el lugar esta libre, 0 si no.
 */
int libre(int filas, int columnas, Orientacion orientacion, Tamanio tamanio,
          char posicion[CONFIG_FILAS][CONFIG_COLUMNAS]);

/*
 * Marca con '#' las casillas del barco. Presupone que rango() y libre()
 * ya fueron llamados y dieron resultado valido; si no, no modifica nada.
 */
void ingresar_barco(int filas, int columnas, Orientacion orientacion,
                    Tamanio tamanio, char posicion[CONFIG_FILAS][CONFIG_COLUMNAS]);

/*
 * Imprime los dos tableros con filas 1..10 y columnas A..J.
 * `posicion` es la flota propia y `posicion_disparos` el mapa de disparos.
 * No modifica las matrices (ver nota sobre const en libre()).
 */
void mostrar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                     char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS]);

#endif
