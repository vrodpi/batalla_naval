#include "../include/tablero.h"


// Completar ambos Tableros con AGUA para empezar desde cero
void inicializar_tablero (char tablero[FIL][COL], char tablero_disparos[FIL][COL]) {
    for (int i = 0; i < FIL; i++){    
        for (int j = 0; j < COL; j++){  
            tablero[i][j] = CASILLA_AGUA;
            tablero_disparos[i][j] = CASILLA_AGUA;
        }
    }
}


// Revisar si hay lugares disponibles alrededor y dentro del tablero para poner el barco.
int libre(Barco barco, char tablero[FIL][COL]) {
    int alto;
    int ancho;
    
    // Calcula cuanto ocupa el barco dependiendo si esta parado o acostado.
    if (barco.orientacion==horizontal) {
        alto=1;    
        ancho=barco.tamanio;
             
    } else if (barco.orientacion==vertical) {
        ancho=1;        
        alto=barco.tamanio;
    }


    // Revisa toda la zona que rodea al barco (incluidas diagonales) para dejar margen y no chocar con otro.
    for (int f = barco.fila - 1; f <= barco.fila + alto; f++) {   
        for (int c = barco.columna - 1; c <= barco.columna + ancho; c++) {        
            // Si la casilla está dentro del tablero y no es agua, significa que hay algo cerca que estorba.
            if (f >= 0 && f < FIL && c >= 0 && c < COL && tablero[f][c] != CASILLA_AGUA) {     
                return -2;    // entonces el espacio no está libre por cercanía/superposición
            }
        }
    }

    // Revisa que el barco no se salga de los bordes del tablero.
    if ((barco.fila + alto) > FIL || (barco.columna + ancho) > COL){
        return -1;          // No entra en el tablero.
    }
       
    return 1;           // Se puede ingresar el barco
}




// Marca en el tablero las casillas que ocupa el barco cambiando agua (~) por barco (#).
void ingresar_barco(Barco barco, char tablero[FIL][COL]){
   
    for(int i =0; i < barco.tamanio; i++){                 
        
        if(barco.orientacion == horizontal) {          
            tablero[barco.fila][barco.columna + i] = CASILLA_BARCO;   
            // Avanza hacia la derecha manteniendo la misma fila.
        } else {
            // Avanza hacia abajo manteniendo la misma columna.
            tablero[barco.fila + i][barco.columna] = CASILLA_BARCO;
        }
    } 
}


// Función principal para intentar colocar un barco: primero chequea si es válido y, si lo es, lo coloca.
int validar_ingreso_barco (Barco barco, char tablero[FIL][COL]) {
        int resultado = libre(barco, tablero); 
        if (resultado == 1) {

            ingresar_barco(barco,tablero);

            return 1;   // Colocado con exito.
        }
    return resultado;   // Devuelve el código de error (-1 o -2) si no se pudo poner.
}
