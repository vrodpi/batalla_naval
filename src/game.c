#include "../include/game.h"
#include <stdlib.h>


void inicializar_partida(Partida *partida) {
      
    partida->turno = TURNO_PJ1;  // jugador de Turno Jugador1 
    partida->cantidad_turnos = 0;
    partida->ganador = 0;       // al iniciar la partida, no hay ganadores

    // Inicializar los barcos vivos
    partida->jugador1.barcos_vivos = BARCOS_MODO_NORMAL;        // cada jugador empieza con 5 barcos
    partida->jugador2.barcos_vivos = BARCOS_MODO_NORMAL;

    
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

        // al iniciar la partida ningún barco fue hundido
        partida->jugador1.barco[i].hundido = 0;     
        partida->jugador2.barco[i].hundido = 0;

        // indicar el tamaño de cada barco
        partida->jugador1.barco[i].tamanio = i + 1;      // Barco[0] --> Lancha ocupa 1 casilla
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
    if (partida->turno == TURNO_PJ1){     // Si el primer turno fue del jugador1, luego el turno pasa a ser del jugador2
        partida->turno = TURNO_PJ2;      

    } else {
        partida->turno = TURNO_PJ1;       //1
    }

    partida->cantidad_turnos++;     // el n° de turno aumenta
}


// determina a que disparó
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]) {

    // Si en el casillero habia Agua ~
    if (tablero[fila_disparo][columna_disparo] == CASILLA_AGUA) {

        return disp_agua;

    } 
    // si habia un BARCO...
    else if (tablero[fila_disparo][columna_disparo] == CASILLA_BARCO) { 
        
        return disp_impacto; 

    } 

    // si no fue agua, ni barco, entonces la coordenada de ataque fue repetida.
    return disp_repetido;    
}


// verificar si algún barco fue hundido
void barco_hundido(Jugador *defensor) {

    for (int i = 0; i < BARCOS_MODO_NORMAL; i++) {          // 5 barcos  por jugador
       
        // para verificiar el estado de todos los barcos que no han sido hundidos
        if (defensor->barco[i].hundido == 0) {   

            int hundido = 1;   // asumimos que el barco fue hundido (verdadero)

          
            // recorrer las posiciones de los barcos (depende de su tamaño)
            for (int j = 0; j < defensor->barco[i].tamanio; j++) {

                int fila = defensor->barco[i].fila;   // el barco empieza en la fila...
                int columna = defensor->barco[i].columna;

                // si es horizontal entonces desde donde empieza hasta el tamaño -1 (J va de 0 a Tamaño -1) 
                if (defensor->barco[i].orientacion == horizontal) {
                    columna += j;
                } else {
                    fila += j;      // fila= fila + j // sumar la cantidad de filas
                }

                // si hay alguna Posición sin impacto
                if (defensor->tableros.tablero[fila][columna] != CASILLA_IMPACTO && 
                    defensor->tableros.tablero[fila][columna] != CASILLA_TORMENTA) {
                    
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

// aplica los cambios en los tableros del jugador atacante (en el Tablero de Disparos)y defensor (Tablero de Barcos)
// si juega el jugador1 (atacante) el defensor es el jugador2
void aplicar_disparo(Jugador *atacante, Jugador *defensor, int fila, int columna, Rdo_Disparo disparo){

    switch (disparo){

        // si el disparo fue al agua
        case disp_agua:
            
            // se actualiza el Tablero de Barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_DISP_AGUA;
            
            // se registran sus impactos en el Tablero de Disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_DISP_AGUA;
            atacante->impacto_agua_consecutivo++;       // aumenta impacto a agua, válido solo para Dificultad Tormenta
           
            return;  // entonces la función termina y vuelve al lugar donde se llamó --> ejecutar_turno_del_jugador(partida, tipo_de_partida); y/o en modo_tormenta(partida,fila_afectada,columna_afectada

        // si disparo impacto sobre la ubicación de algún barco
        case disp_impacto:

            // se actualiza el Tablero de Barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_IMPACTO;
            
            // y el Tablero de Disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_IMPACTO;

            //Se reinicia el contador porque el impacto fue distinto a agua. Válido para Dificultad Tormenta
            atacante->impacto_agua_consecutivo = 0;

            // se comprueba si se hundió un barco
            barco_hundido(defensor);
            
            return;
            
        case disp_tormenta:
            // se actualiza el Tablero de Barcos del defensor
            defensor->tableros.tablero[fila][columna] = CASILLA_TORMENTA;
            
            // y el Tablero de Disparos del atacante
            atacante->tableros.tablero_disparos[fila][columna] = CASILLA_TORMENTA;

            // se comprueba si se hundió un barco
            barco_hundido(defensor);
        
            return;

        // si el disparo es repetido, no hace nada
        case disp_repetido:
            
            return;
    } 
}



// Tormenta cada 5 turnos || Por 3 disparos consecutivos a Agua
// Devuelve 1 si se activó en el turno
int modo_tormenta(Partida *partida, int *fila_afectada, int *columna_afectada){

    //  Si la dificultad no es Tormenta sale de la función
    if(partida->dificultad != TORMENTA){
        return 0;
    }

    //Determina a que jugador se le aplica el daño
    // Identificar atacante y defensor.
    Jugador *atacante;
    Jugador *defensor;

    if (partida->turno == TURNO_PJ1){
            atacante = &partida->jugador1;
            defensor = &partida->jugador2;
    } else {
            atacante = &partida->jugador2;
            defensor = &partida->jugador1;
            }

    // Verifica que hallan pasado 5 turnos.  Multiplo de 5, por eso el %
    // Verifica que la cantidad de impactos a agua consecutivos hechos por el atacante no sea menor a 3. 
    int tormenta_turnos = partida->cantidad_turnos % 5 == 0;
    int tormenta_agua = atacante->impacto_agua_consecutivo >= 3;


    if(!tormenta_turnos && !tormenta_agua){       // por lo tanto, sino se cumple ninguna retorna
        return 0;
    }
    
    

    //Arreglos temporales para guardar posiciones que aun tienen barcos activos
    int Fil_Tormenta[FIL*COL];   // 100 casillas= 10*10 // int Fil_Tormenta[100]
    int Col_Tormenta[FIL*COL];  // arreglo que guarda las columnas

    int cont_barcos = 0;    // 0 partes de barcos encontrados

    //Recorre el tablero del defensor, en busca de lugares ocupados
    // # = barco o parte del barco aún no descubierto = casilla_barco 
    for (int i = 0; i < FIL; i++){        // recorre todas las filas del Tablero de Barcos 
        for (int j = 0; j < COL; j++){          // recorre todas las columnas del Tablero de Barcos
            if (defensor->tableros.tablero[i][j] == CASILLA_BARCO){   // si encontramos algún barco

                // recordar que primero cont_barcos=0-->Fil_Tormenta[0]=?
                Fil_Tormenta[cont_barcos] = i; // Fil[0]=una parte del barco se encontró en la fila i
                Col_Tormenta[cont_barcos] = j; // fila i y columna j.

                cont_barcos++;   // cantidad de casilleros con barcos activos 
            }
        }
    }

    //Si no hay partes de barcos disponibles, la tormenta no hace efecto.
    if (cont_barcos == 0){
        return 0;
    }

    //Elige de forma aleatoria una posición de ataque
    int k = rand() % cont_barcos; 

    *fila_afectada = Fil_Tormenta[k];
    *columna_afectada = Col_Tormenta[k];
    
    // Registrar los impactos en los tableros 
    aplicar_disparo(atacante, defensor, *fila_afectada, *columna_afectada, disp_tormenta); 

    // reiniciar el contador 0, si se activo tormenta por cantidad de impactos a agua (3 consecutivas)
    if(tormenta_agua){
        atacante->impacto_agua_consecutivo = 0;  
    }

    // se activó el modo tormenta en el turno actual
    return 1;
}