#include <stdio.h>
#include <string.h>
#include "../include/ui_cli.h"
#include "../include/game.h"

int main(void){

    Partida partida;

    mostrar_presentacion_y_reglamento(partida.jugador1.nombre, "files/Reglamento.txt");

    //inicializar_partida(&partida);

    elegir_modo_de_juego(&partida);

    switch (partida.modo_de_juego){
        case PVP: 
            printf("JvJ\n");
            pedir_nombre_jugador(partida.jugador2.nombre);

            imprimir_linea();

            printf("Estas jugando contra %s\n", partida.jugador2.nombre);

            break;
        case PVIA:
            printf("JvIA\n");
            strcpy(partida.jugador2.nombre, "IA");

            imprimir_linea();

            printf("Estas jugando contra %s\n", partida.jugador2.nombre);
            break;
    }
    switch (partida.dificultad){
        case NORMAL:
            printf("Dificultad: Normal\n");
            break;
        case TORMENTA:
            printf("Dificultad: Tormenta\n");
            break;
    }

    for (int i = 5; i > 0; i--){

        mostrar_pantalla_de_colocacion_de_barcos(partida.jugador1, i);

        // validar posicion e ingresar al tablero
    }

    return 0;
}