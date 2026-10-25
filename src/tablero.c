/*
 * tablero.c - Reglas de tablero y colocacion de barcos.
 *
 * ---------------------------------------------------------------------------
 * RESPONSABILIDAD
 * ---------------------------------------------------------------------------
 * Mantiene el estado de las casillas y decide si un barco puede colocarse.
 * No pide datos por teclado ni muestra menus: eso es ui_cli.c. No decide
 * turnos ni victoria: eso sera game.c.
 *
 * Convencion interna:
 *   - filas y columnas se cuentan desde 0;
 *   - el tablero propio guarda '~' (agua) y '#' (barco);
 *   - el tablero de disparos guarda '~' (sin disparar), '/' (agua) y
 *     'X' (impacto). Los caracteres salen del enum Estado.
 *
 * ---------------------------------------------------------------------------
 * REGLA DE COLOCACION
 * ---------------------------------------------------------------------------
 * Un barco es valido si:
 *   1. su Casilla inicial esta dentro del tablero;
 *   2. con la orientacion elegida entra completo (rango);
 *   3. todas sus casillas y las de alrededor estan en agua (libre).
 *
 * El punto 3 implementa la regla "dejar un espacio entre barcos" que
 * figuraba en el codigo original. Si el equipo decide que los barcos
 * pueden tocarse, basta con reducir el rectangulo de libre() al cuerpo
 * del barco (quitar el -1/+1 de los extremos).
 *
 * Complejidad: rango es O(1); libre recorre a lo sumo un rectangulo de
 * (largo+2) x 3 casillas, O(largo), con largo <= 5 por defecto.
 *
 * ---------------------------------------------------------------------------
 * ESTANDAR
 * ---------------------------------------------------------------------------
 * C99 (-std=c99 -pedantic -Wall -Wextra). Este modulo se compila con el
 * estandar unico del proyecto. Los tamanios salen de config.h.
 */

#include <stdio.h>

#include "tablero.h"

void inicializar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                         char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int i;
    int j;

    if (posicion == NULL || posicion_disparos == NULL) {
        return;
    }
    for (i = 0; i < CONFIG_FILAS; i++) {
        for (j = 0; j < CONFIG_COLUMNAS; j++) {
            posicion[i][j] = (char)agua;
            posicion_disparos[i][j] = (char)agua;
        }
    }
}

int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio)
{
    int largo;

    if (orientacion != horizontal && orientacion != vertical) {
        return 0;
    }
    if (filas < 0 || filas >= CONFIG_FILAS) {
        return 0;
    }
    if (columnas < 0 || columnas >= CONFIG_COLUMNAS) {
        return 0;
    }

    largo = (int)tamanio;
    if (largo <= 0) {
        return 0;
    }

    /*
     * Si empieza en la columna c y mide `largo`, la ultima casilla es
     * c + largo - 1. Para que no se salga: c + largo <= CONFIG_COLUMNAS.
     * Lo mismo para las filas en vertical.
     */
    if (orientacion == horizontal) {
        return (columnas + largo <= CONFIG_COLUMNAS);
    }
    return (filas + largo <= CONFIG_FILAS);
}

int libre(int filas, int columnas, Orientacion orientacion, Tamanio tamanio,
          char posicion[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int largo;
    int f;
    int c;
    int fila_ini;
    int fila_fin;
    int col_ini;
    int col_fin;

    if (posicion == NULL) {
        return 0;
    }
    if (!rango(filas, columnas, orientacion, tamanio)) {
        return 0;
    }

    largo = (int)tamanio;
    if (orientacion == horizontal) {
        fila_ini = filas;
        fila_fin = filas;
        col_ini = columnas;
        col_fin = columnas + largo - 1;
    } else {
        fila_ini = filas;
        fila_fin = filas + largo - 1;
        col_ini = columnas;
        col_fin = columnas;
    }

    /*
     * Recorrer el rectangulo expandido un casillero en todas las
     * direcciones. Incluye el cuerpo del barco (debe ser agua) y el anillo
     * exterior (evita que dos barcos se toquen, incluso en diagonal).
     * Las casillas fuera del tablero simplemente se ignoran.
     */
    for (f = fila_ini - 1; f <= fila_fin + 1; f++) {
        for (c = col_ini - 1; c <= col_fin + 1; c++) {
            if (f < 0 || f >= CONFIG_FILAS || c < 0 || c >= CONFIG_COLUMNAS) {
                continue;
            }
            if (posicion[f][c] != (char)agua) {
                return 0;
            }
        }
    }
    return 1;
}

void ingresar_barco(int filas, int columnas, Orientacion orientacion,
                    Tamanio tamanio, char posicion[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int i;
    int largo;

    if (posicion == NULL) {
        return;
    }
    /*
     * Validacion defensiva: el llamador deberia haber llamado a rango() y
     * libre(). Si no lo hizo, no se escribe fuera del tablero.
     */
    if (!rango(filas, columnas, orientacion, tamanio)) {
        return;
    }

    largo = (int)tamanio;
    for (i = 0; i < largo; i++) {
        if (orientacion == horizontal) {
            posicion[filas][columnas + i] = (char)barco;
        } else {
            posicion[filas + i][columnas] = (char)barco;
        }
    }
}

void mostrar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                     char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int i;
    int k;

    if (posicion == NULL || posicion_disparos == NULL) {
        printf("Tablero no inicializado.\n");
        return;
    }

    printf("\n--- MI FLOTA ---\n   ");
    for (k = 0; k < CONFIG_COLUMNAS; k++) {
        printf("%c ", 'A' + k);
    }
    printf("\n");
    for (i = 0; i < CONFIG_FILAS; i++) {
        printf("%2d ", i + 1);
        for (k = 0; k < CONFIG_COLUMNAS; k++) {
            printf("%c ", posicion[i][k]);
        }
        printf("\n");
    }

    printf("\n--- DISPAROS A FLOTA ENEMIGA ---\n   ");
    for (k = 0; k < CONFIG_COLUMNAS; k++) {
        printf("%c ", 'A' + k);
    }
    printf("\n");
    for (i = 0; i < CONFIG_FILAS; i++) {
        printf("%2d ", i + 1);
        for (k = 0; k < CONFIG_COLUMNAS; k++) {
            printf("%c ", posicion_disparos[i][k]);
        }
        printf("\n");
    }
}
