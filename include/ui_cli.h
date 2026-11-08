#ifndef UI_CLI_H
#define UI_CLI_H

#include "game.h"


// prototipos de funciones

void limpiar_pantalla_cli(void);

void mostrar_presentacion_y_reglamento(char jugador[], char ruta_al_archivo[]);
void pedir_nombre_jugador(char jugador[]);
void imprimir_linea(void);
// devuelve 1 si se eligió un modo de juego válido y 0 en caso contrario
int elegir_modo_de_juego(Partida *partida);
void pedir_posicion_del_barco(Jugador *jugador, int tamanio);
void mostrar_tableros(Jugador jugador);

void mostrar_pantalla_de_colocacion_de_barcos(Jugador jugador, int tamanio);
#endif