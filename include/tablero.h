#ifndef TABLERO_H
#define TABLERO_H
#include <stdio.h>
#define FIL 10
#define COL 10

typedef enum {              // orientación del barco a ingresar
    horizontal,
    vertical
} Orientacion;

typedef enum {              // Tipo y tamaño del barco
    porta_aviones=5,
    buque=4,
    submarino=3,
    lancha=2,
    velero=1
} Tamanio;

typedef struct {
    Tamanio tamanio;
    Orientacion orientacion;
    int fila;
    int columna;
    int hundido;            // 0 aun está en juego, hay partes del barco sin descubrir  / 1 para indicar que está hundido
} Barco;

typedef struct {
    char tablero[FIL][COL];  // Tablero con mis barcos.
    char tablero_disparos[FIL][COL];   // Tablero de disparos al oponente.
} Tableros;

// posibles resultado en las coordenadas:
typedef enum {   // Los Disparos se relacionan con el funcionamiento del tablero.
    disp_NO_valido,
    disp_agua,
    disp_impacto,  // se encontró un barco, impacto.
    disp_repetido
} Rdo_Disparo;


void inicializar_tablero(char tablero[FIL][COL], char tablero_disparos[FIL][COL]);
int validar_rango(Barco barco);
int libre(Barco barco, char tablero[FIL][COL]);
void validar_ingreso_barco(Barco barco, char tablero[FIL][COL]);
void ingresar_barco(Barco barco, char tablero[FIL][COL]);
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]);


#endif