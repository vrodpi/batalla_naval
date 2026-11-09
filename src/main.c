#include <string.h>
#include "../include/ui_cli.h"
#include "../include/game.h"
#include "../include/tablero.h"
#include <stdio.h>

void colocar_barcos(Jugador *jugador);
void partida_pvp(Partida *partida);
void partida_pvia(Partida *partida);

int main(void){

    Partida partida;

    mostrar_presentacion_y_reglamento(partida.jugador1.nombre, "files/Reglamento.txt");

    inicializar_partida(&partida);

    elegir_modo_de_juego(&partida);

    switch (partida.modo_de_juego){
        case PVP: 

            partida_pvp(&partida);

            break;
        
        case PVIA:
    
            partida_pvia(&partida);

            break;
    }


    return 0;
}

void colocar_barcos(Jugador *jugador){
    
    int contador = 5;

    while (contador > 0){

        mostrar_pantalla_de_colocacion_de_barcos(jugador, contador);

        if (validar_ingreso_barco(jugador->barco[contador], jugador->tableros.tablero) == 1){
            contador--;
            getchar();
        }
        else {
            imprimir_posicion_invalida();
        }
    }
}

void partida_pvp(Partida *partida){
    
    pedir_nombre_jugador(partida->jugador2.nombre);
    
    colocar_barcos(&partida->jugador1);

    mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->turno);

    colocar_barcos(&partida->jugador2);

    partida->cantidad_turnos++;

    while (comprobar_partida_terminada(partida) == 0){

        if (partida->turno == 1){
            mostrar_pantalla_cambio_de_turno(partida->jugador1, partida->cantidad_turnos);
            
            // pantalla ataque (jugador 1)
            // validar disparo
            // comprobar barcos hundidos
        }
        else if (partida->turno == 2){
            mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->cantidad_turnos);
            
            // pantalla ataque (jugador 2)

            partida->cantidad_turnos++;     // sólo aumenta después de que juege el PJ 2
        }

        cambiar_turno(partida);
    }

    mostrar_pantalla_de_victoria(partida);

    // mostrar ranking
}

void partida_pvia(Partida *partida){
    
    strcpy(partida->jugador2.nombre, "IA");
    
    colocar_barcos(&partida->jugador1);

    mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->turno);


}