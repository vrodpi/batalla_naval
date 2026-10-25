#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include "tablero.h"


enum Modo { 
    PvP_N, // N normal
    PvIA_N, // Persona vs IA (normal)
    PvP_T, // Persona vs persona (tormenta)
    PvIA_T, // T tormenta
};

typedef struct Jugador {
    Tablero tableros;
    char Nombres[50];
    int Barcos_vivos; 
    int Hits;  //(ranking)
}Jugador;

typedef struct Partida {
    Jugador jugador1;
    Jugador jugador2;
    int Turno;
    int Victoria;
    int Cantidad_turnos;
    enum Modo modo_de_juego;
    int Ganador;
}Partida;

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
#endif