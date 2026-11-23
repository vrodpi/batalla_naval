#include "../include/game.h"
#include "../include/tablero.h"
#include "../include/ui_cli.h"

#include <stdlib.h>
#include <time.h>

void inicializar_partida(Partida *partida) {
      
    partida->turno = PJ1;  
    partida->cantidad_turnos = 0;
    partida->ganador = 0;

    // Inicializar los barcos vivos
    partida->jugador1.barcos_vivos = BARCOS_MODO_NORMAL;        // cada jugador empieza con 5 barcos
    partida->jugador2.barcos_vivos = BARCOS_MODO_NORMAL;

    
    // Inicializar los impactos
    partida->jugador1.hits = 0;                 // cada jugador empieza con 0 impacto a barcos
    partida->jugador2.hits = 0;

    partida->jugador1.impacto_agua_consecutivo= 0;          // el contador de impactos a agua, comienza en 0.
    partida->jugador2.impacto_agua_consecutivo= 0;

    // Llenar los tableros de AGUA
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
        partida->jugador1.barco[i].tamanio = i + 1;
        partida->jugador2.barco[i].tamanio = i + 1;
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
    if (partida->turno == PJ1){     // 1
        partida->turno = PJ2;      // 2

    } else {
        partida->turno = PJ1;       //1
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

// aplica los cambios en los tableros del jugador atacante (en el tablero de disparos)y defensor (tablero)
// si juega el jugador1 (atacante) el defensor es el jugador2
void aplicar_disparo(Jugador *atacante, Jugador *defensor, int fila, int columna, Rdo_Disparo disparo){

    switch (disparo){

        // si el disparo fue al agua
        case disp_agua:
            
            // se actualiza el tablero de barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_DISP_AGUA;
            
            // se registran sus impactos en el tablero de disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_DISP_AGUA;
            atacante->impacto_agua_consecutivo++;
           
            return;

        // si el disparo fue impacto
        case disp_impacto:

            // se actualiza el tablero de barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_IMPACTO;
            
            // y el tablero de disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_IMPACTO;

            // se aumentan los hits del atacante
            atacante->hits++;
            
            //Se reinicia el contador porque el impacto fue distinto a agua
            atacante->impacto_agua_consecutivo=0;

            // se comprueba si se hundió un barco
            barco_hundido(defensor);
            
            return;
        case disp_tormenta:
            // se actualiza el tablero de barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_TORMENTA;
            
            // y el tablero de disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_TORMENTA;

        
        // si el disparo es repetido, no hace nada
        case disp_repetido:
            
            return;
    } 
}




// Tormenta cada 3 turnos.
void modo_tormenta(Partida *partida){

    srand(time(NULL));
    
    //  Si la dificultad no es Tormenta sale de la función
    if(partida->dificultad != TORMENTA){
        return;
    }

    //Determina a que jugador se le aplica el daño
    // Identificar atacante y defensor.
    Jugador *atacante;
    Jugador *defensor;

    if (partida->turno == PJ1){
            atacante = &partida->jugador1;
            defensor = &partida->jugador2;
    } else {
            atacante = &partida->jugador2;
            defensor = &partida->jugador1;
            }


    int tormenta_turnos = (partida->cantidad_turnos + 1) % 12 == 0;
    int tormenta_agua=atacante->impacto_agua_consecutivo >= 3;

    // Verifica que hallan pasado 3 turnos.  Multiplo de 3, por eso el %
    // Se ejecuta antes de cambiar_turno. (por eso +1)  en donde se incrementa el contador
    // Verifica que la cantidad de impactaos a agua consecutivos hechos por el atacante no sea menor a 3. 

    if(!tormenta_turnos &&  !tormenta_agua) {       // por lo tanto, sino se cumple ninguna retorna
        return;
    }
    
    

    //Arreglos temporales para guardar posiciones que aun tienen barcos activos
    int Fil_Tormenta[FIL*COL];   // 100 casillas= 10*10
    int Col_Tormenta[FIL*COL];

    int cont_barcos = 0;

    //Recorre el tablero del defensor, en busca de lugares ocupados
    // #: barco no descubierto. 
    for (int i = 0; i < FIL; i++){        // recorre las filas
        for (int j = 0; j < COL; j++){          // recorre las columnas
            if (defensor->tableros.tablero[i][j] == CASILLA_BARCO){
                Fil_Tormenta[cont_barcos] = i;
                Col_Tormenta[cont_barcos] = j;

                cont_barcos++;
            }
        }
    }

    //Si no hay partes de barcos disponibles, la tormenta no hace efecto.
    if (cont_barcos == 0){
        return;
    }

    //Elige de forma aleatoria una posición
    int k = rand() % cont_barcos; 

    int Fil_afectada = Fil_Tormenta[k];
    int Col_afectada = Col_Tormenta[k];
    
    // Registrar los impactos en los tableros 
    aplicar_disparo(atacante, defensor,Fil_afectada, Col_afectada, disp_tormenta); 

    mostrar_mensaje_tormenta(defensor, Fil_afectada, Col_afectada);
    
    // reiniciar el contador 0, si se activo tormenta por cantidad de impactos a agua (3 consecutivas)
    if(tormenta_agua){
        atacante->impacto_agua_consecutivo = 0;
    }

}