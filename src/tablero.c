

#include "../include/tablero.h"
#include <stdio.h>




// Completar ambos Tableros con AGUA
void inicializar_tablero (char tablero[FIL][COL], char tablero_disparos[FIL][COL]) {
    for (int i = 0; i < FIL; i++){    // recorre las filas
        for (int j = 0; j < COL; j++){  // recorre las columnas
            tablero[i][j] = '~';
            tablero_disparos[i][j] = '~';
        }
    }
}




//  Verificar que las coordenadas estén dentro del rango y que de acuerdo al tamaño del barco, el mismo pueda ingresar de forma entera.
int validar_rango (Barco barco) {

    int alto;
    int ancho;

    // analizar la orientacion, para corroborar que el barco entre completo
    if (barco.orientacion==horizontal) {
        alto=1;    // barco horizontal, ocupa una fila
        ancho=barco.tamanio;
    } else if (barco.orientacion==vertical) {
        ancho=1;        // barco vertical, ocupa una columna
        alto=barco.tamanio;
    } else {
        return 0;   // orientacion invalida
    }   


         // Verificar que las coordenadas sean válidas
    if (barco.fila < 0 || barco.fila >= FIL || barco.columna < 0 || barco.columna >= COL) { 
        return 0;
    }

    // Verificar que el barco por su tamaño no se salga del tablero
    if (barco.fila + alto > FIL || barco.columna + ancho > COL) {
        return 0;
    }

    return 1;
}



// determinar si el barco entra en el tablero
int libre(Barco barco, char tablero[FIL][COL]) {
    int alto;
    int ancho;

    // analizar la orientacion, para corroborar que el barco entre completo
    if (barco.orientacion==horizontal) {
        alto=1;    // barco horizontal, ocupa una fila
        ancho=barco.tamanio;
    } else if (barco.orientacion==vertical) {
        ancho=1;        // barco vertical, ocupa una columna
        alto=barco.tamanio;
    } else {
        return 0;   // orientacion invalida
    }   


    // para revisar el entorno (incluso diagonales), para evitar que se superpongan.
    for (int f = barco.fila - 1; f <= barco.fila + alto; f++) {   /// para recorrer la fila de abajo y de arriba del barco                          ~ ~ ~
        for (int c = barco.columna - 1; c <= barco.columna + ancho; c++) {        // para recorrer la columna a su izquierda y derecha              ~ # ~
            if (f >= 0 && f < FIL && c >= 0 && c < COL && tablero[f][c] != '~') {      // si las posiciones son distintos de agua                   ~ ~ ~
                return 0;                                                                       // entonces el espacio no está libre
            }
        }
    }
   
    return 1;           // espacio libre para ingresar el barco
}




// para cambiar ~ (agua) por # (barco)
void ingresar_barco(Barco barco, char tablero[FIL][COL]){
   

    for(int i =0; i<barco.tamanio;i++){                 // recorrer desde cero hasta el tamaño del barco
        if(barco.orientacion ==horizontal) {            // si es horizontal entonces...
            tablero[barco.fila][barco.columna+i]='#';   // dichas posiciones cambairlas por #
        } else {
            tablero[barco.fila+i][barco.columna]='#';
        }
    } 
}



int validar_ingreso_barco (Barco barco, char tablero[FIL][COL]) {
    // coordenadas válidas? el barco entra completo?
    if (validar_rango(barco)==1) {
        // posiciones disponibles ?
        if (libre(barco, tablero)==1) {

            ingresar_barco(barco,tablero);
            return 1;
        }
    }
    return 0;
}

// 
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]) {
     
    // verificar  que las coordenadas sean válidas
    if (fila_disparo < 0 || fila_disparo >= FIL || columna_disparo < 0 || columna_disparo >= COL) {
        // printf("COORDENADA INVALIDA \n");
        return disp_NO_valido;
       
    }


    // En caso de que haya AGUA ~
    if (tablero[fila_disparo][columna_disparo]=='~') {

        tablero[fila_disparo][columna_disparo]='/';      // se disparó a agua
        // printf("AGUA\n");

        return disp_agua;

    // si hay un BARCO...
    } else if(tablero[fila_disparo][columna_disparo]=='#') {     // se disparo a un barco

        tablero[fila_disparo][columna_disparo]='X';
        // printf("IMPACTO\n");

        return disp_impacto; 

    } else if (tablero[fila_disparo][columna_disparo]=='/' || tablero[fila_disparo][columna_disparo]=='X') {

        // printf("UBICACIÓN REPETIDA\n");

        return disp_repetido;
    }
    return disp_NO_valido;
}

