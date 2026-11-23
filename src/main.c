#include "../include/config.h"
#include "../include/ui_cli.h"
#include "../include/game.h"
#include "../include/tablero.h"
#include "../include/ranking.h"
#include "../include/ai.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void partida_pvp(Partida *partida);
//void partida_pvia(Partida *partida);


int main(void){

    Partida partida;
    Ranking ranking;
    int opcion_menu;
    
    ranking_inicializar(&ranking);

    ranking_cargar(&ranking, CONFIG_RANKING_RUTA);

    srand(time(NULL)); // para modo tormenta

    mostrar_presentacion_y_reglamento(partida.jugador1.nombre, CONFIG_REGLAMENTO_RUTA);

    do {
        inicializar_partida(&partida);
        
        mostrar_menu_principal();

        opcion_menu = pedir_opcion_al_usuario("Ingrese una opción: ");

        switch (opcion_menu){
            case 1:

                partida_pvp(&partida);
                break;

            case 2:

         //       partida_pvia(&partida);
                break;
            
            case 3:
                system("clear");
                // placeholder
                printf("Jugador | Puntos\n");
                imprimir_linea();
                printf("Vicen   | 1.500.123\n");

                esperar_usuario();
                break;

            case 4:
                
                mostrar_archivo_de_texto(CONFIG_REGLAMENTO_RUTA);
                
                esperar_usuario();
                break;

            case 0:
                printf("Saliendo...\n");
                return 0;
            
            default:
                printf("Opción inválida.\n");
                esperar_usuario();
                break;
        }
    } while (opcion_menu != 0);
    
/*    
    opcion_menu = menu_principal(&partida);
        
        if (opcion_menu != 0) {   // para evitar que se ejecute una partida, si el jugador prefirió salir

        // Para saber en que posición del puntero a función ejecutar_modo[]()
        // En el Enum, PVP es 0 (y Dificultad Normal 0) entonces Posicion 0; 
        // PVP es 0 (Y Dificultad Tormenta es 1) entonces 0*2 + 1 = 1  Posicion 1;
        // PVIA es 1 (Dificultad Normal 0) entonces posición 2 y PVIA Tormenta posición 3
        //int indice = partida.modo_de_juego * 2 + partida.dificultad;      

        partida_pvp(&partida);

        // Actualiza el ranking con el ganador de la ultima partida
        ranking_registrar_victoria(
            &ranking,
            partida.ganador == 1 
                ? partida.jugador1.nombre
                : partida.jugador2.nombre
        );
        // Implementar la función que corresponde
       // ejecutar_modo[indice](&partida);
    }

    ranking_guardar(&ranking, CONFIG_RANKING_RUTA);
*/
    return 0;
}

void colocar_barcos(Jugador *jugador, int tipo_de_partida){
    
    int indice = 4;

    while (indice >= 0){

        mostrar_tableros(*jugador, 0, tipo_de_partida);

        pedir_posicion_del_barco(jugador, indice);

        int validacion = validar_ingreso_barco(jugador->barco[indice], jugador->tableros.tablero);

        if (validacion == 1){
            indice--;
        }

        ui_confirmacion_barco_colocado(validacion);
        
        mostrar_tableros(*jugador, 0, tipo_de_partida);
    }

    // le da tiempo al jugador a ver su tablero después de poner el último barco
    if (indice == -1){
        esperar_usuario();
    }
}

// Hacer que reciba puntero a partida?
void ejecutar_turno_del_jugador(Jugador *atacante, Jugador *defensor, int turno, int tipo_de_partida){
    
    int fila_ataque, columna_ataque;

    Rdo_Disparo disparo;

    // pantalla cambio de turno
    mostrar_pantalla_cambio_de_turno(*atacante, turno);

    mostrar_tableros(*atacante, turno, tipo_de_partida);

    do {
        pedir_coordenadas_de_ataque(&fila_ataque, &columna_ataque);

        // determina el resultado del ataque
        disparo = recibir_disparo(fila_ataque, columna_ataque, defensor->tableros.tablero);

        // aplica el cambio en los tableros de ambos jugadores
        // si es disp_repetido, no hace nada
        aplicar_disparo(atacante, defensor, fila_ataque, columna_ataque, disparo);
  
        // actualiza el tablero en pantalla
        mostrar_tableros(*atacante, turno, tipo_de_partida);

        // avisa al usuario el resultado de su ataque
        resultado_del_ataque(disparo);

    // si el disparo es repetido, vuelve a pedir coords de ataque
    } while (disparo == disp_repetido);
}

// Partdida entre Personas
void partida_pvp(Partida *partida){

    partida->modo_de_juego = PVP;

    // muestra el menú de dificultad, si el usuario elige 0 vuelve al menú principal
    if (elegir_dificultad(&partida->dificultad) == 0){
        return;
    }

    // variable para representar el modo de juego
    int tipo_de_partida = partida->modo_de_juego * 2 + partida->dificultad;

    pedir_nombre_jugador(partida->jugador2.nombre, 2);
    
    colocar_barcos(&partida->jugador1, tipo_de_partida);

    mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->cantidad_turnos);

    colocar_barcos(&partida->jugador2, tipo_de_partida);

    partida->cantidad_turnos++;

    while (comprobar_partida_terminada(partida) == 0){
        
        int tormenta_fila, tormenta_columna;
        
        if (partida->turno == TURNO_PJ1){ //1
            
            ejecutar_turno_del_jugador(
                &partida->jugador1, 
                &partida->jugador2, 
                partida->cantidad_turnos,
                tipo_de_partida
            );
            
            // MODO TORMENTA
            if (partida->dificultad == TORMENTA){
                modo_tormenta(partida, &tormenta_fila, &tormenta_columna); // separar mensaje de tormenta y mostrar durante cambio de turno
            }
        }
        else if (partida->turno == TURNO_PJ2){  //2

            ejecutar_turno_del_jugador(
                &partida->jugador2, 
                &partida->jugador1, 
                partida->cantidad_turnos,
                tipo_de_partida
            );

        }


        cambiar_turno(partida);
    }

    mostrar_pantalla_de_victoria(partida);
}

/*
 * Ejecuta un turno completo de la IA.
 * Devuelve 1 si pudo disparar y 0 si no quedaban casillas disponibles.
 */
/*
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

    */
/*
 * Flujo comun para Jugador vs IA, tanto en dificultad Normal como Tormenta.
 * modo_tormenta() no hace nada si la dificultad es NORMAL.
 
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

    partida->turno = TURNO_PJ1;
    partida->cantidad_turnos = 1;

    while (comprobar_partida_terminada(partida) == 0) {

        if (partida->turno == TURNO_PJ1) {

            ejecutar_turno_del_jugador(&partida->jugador1,
                                       &partida->jugador2,
                                       partida->cantidad_turnos);

        } else if (partida->turno == TURNO_PJ2) {

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

    partida->modo_de_juego = PVIA;

    if (elegir_dificultad(&partida->dificultad) == 0){
        return;
    }

    jugar_pvia(partida);
}
*/