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
    char tablero_barcos[FIL][COL];  // Tablero con mis barcos.
    char tablero_disparos[FIL][COL];   // Tablero de disparos al oponente.
} Tableros;

typedef struct {
    char nombre[20];
    Barco barco[BARCOS_MODO_NORMAL];
} Jugador;

void inicializar_tablero(char tablero[FIL][COL], char tablero_disparos[FIL][COL]);
int rango(Barco barcos, char posicion[FIL][COL]);
int libre(Barco barcos, char posicion[FIL][COL]);
void ingresar_barco(Barco barcos, char tablero[FIL][COL]);
void mostrar_tablero(char tablero[FIL][COL], char tablero_disparos[FIL][COL]);

#endif // TABLERO_H