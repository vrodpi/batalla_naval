#ifndef GAME_H   // si este archivo no esta definido
#define GAME_H   // se define con esta macro y se procesa el código // sino se saltea para evitar duplicados.

#include "config.h"
#include "tablero.h"   // incluir archivo de cabecera

typedef enum { 
    PVP,         // Partida entre Personas 
    PVIA        // Persona vs IA (normal)
} Modo_de_juego;

typedef enum {
    NORMAL,     
    TORMENTA   
} Dificultad;

typedef enum {
    TURNO_PJ1,   // turno del jugador 1
    TURNO_PJ2      
} Turno;

typedef struct  {
    Tableros tableros;
    char nombre[MAX_NOMBRE_DE_USUARIO];
    Barco barco [BARCOS_MODO_NORMAL];
    int barcos_vivos;              //  comienza en 5, contador. barcos aun en juego.
    int impacto_agua_consecutivo;   // para que haya un registro independiente entre los jugadores
} Jugador;

typedef struct {
    Jugador jugador1;
    Jugador jugador2;
    Turno turno;            // Jugador de Turno.
    int cantidad_turnos;   // n° de turno.
    Modo_de_juego modo_de_juego;        // Entre Personas o Vs IA
    Dificultad dificultad;
    int ganador;
} Partida;


void inicializar_partida( Partida *partida);
int comprobar_partida_terminada(Partida *partida);
void cambiar_turno(Partida *partida);
void aplicar_disparo(Jugador *atacante, Jugador *defensor, int fila, int columna, Rdo_Disparo disparo);
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]);
int modo_tormenta(Partida *partida, int *fila_afectada, int *columna_afectada); 

#endif  // fin de la macro.