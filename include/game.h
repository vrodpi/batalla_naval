#ifndef GAME_H
#define GAME_H
#include "tablero.h"
#define BARCOS_MODO_NORMAL 5

#include <stdio.h>



typedef enum  { 
    PvP_N, // N normal
    PvIA_N, // Persona vs IA (normal)
    PvP_T, // Persona vs persona (tormenta)
    PvIA_T, // T tormenta
}Modo;

typedef struct  {
    Tableros tableros;
    char nombres[50];
    Barco barco [BARCOS_MODO_NORMAL];
    int barcos_vivos;              //  comienza en 5, contador. barcos aun en juego.
    int hits;                     //( impactos acertados; ranking)
} Jugador;

typedef struct {
    Jugador jugador1;               // usar strncpy para cargas los nombres
    Jugador jugador2;
    int turno;
    int victoria;
    int cantidad_turnos;
    Modo modo_de_juego;
    int ganador;
}Partida;


void inicializar_partida( Partida *partida, Modo modo);
void jugar(Partida *partida);
int fin_juego(Partida *partida);
void barco_hundido(Jugador *defensor);
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]);
void coordenadas_de_ataque(Jugador *atacante, Jugador *defensor);


#endif