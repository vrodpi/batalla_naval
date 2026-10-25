#include <stdio.h>

#include "../include/tablero.h"





// Que los tableros solo contengan agua
void inicializar_tablero (char posicion[FIL][COL], char posicion_disparos[FIL][COL]) {
    for (int i = 0; i < FIL; i++){    // recorre las filas
        for (int j = 0; j < COL; j++){  // recorre las columnas
            posicion[i][j] = '~';
            posicion_disparos[i][j] = '~';
        }
    }
}

// para determinar si la coordenada ingresada es válida (que la coordenada esté dentro del tablero)); 
// El jugador ingresa "1","0"; 1 es fila y 0 es columna, en gráfica el 0 es A
int rango(int fila, int columna, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]) {    // n° de fila y n° de columnna ingresada por el jugador. 
    // filas
    for (int i=0; i<FIL; i++) {
        if(fila>=0 && fila<=FIL && columna>=0 && columna<=COL) {   // coordenada válida?  (dentro de la función); 
            if(Orientacion==horizontal) {                             // si es horizontal
                if((columna + tamanio)<=COL) {        // si coordenada columna + tamaño es menor o igual a 10, entonces el barco entra completo, es válido.  
                    return 1; // indica que esta dentro del rango. Coordenada válida
                } 
            } else if (Orientacion==vertical) {
                if((fila + tamanio)<=FIL) {        // si coordenada columna + tamaño es menor o igual a 10, entonces el barco entra completo, es válido.  
                    return 1; // indica que esta dentro del rango. Coordenada válida
                } 
            }

        }
           
    }
    return 0;       // retorna 0, si no es posible ingresa el barco, porque no entra o las coordenadas no son válidas
}

// que los barcos no se superpongan y que haya un espacio entre los barcos. 
int libre(int fila, int columna, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]) {


    if (orientacacion == horizontal) {      // si la orientación ingresada es horizontal
        for(int i=0; i<tamanio;i++) {       // desde 0, hasta el tamaño del barco
            if(posicion[fila][columna +i] !='~') {    // comprobar si el barco completo entra en un espacio en donde solo hay agua
                return 0;
            }
            if(fila>0 && posicion[fila-1][columna +i] !='~') {    // comprobar los cuadrantes de arriba del barco
                return 0;
            }
            if(fila < FIL -1 && posicion[fila+1][columna +i] !='~') {       // comprobar los cuadrantes de abajo del barco
                return 0;
            }
            if(columa > 0 && posicion[fila][columna - 1] !='~'){           // comprobar el cuadrante anterior inmediato a la izquierda
                return 0;
            }
            if(columna + tamanio < COL &&  posicion[fila][columna + tamanio] != '~') {    // Siempre que esté por dentro del rango, comprobar el cuadrante inmediato a la derecha
                return 0:
            }
        }
    } else { 
         for(int i=0; i<tamanio<;i++) {
            if(posicion[fila + i][columna] !='~') {    // comprobar si el barco completo entra en un espacio en donde solo hay agua
                return 0;
            }
            if(columna > 0 && posicion[fila + i ][columna - 1] !='~') {    // comprobar los cuadrantes a la derecha
                return 0;
            }
            if(columna < COL -1 && posicion[fila+i][columna +1] !='~') {       // comprobar los cuadrantes a la izquierda
                return 0;
            }
            if(fila > 0 && posicion[fila -1][columna ] !='~'){           // comprobar el cuadrante anterior inmediato 
                return 0;
            }
            if(fila + tamanio < FIL &&  posicion[fila + tamanio][columna] != '~') {    // Siempre que esté por dentro del rango, comprobar el cuadrante  posterior inmediato 
                return 0;
            }
         }
         return 1; // no hay impedimentos, en el lugar donde va el barco y sus alrededores está vacio, apto para ingresarlo al tablero.
    }
}

// para cambiar ~ (agua) por # (barco)
void ingresar_barco(int fila, int columna, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]){
    for(int i =0; i<tamanio;i++){
        if(orientacion==horizontal) {
            posicion[fila][columna+i]='#';
        } else {
            posicion[fila+i][columna]='#';
        }
    }
}


// muestra los dos tableros, el del barcos del jugador de turno y el tablero de disparos al oponente
void mostrar_tablero(char posicion[FIL][COL], char posicion_disparos[FIL][COL]) {
    printf("\n---MI FLOTA---\n   ");
    for(int k=0; k<COL;k++) printf("%c ",'A'+k);
    printf("\n");
    for(int i=0; i<FIL;i++) {
        printf("%2d ",i+1);
        for(int k=0; k<COL;k++) {
            printf("%c ",posicion[i][k]);
        }
        printf("\n");
    }
    printf("\n---DISPAROS A FLOTA ENEMIGA---\n   ");
    for(int k=0; k<COL;k++) printf("%c ",'A'+k);
    printf("\n");
    for(int i=0; i<FIL;i++) {
        printf("%2d ",i+1);
        for(int k=0; k<COL;k++) {
            printf("%c ",posicion_disparos[i][k]);
        }
        printf("\n");
    }
}

