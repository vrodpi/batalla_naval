#ifndef UI_CLI_H
#define UI_CLI_H

#include "game.h"
#include "ranking.h"
#include "config.h"

typedef enum {
    UI_MENU_OPCION_PVP = 1,
    UI_MENU_OPCION_PVIA = 2,
    UI_MENU_MOSTRAR_RANKING = 3,
    UI_MENU_MOSTRAR_REGLAMENTO = 4,
    UI_MENU_SALIR = 0
} UI_Menu_principal;

typedef enum {
    UI_RANKING_COMPLETO = 1,
    UI_RANKING_REGISTRO = 2,
    UI_RANKING_SALIR = 0
} UI_Menu_ranking;

typedef enum {
    UI_MENSAJE_FLOTA,
    UI_MENSAJE_DISPARO
} UI_Mensajes_Error_IA;


int pedir_opcion_al_usuario(const char *texto, int *opcion);

// pide el nombre del jugador 1, le da la bienvenida y muestra el reglamento
void ui_mostrar_presentacion_y_reglamento(char jugador[], char ruta_al_archivo[]);
void pedir_nombre_jugador(char jugador[], const int numero);
void ui_mostrar_ranking(const Ranking ranking);
void ui_mensaje_disparo_ia(const char *nombre, const int fila, const int columna);
void ui_mensaje_error_ia(UI_Mensajes_Error_IA mensaje);

// menu principal del juego, permite elegir la partida, dificultad, ver el ránking y el reglamento
// devuelve 1 si se eligió un modo de juego válido y 0 en caso contrario
void mostrar_menu_principal(void);
int elegir_dificultad(Dificultad *dificultad);
void ui_mostrar_reglamento(const char *ruta_al_archivo);

// muestra los tableros de barcos y disparos del jugador indicado
void ui_mostrar_tableros(const Jugador jugador, const int turno, const int tipo_de_partida);

// mensaje de error
void ui_confirmacion_barco_colocado(int caso);

// muestra el tablero y solicita la colocación del barco indicado con tamanio
void pedir_posicion_del_barco(Jugador *jugador, int tamanio);
void mostrar_pantalla_cambio_de_turno(const Jugador jugador, const int turno);

void mostrar_pantalla_de_victoria(Partida *partida);
void pedir_coordenadas_de_ataque(int *fila, int *columna);
void resultado_del_ataque(Rdo_Disparo disparo);
void mostrar_mensaje_tormenta (Jugador *defensor, int Fil_afectada, int Col_afectada);

#endif