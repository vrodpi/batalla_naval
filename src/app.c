#include <stdio.h>
#include <string.h>
#include "../include/ui_cli.h"
#include "../include/game.h"

int main(void){

    Partida partida;

    mostrar_presentacion_y_reglamento(partida.jugador1.nombre, "files/Reglamento.txt");

    elegir_modo_de_juego(&partida.modo_de_juego, &partida.dificultad);

    switch (partida.modo_de_juego){
        case PVP: 
            printf("JvJ\n");
            pedir_nombre_jugador(partida.jugador2.nombre);
            break;
        case PVIA:
            printf("JvIA\n");
            strcpy(partida.jugador2.nombre, "IA");
            break;
    }
    switch (partida.dificultad){
        case NORMAL:
            printf("Normal\n");
            break;
        case TORMENTA:
            printf("Tormenta\n");
            break;
    }
    /*
    inicializar_partida(&partida, partida.modo_de_juego) ???
        - inicializar_partida() debería pedir el nombre del J2, gestionar turnos y disparos


    while (!partida_terminada(&partida)) {
        pedir acción a la UI y aplicarla mediante game.c
    }
    */
 
    return 0;
}