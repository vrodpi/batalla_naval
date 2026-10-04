#ifndef UI_CLI_H
#define UI_CLI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// prototipos de funciones

void esperar_usuario(void);
// limpia la consola de Windows y Linux
void limpiar_pantalla_cli(void);
// pide un entero con el número del jugador y devuelve un char * con el nombre
char *pedir_nombre_jugador(int numero_de_jugador);
// saluda al jugador indicado
void saludar_jugador(char *nombre_jugador);

void ingresar_jugador(void);
// recibe la ruta a un archivo de texto y lo muestra por pantalla
void mostrar_archivo_de_texto(const char *ruta_al_archivo);

void mostrar_presentacion_y_reglamento(void);
#endif