#ifndef TABLERO_H

#define TABLERO_H
#include <string.h>
 
#define FIL 10
#define COL 10
#define BARCOS_MODO_NORMAL 5

typedef enum {              // estado de cada celda
    agua,                   //    ~
    barco,                  //    #
    impacto,                //    X
    disparo_a_agua         //    /
} Estado;


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
} Barco;

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