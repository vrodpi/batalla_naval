#include <stdio.h>

#include "../include/tablero.h"



typedef struct {
    char posicion[FIL][COL];  // Tablero con mis barcos.
    char posicion_disparos[FIL][COL];   // Tablero de disparos al oponente.
} Tablero;






// Que el tablero solo tenga agua
void inicializar_tablero (char posicion[FIL][COL], char posicion_disparos[FIL][COL]) {
    for (int i = 0; i < FIL; i++){
        for (int j = 0; j < COL; j++){
            posicion[i][j] = '~';
            posicion_disparos[i][j] = '~';
        }
    }
}

// para determinar si la coordenada ingresada es válida (que la coordenada esté dentro del tablero)); 
int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]) {
    // filas
    for (int i=0; i<fil; i++) {
        if(filas>=0 && filas<=FIL && columnas>=0 && columnas<=COL) {   // coordenada válida?  (dentro de la función); 
            if(Orientacion==horizontal) {                             // si es horizontal
                if((columnas + tamanio)<=COL) {        // si coordenada columna + tamaño es menor o igual a 10, entonces el barco entra completo, es válido.  
                    return 1; // indica que esta dentro del rango. Coordenada válida
                } 
            } else if (Orientacion==vertical) {
                if((filas + tamanio)<=FIL) {        // si coordenada columna + tamaño es menor o igual a 10, entonces el barco entra completo, es válido.  
                    return 1; // indica que esta dentro del rango. Coordenada válida
                } 
            }

        }
           
    }
    return 0; 
}

// que los barcos no se superpongan y que haya un espacio entre los barcos. 
int libre(int filas, int columnas, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]) {


    if (orientacacion == horizontal) {
        for(int i=0; i<tamanio<;i++) {
            if(posicion[filas][columnas +i] !='~') {    // comprobar si el barco completo entra en un espacio en donde solo hay agua
                return 0;
            }
            if(filas>0 && posicion[filas-1][columnas +i] !='~') {    // comprobar los cuadrantes de arriba del barco
                return 0;
            }
            if(filas < FIL -1 && posicion[filas+1][columnas +i] !='~') {       // comprobar los cuadrantes de abajo del barco
                return 0;
            }
            if(columas > 0 && posicion[filas][columnas - 1] !='~'){           // comprobar el cuadrante anterior inmediato a la izquierda
                return 0;
            }
            if(columnas + tamanio < COL &&  posicion[filas][columnas + tamanio] != '~') {    // Siempre que esté por dentro del rango, comprobar el cuadrante inmediato a la derecha
                return 0:
            }
        }
    } else { 
         for(int i=0; i<tamanio<;i++) {
            if(posicion[filas + i][columnas] !='~') {    // comprobar si el barco completo entra en un espacio en donde solo hay agua
                return 0;
            }
            if(columnas > 0 && posicion[filas + i ][columnas - 1] !='~') {    // comprobar los cuadrantes a la derecha
                return 0;
            }
            if(columnas < COL -1 && posicion[filas+i][columnas +1] !='~') {       // comprobar los cuadrantes a la izquierda
                return 0;
            }
            if(filas > 0 && posicion[filas -1][columnas ] !='~'){           // comprobar el cuadrante anterior inmediato 
                return 0;
            }
            if(filas + tamanio < FIL &&  posicion[filas + tamanio][columnas] != '~') {    // Siempre que esté por dentro del rango, comprobar el cuadrante  posterior inmediato 
                return 0:
            }
         }
         return 1; // no hay impedimentos, en el lugar donde va el barco y sus alrededores está vacio, apto para ingresarlo al tablero.
    }
}

// para cambiar ~ (agua) por # (barco)
void ingresar_barco(int filas, int columnas, Orientacion orientacion, Tamanio tamanio, char posicion[FIL][COL]){
    for(int i =0; i<tamanio<i++){
        if(orientacion==horizontal) {
            posicion[filas][columnas+i]='#';
        } else {
            posicion[filas+i][columnas]='#';
        }
    }
}


// muestra los dos tableros, el del barcos del jugador de turno y el tablero de disparos al oponente
void mostrar_tablero(char posicion[FIL][COL], char posicion_disparos[FIL][COL]) {
    for(int k=0; k<COL;k++) {
        printf("%c ",'A'+k);    // letras de las columnas
        printf("\n");
    }
    for(int i=0; i<FIL;i++) {
        printf("%d",i+1);      // n° de las filas
        for(int k=0; k<COL;k++) {
            printf("%c",posicion[i][k]);
        }
    }
}

