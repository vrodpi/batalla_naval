#ifndef GAME_H
#define GAME_H
#include "tablero.h"
#define BARCOS_MODO_NORMAL 5

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
    Jugador jugador1;
    Jugador jugador2;
    Turno turno;
    int cantidad_turnos;
    Modo_de_juego modo_de_juego;
    Dificultad dificultad;
    int victoria;
    int ganador;
} Partida;

typedef struct  {
    Tableros tableros;
    char nombres[50];
    Barco barco [BARCOS_MODO_NORMAL];
    int barcos_vivos;              //  comienza en 5, contador. barcos aun en juego.
    int hits;                     //( impactos acertados; ranking)
} Jugador;


void inicializar_partida( Partida *partida, Modo_de_juego modo);
void jugar(Partida *partida);
int fin_juego(Partida *partida);
void barco_hundido(Jugador *defensor);
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]);
void coordenadas_de_ataque(Jugador *atacante, Jugador *defensor);


#endif