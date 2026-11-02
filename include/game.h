#ifndef GAME_H
#define GAME_H

#include "tablero.h"

typedef enum { 
    PVP, // N normal
    PVIA // Persona vs IA (normal)
} Modo_de_juego;

typedef enum {
    NORMAL,
    TORMENTA
} Dificultad;

typedef enum {
    PJ1,
    PJ2
} Turno;

typedef struct {
    Tablero tableros;
    char nombre[50];
    int barcos_vivos; 
    int hits;  //(ranking)
} Jugador;

typedef struct {
    Jugador jugador1;
    Jugador jugador2;
    Turno turno;
    int cantidad_turnos;
    Modo_de_juego modo_de_juego;
    Dificultad dificultad;
    int victoria;
    int ganador;
} Partida;

#endif