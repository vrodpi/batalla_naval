#include <stdio.h>
#include <stdlib.h>
#include "../include/ui_cli.h"

#ifdef _WIN32 // funciones escritas para Windows

void limpiar_pantalla_cli(void) {
    system("cls");
}

#elif defined(__linux__) // funciones escritas para Linux

void limpiar_pantalla_cli(void) {
    system("clear");
}

#endif

void pedir_nombre_jugador(void){
    

}

void saludar_jugador(){}

void mostrar_reglamento(void) {} // usamos un archivo reglamento.txt
