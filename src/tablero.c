#include "../include/tablero.h"


// Completar ambos Tableros con AGUA
void inicializar_tablero (char tablero[FIL][COL], char tablero_disparos[FIL][COL]) {
    for (int i = 0; i < FIL; i++){    // recorre las filas
        for (int j = 0; j < COL; j++){  // recorre las columnas
            tablero[i][j] = CASILLA_AGUA;
            tablero_disparos[i][j] = CASILLA_AGUA;
        }
    }
}


// determinar si el barco entra en el tablero
int libre(Barco barco, char tablero[FIL][COL]) {
    int alto;
    int ancho;
    
    if (barco.orientacion==horizontal) {
        alto=1;    // barco horizontal, ocupa una fila
        ancho=barco.tamanio;
             
    } else if (barco.orientacion==vertical) {
        ancho=1;        // barco vertical, ocupa una columna
        alto=barco.tamanio;
    }


    // para revisar el entorno (incluso diagonales), para evitar que se superpongan.
    for (int f = barco.fila - 1; f <= barco.fila + alto; f++) {   /// para recorrer la fila de abajo y de arriba del barco                          ~ ~ ~
        for (int c = barco.columna - 1; c <= barco.columna + ancho; c++) {        // para recorrer la columna a su izquierda y derecha              ~ # ~
            if (f >= 0 && f < FIL && c >= 0 && c < COL && tablero[f][c] != CASILLA_AGUA) {      // si las posiciones son distintos de agua                   ~ ~ ~
                return -2;                                                                       // entonces el espacio no está libre
            }
        }
    }

    // revisa que el barco entre completo en el tablero
    if ((barco.fila + alto) > FIL || (barco.columna + ancho) > COL){
        return -1;
    }
       
    return 1;           // espacio libre para ingresar el barco
}




// para cambiar ~ (agua) por # (barco)
void ingresar_barco(Barco barco, char tablero[FIL][COL]){
   

    for(int i =0; i < barco.tamanio; i++){                 // recorrer desde cero hasta el tamaño del barco
        
        if(barco.orientacion == horizontal) {            // si es horizontal entonces...
            tablero[barco.fila][barco.columna + i] = CASILLA_BARCO;   // dichas posiciones cambairlas por #
        } else {
            tablero[barco.fila + i][barco.columna] = CASILLA_BARCO;
        }
    } 
}



int validar_ingreso_barco (Barco barco, char tablero[FIL][COL]) {
        // El barco entra completo? No hay superposición entre ellos? Existe un espacio entre cada uno?
        int resultado = libre(barco, tablero); 
        if (resultado == 1) {

            ingresar_barco(barco,tablero);

            return 1;
        }
    return resultado;
}


// EN MAIN. 
// si libre dio cero, con un print: Posición Invalida, Los barcos no pueden Superponerse. Recuerde dejar un casillero de       espacio entre ellos. 
/// posicion invalida.El Barco por sus dimensiones no entra en el Tablero.