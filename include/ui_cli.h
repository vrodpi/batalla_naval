#ifndef UI_CLI_H
#define UI_CLI_H

#include "game.h"

void imprimir_linea(void);

void esperar_usuario(void);

// pide el nombre del jugador 1, le da la bienvenida y muestra el reglamento
void mostrar_presentacion_y_reglamento(char jugador[], char ruta_al_archivo[]);
void pedir_nombre_jugador(char jugador[]);

// menu principal del juego, permite elegir la partida, dificultad, ver el ránking y el reglamento
// devuelve 1 si se eligió un modo de juego válido y 0 en caso contrario
int menu_principal(Partida *partida);

// muestra los tableros de barcos y disparos del jugador indicado
void mostrar_tableros(Jugador jugador, int turno);

// mensaje de error
void imprimir_posicion_invalida(void);

// muestra el tablero y solicita la colocación del barco indicado con tamanio
void pedir_posicion_del_barco(Jugador *jugador, int tamanio);
void mostrar_pantalla_cambio_de_turno(const Jugador jugador, const int turno);

void mostrar_pantalla_de_victoria(Partida *partida);
void pedir_coordenadas_de_ataque(int *fila, int *columna);
void resultado_del_ataque(Rdo_Disparo disparo);
#endif