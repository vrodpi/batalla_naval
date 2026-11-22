#ifndef GAME_H
#define GAME_H

#define BARCOS_MODO_NORMAL 5

#include "tablero.h"

typedef enum { 
    PVP,    // Entre Personas // 0
    PVIA    // Persona vs IA (normal) // 1
} Modo_de_juego;

typedef enum {
    NORMAL,     //0
    TORMENTA   //1
} Dificultad;

typedef enum {
    PJ1,        
    PJ2      
} Turno;

typedef struct  {
    Tableros tableros;
    char nombre[50];
    Barco barco [BARCOS_MODO_NORMAL];
    int barcos_vivos;              //  comienza en 5, contador. barcos aun en juego.
    int hits;                     //( impactos acertados; ranking)
    int impacto_agua_consecutivo;   // para que haya un registro independiente entre los jugadores
} Jugador;

typedef struct {
    Jugador jugador1;
    Jugador jugador2;
    Turno turno;
    int cantidad_turnos;
    Modo_de_juego modo_de_juego;
    Dificultad dificultad;
    int ganador;
} Partida;


void inicializar_partida( Partida *partida);
int comprobar_partida_terminada(Partida *partida);
void cambiar_turno(Partida *partida);
void aplicar_disparo(Jugador *atacante, Jugador *defensor, int fila, int columna, Rdo_Disparo disparo);
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]);
void modo_tormenta(Partida *partida); 

#endif