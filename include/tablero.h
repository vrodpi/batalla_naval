#ifndef TABLERO_H

#define TABLERO_H
#include <string.h>
#include "game.h"
 
#define FIL 10
#define COL 10
#define BARCOS_MODO_NORMAL 5

typedef enum {              // estado de cada celda
    agua,                   //    ~
    barco,                  //    #
    impacto,                //    X
    disparo_a_agua         //    /
} Estado;


typedef struct {
    char posicion[FIL][COL];  // Tablero con mis barcos.
    char posicion_disparos[FIL][COL];   // Tablero de disparos al oponente.
} Tablero;

typedef struct {
    char nombre[20];
    Barco barco[BARCOS_MODO_NORMAL];
} Jugador;

void inicializar_tablero(char posicion[FIL][COL], char posicion_disparos[FIL][COL]);
int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]);
int libre(int filas, int columnas, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]);
void ingresar_barco(int filas, int columnas, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]);
void mostrar_tablero(char posicion[FIL][COL], char posicion_disparos[FIL][COL]);

#endif // TABLERO_H