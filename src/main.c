
#include "../include/ui_cli.h"
#include "../include/game.h"
#include "../include/tablero.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void colocar_barcos(Jugador *jugador);
void partida_pvp(Partida *partida);
void partida_pvia(Partida *partida);
void partida_pvp_t(Partida *partida);
void partida_pvia_t(Partida *partida);

// arreglo de punteros a funciones para Modo y Dificultad
void (*ejecutar_modo[4])(Partida *) ={
    partida_pvp,                                // posición 0
    partida_pvp_t,                              // posición 1
    partida_pvia,                               // posición 2
    partida_pvia_t                              // posición 3
};


int main(void){

    Partida partida;
    srand(time(NULL)); // para modo tormenta

    int opcion_menu;

    mostrar_presentacion_y_reglamento(partida.jugador1.nombre, "files/Reglamento.txt");

    
    inicializar_partida(&partida);
    
    opcion_menu = menu_principal(&partida);
        
        if (opcion_menu != 0) {   // para evitar que se ejecute una partida, si el jugador prefirió salir

        // Para saber en que posición del puntero a función ejecutar_modo[]()
        // En el Enum, PVP es 0 (y Dificultad Normal 0) entonces Posicion 0; 
        // PVP es 0 (Y Dificultad Tormenta es 1) entonces 0*2 + 1 = 1  Posicion 1;
        // PVIA es 1 (Dificultad Normal 0) entonces posición 2 y PVIA Tormenta posición 3
        int indice = partida.modo_de_juego * 2 + partida.dificultad;      

        // Implementar la función que corresponde
        ejecutar_modo[indice](&partida);
    }

 

    return 0;
}

void colocar_barcos(Jugador *jugador){

    int indice = 4;

    while (indice >= 0){

        mostrar_tableros(*jugador, 0);

        pedir_posicion_del_barco(jugador, indice);

        // si la posición es válida, lo asigna y continúa con el siguiente barco
        if (validar_ingreso_barco(jugador->barco[indice], jugador->tableros.tablero) == 1){
            indice--;
        }
        else {
            imprimir_posicion_invalida();
        }

        mostrar_tableros(*jugador, 0);
    }

    // le da tiempo al jugador a ver su tablero después de poner el último barco
    if (indice == -1){
        esperar_usuario();
    }
}

void ejecutar_turno_del_jugador(Jugador *atacante, Jugador *defensor, int turno){
    
    int fila_ataque, columna_ataque;

    Rdo_Disparo disparo;

    // pantalla cambio de turno
    mostrar_pantalla_cambio_de_turno(*atacante, turno);

    mostrar_tableros(*atacante, turno);

    do {
        pedir_coordenadas_de_ataque(&fila_ataque, &columna_ataque);

        // determina el resultado del ataque
        disparo = recibir_disparo(fila_ataque, columna_ataque, defensor->tableros.tablero);

        // aplica el cambio en los tableros de ambos jugadores
        // si es disp_repetido, no hace nada
        aplicar_disparo(atacante, defensor, fila_ataque, columna_ataque, disparo);
  
        // actualiza el tablero en pantalla
        mostrar_tableros(*atacante, turno);

        // avisa al usuario el resultado de su ataque
        resultado_del_ataque(disparo);

    // si el disparo es repetido, vuelve a pedir coords de ataque
    } while (disparo == disp_repetido);
}

// Partdida entre Personas. Dificultad Normal.
void partida_pvp(Partida *partida){
    
    pedir_nombre_jugador(partida->jugador2.nombre);
    
    colocar_barcos(&partida->jugador1);

    mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->cantidad_turnos);

    colocar_barcos(&partida->jugador2);

    partida->cantidad_turnos++;

    while (comprobar_partida_terminada(partida) == 0){

        if (partida->turno == PJ1){ //1
            
            ejecutar_turno_del_jugador(&partida->jugador1, &partida->jugador2, partida->cantidad_turnos);
        
        }
        else if (partida->turno == PJ2){  //2

            ejecutar_turno_del_jugador(&partida->jugador2, &partida->jugador1, partida->cantidad_turnos);

        }

        cambiar_turno(partida);
    }

    mostrar_pantalla_de_victoria(partida);

    // actualizar y mostrar ranking
}

void partida_pvia(Partida *partida){
    
    strcpy(partida->jugador2.nombre, "IA");
    
    colocar_barcos(&partida->jugador1);

    mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->turno);

    /*
    cosas...
    */

}


// Partida entre Personas, con dificultad TORMENTA
void partida_pvp_t(Partida *partida){

    pedir_nombre_jugador(partida->jugador2.nombre);

    colocar_barcos(&partida->jugador1);

    mostrar_pantalla_cambio_de_turno(partida->jugador2,
                                     partida->cantidad_turnos);

    colocar_barcos(&partida->jugador2);

    while (comprobar_partida_terminada(partida) == 0){

        if (partida->turno == 1){   //pj1 

            ejecutar_turno_del_jugador(&partida->jugador1,
                                       &partida->jugador2,
                                       partida->cantidad_turnos);

        }

        else if (partida->turno == 2){   // pj2

            ejecutar_turno_del_jugador(&partida->jugador2,
                                       &partida->jugador1,
                                       partida->cantidad_turnos);


        }
        // La tormenta afecta al jugador 2
        modo_tormenta(partida);

        cambiar_turno(partida);
    }

    mostrar_pantalla_de_victoria(partida);
}

// es para borrar para probar si compilada
void partida_pvia_t(Partida *partida) {
    pedir_nombre_jugador(partida->jugador2.nombre);
}