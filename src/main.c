#include "../include/config.h"
#include "../include/ui_cli.h"
#include "../include/ranking.h"
#include "../include/ai.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int partida_pvp(Partida *partida);
//void partida_pvia(Partida *partida);


int main(void){

    Partida partida;
    Ranking ranking;
    int opcion_menu;
    
    ranking_inicializar(&ranking);

    ranking_cargar(&ranking, CONFIG_RANKING_RUTA);

    srand(time(NULL)); // para modo tormenta

    ui_mostrar_presentacion_y_reglamento(partida.jugador1.nombre, CONFIG_REGLAMENTO_RUTA);

    do {
        inicializar_partida(&partida);
        
        mostrar_menu_principal();

        opcion_menu = pedir_opcion_al_usuario("Ingrese una opción: ");

        switch (opcion_menu){
            case UI_MENU_OPCION_PVP:

                partida.modo_de_juego = PVP;

                // muestra el menú de dificultad, si el usuario elige 0 vuelve al menú principal
                if (elegir_dificultad(&partida.dificultad) == 0){
                    break;
                }

                // Ejecuta la partida
                if (partida_pvp(&partida) == 1){
                    
                    // Actualiza el ranking con el ganador de la ultima partida
                    ranking_registrar_victoria(&ranking,
                        partida.ganador == 1 
                            ? partida.jugador1.nombre
                            : partida.jugador2.nombre
                    );

                    // mostrar ranking
                }
                break;

            case UI_MENU_OPCION_PVIA:
                partida.modo_de_juego = PVIA;

                // muestra el menú de dificultad, si el usuario elige 0 vuelve al menú principal
                if (elegir_dificultad(&partida.dificultad) == 0){
                    break;
                }

         //       partida_pvia(&partida);
                break;
            
            case UI_MENU_MOSTRAR_RANKING:
                ui_mostrar_ranking(ranking);
                break;

            case UI_MENU_MOSTRAR_REGLAMENTO:
                
                ui_mostrar_reglamento(CONFIG_REGLAMENTO_RUTA);
                
                break;

            case UI_MENU_SALIR:
                printf("Saliendo...\n");
                break;
            
            default:
                printf("Opción inválida.\n");
                esperar_usuario();
                break;
        }
    } while (opcion_menu != 0);
    
    ranking_guardar(&ranking, CONFIG_RANKING_RUTA);
    
    return 0;
}

void colocar_barcos(Jugador *jugador, int tipo_de_partida){
    
    int indice = BARCOS_MODO_NORMAL - 1;

    while (indice >= 0){

        ui_mostrar_tableros(*jugador, 0, tipo_de_partida);

        pedir_posicion_del_barco(jugador, indice);

        // guarda el resultado de la validacion
        int validacion = validar_ingreso_barco(jugador->barco[indice], jugador->tableros.tablero);

        // si el barco ingresó, se disminuye el indice
        if (validacion == 1){
            indice--;
        }

        // muestra un mensaje de acuerdo a la validacion
        ui_confirmacion_barco_colocado(validacion);
        
        // actualiza los tableros
        ui_mostrar_tableros(*jugador, 0, tipo_de_partida);
    }

    // le da tiempo al jugador a ver su tablero después de poner el último barco
    if (indice == -1){
        esperar_usuario();
    }
}

// Hacer que reciba puntero a partida?
void ejecutar_turno_del_jugador(Partida *partida, int tipo_de_partida){
    
    Jugador *atacante, *defensor;

    // determina de quien es el turno
    if (partida->turno == TURNO_PJ1){
        atacante = &partida->jugador1;
        defensor = &partida->jugador2;
    }
    else {
        atacante = &partida->jugador2;
        defensor = &partida->jugador1;
    }

    int fila_ataque, columna_ataque;

    Rdo_Disparo disparo;

    // pantalla cambio de turno
    mostrar_pantalla_cambio_de_turno(*atacante, partida->cantidad_turnos);
    
    // muestra la información del modo tormenta si se activa
    if (modo_tormenta(partida, &fila_ataque, &columna_ataque) == 1){
        mostrar_mensaje_tormenta(defensor, fila_ataque, columna_ataque);
    }

    // actualizar tableros
    ui_mostrar_tableros(*atacante, partida->cantidad_turnos, tipo_de_partida);
    
    do {
        pedir_coordenadas_de_ataque(&fila_ataque, &columna_ataque);

        // determina el resultado del ataque
        disparo = recibir_disparo(fila_ataque, columna_ataque, defensor->tableros.tablero);

        // aplica el cambio en los tableros de ambos jugadores
        // si es disp_repetido, no hace nada
        aplicar_disparo(atacante, defensor, fila_ataque, columna_ataque, disparo);
  
        // actualiza el tablero en pantalla
        ui_mostrar_tableros(*atacante, partida->cantidad_turnos, tipo_de_partida);

        // avisa al usuario el resultado de su ataque
        resultado_del_ataque(disparo);

    // si el disparo es repetido, vuelve a pedir coords de ataque
    } while (disparo == disp_repetido);
}

// Partdida entre Personas
int partida_pvp(Partida *partida){

    // variable para representar el modo de juego
    int tipo_de_partida = partida->modo_de_juego * 2 + partida->dificultad;


    pedir_nombre_jugador(partida->jugador2.nombre, 2);
    
    colocar_barcos(&partida->jugador1, tipo_de_partida);

    mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->cantidad_turnos);

    colocar_barcos(&partida->jugador2, tipo_de_partida);

    partida->cantidad_turnos++;

    while (comprobar_partida_terminada(partida) == 0){
        
        ejecutar_turno_del_jugador(partida, tipo_de_partida);
            
        cambiar_turno(partida);
    }

    mostrar_pantalla_de_victoria(partida);

    return 1;
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
    ui_mostrar_tableros(partida->jugador1, partida->cantidad_turnos);

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

        ui_mostrar_tableros(partida->jugador1, partida->cantidad_turnos);
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