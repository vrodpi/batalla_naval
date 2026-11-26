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
    UI_MENU_SALIR = 0,
    UI_MENU_DEFAULT = -1
} UI_Menu_principal;

typedef enum {
    UI_MENSAJE_SALIR = 0,
    UI_MENSAJE_DEFAULT = -1
} UI_Mensaje_menu;

typedef enum {
    UI_RANKING_COMPLETO = 1,
    UI_RANKING_REGISTRO = 2,
    UI_RANKING_SALIR = 0
} UI_Menu_ranking;

typedef enum {
    UI_MENSAJE_FLOTA,
    UI_MENSAJE_DISPARO
} UI_Mensajes_Error_IA;

// Prototipos
int ui_pedir_opcion_al_usuario(const char *texto, int *opcion);
void ui_pedir_nombre_jugador(char jugador[], const int numero);
void ui_mostrar_reglamento(const char *ruta_al_archivo);
void ui_nombre_juego(void);
void ui_mostrar_presentacion_y_reglamento(char jugador[], const char ruta_al_archivo[]);
void ui_mostrar_menu_principal(void);
void ui_mensajes_menu(const UI_Mensaje_menu opcion);
int ui_elegir_dificultad(Dificultad *dificultad);
void ui_pedir_posicion_del_barco(Jugador *jugador, const int tamanio);
void ui_confirmacion_barco_colocado(const int caso);
void ui_pedir_coordenadas_de_ataque(int *fila, int *columna);
void ui_resultado_del_ataque(const Rdo_Disparo disparo);
void ui_mostrar_tableros(const Jugador jugador, const int turno, const int tipo_de_partida);
void ui_mostrar_pantalla_cambio_de_turno(const Jugador jugador, const int turno);
void ui_mostrar_pantalla_de_victoria(Partida *partida);
void ui_mostrar_mensaje_tormenta(Jugador *defensor, const int Fil_afectada, const int Col_afectada);
void ui_mensaje_disparo_ia(const char *nombre, const int fila, const int columna);
void ui_mensaje_error_ia(const UI_Mensajes_Error_IA mensaje);
void ui_mostrar_ranking(const Ranking ranking);

#endif