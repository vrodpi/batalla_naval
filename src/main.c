
#include "../include/ui_cli.h"
#include "../include/game.h"
#include "../include/tablero.h"
#include "../include/ai.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void colocar_barcos(Jugador *jugador);
void partida_pvp(Partida *partida);
void partida_pvia(Partida *partida);
void partida_pvp_t(Partida *partida);
void partida_pvia_t(Partida *partida);

static int ejecutar_turno_ia(Partida *partida, EstadoIA *estado);
static void jugar_pvia(Partida *partida);

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

/*
 * Ejecuta un turno completo de la IA.
 * Devuelve 1 si pudo disparar y 0 si no quedaban casillas disponibles.
 */
static int ejecutar_turno_ia(Partida *partida, EstadoIA *estado){

    int fila, columna;
    Rdo_Disparo disparo;
    int barcos_antes, barcos_despues;

    mostrar_pantalla_cambio_de_turno(partida->jugador2,
                                     partida->cantidad_turnos);

    // Solo se muestra el tablero del humano; la flota de la IA queda oculta.
    mostrar_tableros(partida->jugador1, partida->cantidad_turnos);

    do {
        if (!ai_elegir_disparo(&partida->jugador2, estado,
                               &fila, &columna)) {
            return 0;
        }

        printf("La IA dispara en %c%d\n", 'A' + columna, fila + 1);

        disparo = recibir_disparo(fila, columna,
                                  partida->jugador1.tableros.tablero);

        barcos_antes = partida->jugador1.barcos_vivos;
        aplicar_disparo(&partida->jugador2, &partida->jugador1,
                        fila, columna, disparo);
        barcos_despues = partida->jugador1.barcos_vivos;

        ai_registrar_resultado(estado, &partida->jugador2, fila, columna,
                               disparo, barcos_antes, barcos_despues);

        mostrar_tableros(partida->jugador1, partida->cantidad_turnos);
        resultado_del_ataque(disparo);

    // Un disparo repetido no consume el turno: se vuelve a elegir.
    } while (disparo == disp_repetido);

    return 1;
}

/*
 * Flujo comun para Jugador vs IA, tanto en dificultad Normal como Tormenta.
 * modo_tormenta() no hace nada si la dificultad es NORMAL.
 */
static void jugar_pvia(Partida *partida){

    EstadoIA estado;

    ai_inicializar(&estado);

    strcpy(partida->jugador2.nombre, "IA");

    if (!ai_colocar_flota(&partida->jugador2)) {
        printf("No se pudo colocar la flota de la IA.\n");
        esperar_usuario();
        return;
    }

    colocar_barcos(&partida->jugador1);

    partida->turno = PJ1;
    partida->cantidad_turnos = 1;

    while (comprobar_partida_terminada(partida) == 0) {

        if (partida->turno == PJ1) {

            ejecutar_turno_del_jugador(&partida->jugador1,
                                       &partida->jugador2,
                                       partida->cantidad_turnos);

        } else if (partida->turno == PJ2) {

            if (!ejecutar_turno_ia(partida, &estado)) {
                printf("La IA no pudo elegir un disparo.\n");
                esperar_usuario();
                return;
            }
        }

        modo_tormenta(partida);
        cambiar_turno(partida);
    }

    mostrar_pantalla_de_victoria(partida);
}

void partida_pvia(Partida *partida){
    jugar_pvia(partida);
}


// Partida entre Personas, con dificultad TORMENTA
void partida_pvp_t(Partida *partida){

    pedir_nombre_jugador(partida->jugador2.nombre);

    colocar_barcos(&partida->jugador1);

    mostrar_pantalla_cambio_de_turno(partida->jugador2,
                                     partida->cantidad_turnos);

    colocar_barcos(&partida->jugador2);

    while (comprobar_partida_terminada(partida) == 0){

        if (partida->turno == PJ1){   

            ejecutar_turno_del_jugador(&partida->jugador1,
                                       &partida->jugador2,
                                       partida->cantidad_turnos);

        }

        else if (partida->turno == PJ2){   // pj2

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

// Jugador vs IA, con dificultad TORMENTA.
void partida_pvia_t(Partida *partida) {
    jugar_pvia(partida);
}