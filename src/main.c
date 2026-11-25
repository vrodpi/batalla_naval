#include "../include/config.h"
#include "../include/ui_cli.h"
#include "../include/ai.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
/*
 *  PENDIENTES:
        - Error al ingresar fila alfanumérica
        - Arrepentirse del disparo?
        - Salir de la partida
*/
void partida_pvp(Partida *partida);
void partida_pvia(Partida *partida);


int main(void){

    Partida partida;
    Ranking ranking;
    int opcion_menu;
    
    ranking_inicializar(&ranking);

    // carga el ranking del achivo al programa
    ranking_cargar(&ranking, CONFIG_RANKING_RUTA);

    srand(time(NULL)); // para modo tormenta

    ui_mostrar_presentacion_y_reglamento(partida.jugador1.nombre, CONFIG_REGLAMENTO_RUTA);

    do {
        inicializar_partida(&partida);
        
        ui_mostrar_menu_principal();

        ui_pedir_opcion_al_usuario("Ingrese una opción: ", &opcion_menu);

        switch (opcion_menu){
            case UI_MENU_OPCION_PVP:

                partida.modo_de_juego = PVP;

                // muestra el menú de dificultad, si el usuario elige 0 vuelve al menú principal
                if (ui_elegir_dificultad(&partida.dificultad) == 0){
                    break;
                }

                // Ejecuta la partida vs jugador
                partida_pvp(&partida);
                    
                // Actualiza el ranking con el ganador de la ultima partida
                ranking_registrar_victoria(&ranking,
                    partida.ganador == 1 
                        ? partida.jugador1.nombre
                        : partida.jugador2.nombre
                );

                break;

            case UI_MENU_OPCION_PVIA:

                partida.modo_de_juego = PVIA;

                // muestra el menú de dificultad, si el usuario elige 0 vuelve al menú principal
                if (ui_elegir_dificultad(&partida.dificultad) == 0){
                    break;
                }

                // Ejecuta la partida vs IA
                partida_pvia(&partida);

                // Actualiza el ranking sólo si ganó el jugador 1
                if (partida.ganador == 1){
                    ranking_registrar_victoria(&ranking, partida.jugador1.nombre);
                }

                break;
            
            case UI_MENU_MOSTRAR_RANKING:
                ui_mostrar_ranking(ranking);
                break;

            case UI_MENU_MOSTRAR_REGLAMENTO:
                ui_mostrar_reglamento(CONFIG_REGLAMENTO_RUTA);
                break;

            case UI_MENU_SALIR:
                ui_mensajes_menu(UI_MENSAJE_SALIR);
                break;
            
            default:
                ui_mensajes_menu(UI_MENSAJE_DEFAULT);
                break;
        }
    } while (opcion_menu != 0);
    
    // guarda el ranking en un archivo
    ranking_guardar(&ranking, CONFIG_RANKING_RUTA);
    
    return 0;
}

static void colocar_barcos(Jugador *jugador, int tipo_de_partida){
    
    int indice = BARCOS_MODO_NORMAL - 1;

    while (indice >= 0){

        ui_mostrar_tableros(*jugador, 0, tipo_de_partida);

        ui_pedir_posicion_del_barco(jugador, indice);

        // guarda el resultado de la validacion
        int validacion = validar_ingreso_barco(jugador->barco[indice], jugador->tableros.tablero);

        // si el barco ingresó, se disminuye el indice
        if (validacion == 1){
            indice--;
        }

        // actualiza los tableros
        ui_mostrar_tableros(*jugador, 0, tipo_de_partida);
        
        // muestra un mensaje de acuerdo a la validacion
        ui_confirmacion_barco_colocado(validacion);
    }
}

// Ejecuta el turno de los jugadores
static void ejecutar_turno_del_jugador(Partida *partida, int tipo_de_partida){
    
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
    ui_mostrar_pantalla_cambio_de_turno(*atacante, partida->cantidad_turnos);
    
    // muestra la información del modo tormenta si se activa
    if (modo_tormenta(partida, &fila_ataque, &columna_ataque) == 1){
        ui_mostrar_mensaje_tormenta(defensor, fila_ataque, columna_ataque);
        // revisa si la tormenta hundió el último barco
        if (comprobar_partida_terminada(partida) == 1){
            return;
        }
    }

    // actualizar tableros
    ui_mostrar_tableros(*atacante, partida->cantidad_turnos, tipo_de_partida);
    
    do {
        ui_pedir_coordenadas_de_ataque(&fila_ataque, &columna_ataque);

        // determina el resultado del ataque
        disparo = recibir_disparo(fila_ataque, columna_ataque, defensor->tableros.tablero);

        // aplica el cambio en los tableros de ambos jugadores
        // si es disp_repetido, no hace nada
        aplicar_disparo(atacante, defensor, fila_ataque, columna_ataque, disparo);
  
        // actualiza el tablero en pantalla
        ui_mostrar_tableros(*atacante, partida->cantidad_turnos, tipo_de_partida);

        // avisa al usuario el resultado de su ataque
        ui_resultado_del_ataque(disparo);

    // si el disparo es repetido, vuelve a pedir coords de ataque
    } while (disparo == disp_repetido);
}

// Partdida entre Personas
void partida_pvp(Partida *partida){

    // variable para representar el modo de juego
    int tipo_de_partida = partida->modo_de_juego * 2 + partida->dificultad;


    ui_pedir_nombre_jugador(partida->jugador2.nombre, 2);
    
    // Colocación de barcos
    colocar_barcos(&partida->jugador1, tipo_de_partida);

    ui_mostrar_pantalla_cambio_de_turno(partida->jugador2, partida->cantidad_turnos);

    colocar_barcos(&partida->jugador2, tipo_de_partida);

    partida->cantidad_turnos = 1;

    // Ejecución de la partida
    while (comprobar_partida_terminada(partida) == 0){
        
        ejecutar_turno_del_jugador(partida, tipo_de_partida);
            
        cambiar_turno(partida);
    }

    ui_mostrar_pantalla_de_victoria(partida);
}

/*
 * Ejecuta un turno completo de la IA.
 * Devuelve 1 si pudo disparar y 0 si no quedaban casillas disponibles.
 */
static int ejecutar_turno_ia(Partida *partida, EstadoIA *estado, int tipo_de_partida){
    
    int fila, columna;
    Rdo_Disparo disparo;

    ui_mostrar_pantalla_cambio_de_turno(partida->jugador2,
                                     partida->cantidad_turnos);

    // Ejecuta el modo tormenta si corresponde
    if (modo_tormenta(partida, &fila, &columna) == 1){
        // Muestra la casilla afectada
        ui_mostrar_mensaje_tormenta(&partida->jugador1, fila, columna);

        if (comprobar_partida_terminada(partida) == 1){
            return 1;
        }
    }

    // Solo se muestra el tablero del humano; la flota de la IA queda oculta.
    ui_mostrar_tableros(partida->jugador1, partida->cantidad_turnos, tipo_de_partida);

    do {
        if (!ai_elegir_disparo(&partida->jugador2, estado,
                               &fila, &columna)) {
            return 0;
        }

        disparo = recibir_disparo(fila, columna,
                                  partida->jugador1.tableros.tablero);

        aplicar_disparo(&partida->jugador2, &partida->jugador1,
                        fila, columna, disparo);

        ai_registrar_resultado(estado, &partida->jugador2, fila, columna,
                               disparo);

        ui_mostrar_tableros(partida->jugador1, partida->cantidad_turnos, tipo_de_partida);
        
        // Resultado del ataque
        ui_mensaje_disparo_ia(partida->jugador2.nombre, fila, columna);
        ui_resultado_del_ataque(disparo);

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

    // variable para representar el modo de juego
    int tipo_de_partida = partida->modo_de_juego * 2 + partida->dificultad;

    ai_inicializar(&estado);

    strcpy(partida->jugador2.nombre, "SKYNET");

    if (!ai_colocar_flota(&partida->jugador2)) {
        ui_mensaje_error_ia(UI_MENSAJE_FLOTA);
        return;
    }

    colocar_barcos(&partida->jugador1, tipo_de_partida);

    partida->turno = TURNO_PJ1;
    partida->cantidad_turnos = 1;

    while (comprobar_partida_terminada(partida) == 0) {

        if (partida->turno == TURNO_PJ1) {

            ejecutar_turno_del_jugador(partida, tipo_de_partida);

        } else if (partida->turno == TURNO_PJ2) {

            if (!ejecutar_turno_ia(partida, &estado, tipo_de_partida)) {
                ui_mensaje_error_ia(UI_MENSAJE_DISPARO);
                return;
            }
        }

        cambiar_turno(partida);
    }

    ui_mostrar_pantalla_de_victoria(partida);
}

void partida_pvia(Partida *partida){

    jugar_pvia(partida);
}
