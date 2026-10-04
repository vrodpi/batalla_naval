#include <stdio.h>

#include "../include/TABLERO.H"
 
#define fil=10;
#define col=10;


typedef enum {
    agua;                   //    ~
    barco;                  //    ^
    impacto;                //    X
    disparo_a_agua;         //    /
} Estado;


typedef enum {
    horizontal,
    vertical,
} Orientacion;

typedef enum {
    porta_aviones=5,
    buque=4,
    submarino=3,
    lancha=2,
    velero=1,
} Tamanio;

Tamanio tamanio;

typedef struct {
    Tamanio tamanio;
    Orientacion orientacion;
    int filas;
    char columnas;
    Estado estado;
}Barco;



typedef struct {
    Estado posicion[fil][col];
} Tablero;


char posicion[fil][col];
char posicion_disparos[fil][col];

void ingresar_barcos(char posicion[fil][col]); 

void motrar_tablero(char posicioin[fil][col]);

int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio);

void inicializar_tablero (char posicion[fil][col], char posicion_disparos[fil][col]){
    for (int i = 0; i < fil; i++){
        for (int j = 0; j < col; j++){
            posicion[i][j] = 'agua';
            posicion_disparos[i][j] = 'agua';
        }
    }
}

// para determinar si la coordenada ingresada es válida (dentro de la función); 
int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio) {
    // filas
    for (int i=0; i<fil, i++) {
        if(filas>=0 && fila<=fil && columnas>=0 && columnas<=col) {   // coordenada válida?  (dentro de la función); 
            if(Orientacion=="Horizontal") {                             // si es horizontal
                if(col + tamanio<10) {        // si coordenada columna + tamaño es menor a 10, entonces el barco entra completo, es válido.  
                    return 1; // indica que esta dentro del rango. Coordenada válida
                } 
            } else if (Orientacion=="Vertical") {
                if(fil + tamanio<10) {        // si coordenada columna + tamaño es menor a 10, entonces el barco entra completo, es válido.  
                    return 1; // indica que esta dentro del rango. Coordenada válida
                } 
            }

        }
             
    }
    return 0; 
}

void mostrar_tablero(char posicion[fil][col], char posicion_disparos[fil][col]) {
    for(int i=0; i<fil;i++) {
        printf("%d",i+1);
        for(int k=0; k<col;k++) {
            printf("%c ",'A'+i);
            printf("%c",posicion[fil][col]);
        }
    }
}

