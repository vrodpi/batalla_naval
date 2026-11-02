#ifndef UI_CLI_H
#define UI_CLI_H

#include "game.h"


// prototipos de funciones

void esperar_usuario(void);
// limpia la consola de Windows y Linux
void limpiar_pantalla_cli(void);
int pedir_opcion_al_usuario(void);
void pedir_nombre_jugador(char jugador[]);
void saludar_jugador(char jugador[]);
void mostrar_archivo_de_texto(const char *ruta_al_archivo);
void mostrar_presentacion_y_reglamento(char jugador[], char ruta_al_archivo[]);
void mostrar_menu_principal(void);
void mostrar_menu_de_dificultad(void);
void imprimir_linea(void);

// devuelve 1 si se eligió un modo de juego válido y 0 en caso contrario
int elegir_modo_de_juego(Modo_de_juego *modo, Dificultad *dificultad);

#endif