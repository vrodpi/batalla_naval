#ifndef UI_CLI_H
#define UI_CLI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// prototipos de funciones

void esperar_usuario(void);
// limpia la consola de Windows y Linux
void limpiar_pantalla_cli(void);
void pedir_nombre_jugador(char jugador[]);
void saludar_jugador(char jugador[]);
void ingresar_y_saludar_jugador(char jugador[]);
void mostrar_archivo_de_texto(const char *ruta_al_archivo);
void mostrar_presentacion_y_reglamento(char jugador[]);
void mostrar_menu_principal(void);
void mostrar_menu_de_dificultad(void);
void imprimir_linea(void);
void elegir_modo_de_juego(void);

#endif