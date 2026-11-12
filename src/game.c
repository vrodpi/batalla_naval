#include "../include/game.h"
#include "../include/tablero.h"

void inicializar_partida(Partida *partida) {
      
    partida->turno = 1;
    partida->cantidad_turnos = 0;
    partida->ganador = 0;

    // Inicializar los barcos vivos
    partida->jugador1.barcos_vivos = BARCOS_MODO_NORMAL;        // cada jugador empieza con 5 barcos
    partida->jugador2.barcos_vivos = BARCOS_MODO_NORMAL;

    
    // Inicializar los impactos
    partida->jugador1.hits = 0;                 // cada jugador empieza con 0 impacto a barcos
    partida->jugador2.hits = 0;

    inicializar_tablero(
        partida->jugador1.tableros.tablero,
        partida->jugador1.tableros.tablero_disparos
    );
    inicializar_tablero(
        partida->jugador2.tableros.tablero,
        partida->jugador2.tableros.tablero_disparos
    );

   // inicializar el estado de los barcos
    for(int i = 0; i < BARCOS_MODO_NORMAL; i++){
        partida->jugador1.barco[i].hundido = 0;
        partida->jugador2.barco[i].hundido = 0;
    }
}

// para verificar que un jugador se quedó sin barcos vivos
int comprobar_partida_terminada(Partida *partida){

    // si el n° de barcos vivos  del jugador1 es menor igual a cero entonces
    if(partida->jugador1.barcos_vivos <= 0){
        
        // el ganador de la partida es el jugador 2
        partida->ganador = 2;
        
         // retorna 1, indicando que hay ganador
        return 1;
    } 
    else if(partida->jugador2.barcos_vivos <= 0){    
        
        // el ganador será el jugador1 si el el jugador2 se quedó sin barcos vivos
        partida->ganador = 1;
        
        return 1;
    }
    
    return 0; // indica que el juego continua
}

// para definir de quien es el turno
void cambiar_turno(Partida *partida){
    
    // Cambiamos el turno
    if (partida->turno == 1){
        partida->turno = 2;

    } else {
        partida->turno = 1;
    }

    partida->cantidad_turnos++;
}

// determina donde cae el disparo
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]) {

    // En caso de que haya AGUA ~
    if (tablero[fila_disparo][columna_disparo] == CASILLA_AGUA) {

        return disp_agua;

    } 
    // si hay un BARCO...
    else if (tablero[fila_disparo][columna_disparo] == CASILLA_BARCO) { 
        
        return disp_impacto; 

    } 

    // si no fue agua, ni barco, entonces el disparo esta repetido
    return disp_repetido;    
}


// verificar si algún barco fue hundido
void barco_hundido(Jugador *defensor) {

    for (int i = 0; i < BARCOS_MODO_NORMAL; i++) {          // 5 barcos  por jugador
       
        // para verificiar el estado de todos los barcos que no han sido hundidos
        if (defensor->barco[i].hundido == 0) {   

            int hundido = 1;   // asumimos que el barco fue hundido (verdadero)

          
            // recorrer las posiciones de los barcos (depende de su tamanio)
            for (int j = 0; j < defensor->barco[i].tamanio; j++) {

                int fila = defensor->barco[i].fila; 
                int columna = defensor->barco[i].columna;

                // calcular la posición de acuerdo a la posición
                if (defensor->barco[i].orientacion == horizontal) {
                    columna += j;
                } else {
                    fila += j;
                }

                // si hay alguna Posición sin impacto
                if (defensor->tableros.tablero[fila][columna] != CASILLA_IMPACTO) {
                    hundido = 0; // entonces aún no está hundido
                }
            }

        
            // si realmente el barco fue hundido
            if (hundido == 1) {

                // se actualiza el estado del barco
                defensor->barco[i].hundido = 1;

                // se reduce el numero de barcos vivos
                defensor->barcos_vivos--;
            }
        }
    }
}

// aplica los cambios en los tableros del jugador atacante y defensor
// si juega el jugador1 (atacante) el defensor es el jugador2
void aplicar_disparo(Jugador *atacante, Jugador *defensor, int fila, int columna, Rdo_Disparo disparo){

    switch (disparo){

        // si el disparo fue al agua
        case disp_agua:
            
            // se actualiza el tablero de barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_DISP_AGUA;
            
            // y el tablero de disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_DISP_AGUA;
           
            return;

        // si el disparo fue impacto
        case disp_impacto:

            // se actualiza el tablero de barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_IMPACTO;
            
            // y el tablero de disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_IMPACTO;

            // se aumentan los hits del atacante
            atacante->hits++;

            // se comprueba si se hundió un barco
            barco_hundido(defensor);
            
            return;
        
        // si el disparo es repetido, no hace nada
        case disp_repetido:
            
            return;
    } 
}
