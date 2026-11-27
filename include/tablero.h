#ifndef TABLERO_H
#define TABLERO_H

#include "config.h"

// Define la orientación del barco ingresado: parado (vertical) o acostado (horizontal)
typedef enum {              
    horizontal,
    vertical
} Orientacion;

// Guarda todos los datos de los Barcos en el juego:
// donde empieza (fila, columna), cuanto mide, como esta orientado, si ya esta hundido.
typedef struct {
    int tamanio;
    Orientacion orientacion;
    int fila;
    int columna;
    int hundido;            // 0 = sigue a flote; 1 = hundido por completo
} Barco;

// Agrupa los dos tableros de un jugador:
// uno para ver sus propios barcos, y otro para ir anotando los tiros hechos al rival.
typedef struct {
    char tablero[FIL][COL];  // Tablero con mis barcos.
    char tablero_disparos[FIL][COL];   // Tablero de disparos al oponente.
} Tableros;

// Los posibles resultado al disparar a una casilla (agua, pego, ya se habia tirado ahi o cayo una tormenta)
typedef enum {   
    disp_agua,
    disp_impacto,  
    disp_repetido,
    disp_tormenta
} Rdo_Disparo;

// Simbolos visuales que se guardan en cada casilla que representa el estado del tablero.
typedef enum {
    CASILLA_AGUA = '~',
    CASILLA_BARCO = '#',
    CASILLA_DISP_AGUA = 'O',
    CASILLA_IMPACTO = 'X',
    CASILLA_TORMENTA = '/'
} Casillas_tablero;

// Prepara ambos tableros vacios, al empezar la partida
void inicializar_tablero(char tablero[FIL][COL], char tablero_disparos[FIL][COL]);

// Revisa si las casillas donde queres poner los barcos estan libres
int libre(Barco barco, char tablero[FIL][COL]);

// Comprueba que el barco entre dentro del tablero y se pueda colocarse sin problemas
int validar_ingreso_barco(Barco barco, char tablero[FIL][COL]);

// Coloca efectivamente el barco en el tablero una vez que es valido.
void ingresar_barco(Barco barco, char tablero[FIL][COL]);

#endif