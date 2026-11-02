

#include "tablero.h"
// Que los tableros solo contengan agua
void inicializar_tablero (char tablero[FIL][COL], char tablero_disparos[FIL][COL]) {
    for (int i = 0; i < FIL; i++){    // recorre las filas
        for (int j = 0; j < COL; j++){  // recorre las columnas
            tablero[i][j] = '~';
            tablero_disparos[i][j] = '~';
        }
    }
}






// determinar si el barco entra en el tablero
int libre(Barco barco, char tablero[FIL][COL]) {
    int alto;
    int ancho;

    // analizar la orientacion, para corroborar que el barco entre completo
    if (barco.orientacion==horizontal) {
        alto=1;    // barco horizontal, ocupa una fila
        ancho=barco.tamanio;
    } else {
        ancho=1;        // barco vvertical, ocupa una columna
        alto=barco.tamanio;
    } else {
        return 0;
    }

        // Verificar que las coordenadas sean válidas
    if (barco.fila < 0 || barco.fila >= FIL || barco.columna < 0 || barco.columna >= COL) {
        return 0;
    }

    // que no supere el maximo de fila y columna, sino no puede ingresarse
    if (barco.fila + alto > FIL || barco.columna + ancho > COL) {
        return 0;
    }

    // para revisar el entorno (incluso diagonales)
    for (int f = barco.fila - 1; f <= barco.fila + alto; f++) {   /// para recorrer la fila de arriba y de abajo del barco
        for (int c = barco.columna - 1; c <= barco.columna + ancho; c++) {        // para recorrer la columna a su izquierda y derecha
            if (f >= 0 && f < FIL && c >= 0 && c < COL && tablero[f][c] != '~') {      // si las posiciones son distintos de agua 
                return 0;                                                                       // entonces el espacio no está libre
            }
        }
    }
    // retornar 1 si hay espacio para que el barc entre
    return 1;
}



// para cambiar ~ (agua) por # (barco)
void ingresar_barco(Barco barco, char tablero[FIL][COL]){
   

    for(int i =0; i<barco.tamanio;i++){
        if(barco.orientacion ==horizontal) {
            tablero[barco.fila][barco.columna+i]='#';
        } else {
            tablero[barco.fila+i][barco.columna]='#';
        }
    }
}


void mostrar_tablero(char tablero[FIL][COL], char tablero_disparos[FIL][COL]) {
    printf("\n---MI FLOTA---\n   ");
    for (int k = 0; k < COL; k++) {
        printf("%d ", k + 1);
    }
    printf("\n");
    for (int i = 0; i < FIL; i++) {
        printf("%2d ", i + 1);
        for (int k = 0; k < COL; k++) {
            printf("%c ", tablero[i][k]);
        }
        printf("\n");
    }

    printf("\n---DISPAROS A FLOTA ENEMIGA---\n   ");
    for (int k = 0; k < COL; k++) {
        printf("%d ", k + 1);
    }
    printf("\n");

    for (int i = 0; i < FIL; i++) {
        printf("%2d ", i + 1);
        for (int k = 0; k < COL; k++) {
            printf("%c ", tablero_disparos[i][k]);
        }
        printf("\n");
    }
}


#include "gam.h"
#include "tab.h"
void inicializar_partida( Partida *partida, Modo modo) {
      
   partida->turno=1;
   partida->ganador=0;
   partida->victoria=0;
   partida->cantidad_turnos=0;
   partida->modo_de_juego=modo;

     // Inicializar los barcos vivos
    partida->jugador1.barcos_vivos = BARCOS_MODO_NORMAL;        // cada jugador empieza con 5 barcos
    partida->jugador2.barcos_vivos = BARCOS_MODO_NORMAL;

    // Inicializar los impactos
    partida->jugador1.hits = 0;                 // cada jugador empieza con 0 impacto a barcos
    partida->jugador2.hits = 0;

   inicializar_tablero(partida->jugador1.tableros.tablero, partida->jugador1.tableros.tablero_disparos);
   inicializar_tablero(partida->jugador2.tableros.tablero,partida->jugador2.tableros.tablero_disparos);
}



Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo, char tablero[FIL][COL]) {
     
      // verificar  que las coordenadas sean válidas
    if (fila_disparo < 0 || fila_disparo >= FIL || columna_disparo < 0 || columna_disparo >= COL) {
        printf("COORDENADA INVALIDA \n");
        return disp_NO_valido;
       
    }


    // En caso de que haya AGUA ~
    if( tablero[fila_disparo][columna_disparo]=='~') {

        tablero[fila_disparo][columna_disparo]='/';      // se disparó a agua
        printf("AGUA\n");

        return disp_agua;
    // si hay un BARCO...
    } else if(tablero[fila_disparo][columna_disparo]=='#') {     // se disparo a un barco

        tablero[fila_disparo][columna_disparo]='X';
        printf("IMPACTO\n");

        return disp_impacto; 

    } else if (tablero[fila_disparo][columna_disparo]=='/' || tablero[fila_disparo][columna_disparo]=='X') {

        printf("UBICACIÓN REPETIDA\n");

        return disp_repetido;
    }
    return disp_NO_valido;
}



void barco_hundido(Jugador *defensor) {

    for (int i = 0; i < BARCOS_MODO_NORMAL; i++) {          // 5 barcos  por jugador

       
        if (defensor->barco[i].hundido == 0) {   // para verificiar el estado de todos los barcos que no han sido hundidos

            int hundido = 1;   // asumimos que el barco fue hundido (verdadero)

          
            for (int j = 0; j < defensor->barco[i].tamanio; j++) {        // recorrer las posiciones de los barcos (depende de sy tamanio)

                int fila = defensor->barco[i].fila; 
                int columna = defensor->barco[i].columna;

                if (defensor->barco[i].orientacion == horizontal) {
                    columna += j;
                } else {
                    fila += j;
                }

                // si hay alguna Posición sin impacto
                if (defensor->tableros.tablero[fila][columna] != 'X') {
                    hundido = 0;
                }
            }

        
            if (hundido == 1) {         // si realmente el barco fue hindido

                defensor->barco[i].hundido = 1;

                defensor->barcos_vivos--;       // reducir el numero de barcos vivos

                printf("\n BARCO HUNDIDO!!\n");
            }
        }
    }
}

void coordenadas_de_ataque(Jugador *atacante, Jugador *defensor) {     // si juega el jugador1 (atacante) el defensor es el jugador2
    int fila, columna;
    Rdo_Disparo rdo= disp_NO_valido;           // resultado, del retorno.
    
    while (rdo == disp_NO_valido || rdo == disp_repetido) {

        printf("\n INGRESE COORDENADAS DE ATAQUE:\n");

        printf("Fila (1 a %d): ", FIL);
        scanf("%d", &fila);
        getchar();

        printf("Columna (1 a %d): ", COL);
        scanf("%d", &columna);
        getchar();

        rdo=recibir_disparo(fila-1, columna -1, defensor->tableros.tablero); // los arreglos empiezan desde 0, no desde 1, por eso restamos (fila va de 1 a 10; pero en arreglo es del 0 al 9)
        // Si está dentro de los límites...
        if(rdo==disp_agua) {

            atacante->tableros.tablero_disparos[fila-1][columna-1]='/';

        } else if (rdo == disp_impacto) {

                atacante->tableros.tablero_disparos[fila-1][columna-1] = 'X';

                atacante->hits++;

                barcos_hundido(defensor);
                }

    } 
}



// para definir de quien es el turno
void jugar(Partida *partida){

    Jugador *atacante, *defensor; 

    while (fin_juego(partida)==0) {     // mientras que aun el juego no haya terminado hacer...

        printf("TURNO NUMERO %d\n", partida->cantidad_turnos + 1);

        if (partida->turno==1) {
            atacante= &partida->jugador1;
            defensor= &partida->jugador2;
        }  else {
            atacante= &partida->jugador2;
            defensor= &partida->jugador1;
        }

        
            printf("Turno de: %s\n", atacante->nombres);
            
            // mostrar los tableros del jugador actual
            mostrar_tablero(atacante->tableros.tablero, atacante->tableros.tablero_disparos);
        
             // El atacante realiza su disparo
        coordenadas_de_ataque(atacante, defensor);

        // Cambiamos el turno
        if (partida->turno == 1) {
            partida->turno = 2;
        } else {
            partida->turno = 1;
        }

        partida->cantidad_turnos++;
    }

    // Fin de la partida
    printf("\n=============================\n");
    printf("        ¡FIN DEL JUEGO!      \n");
    printf("=============================\n");

    if (partida->ganador == 1) {

        printf("¡%s! VICTORIA, PARTIDA GANADA.\n",
               partida->jugador1.nombres);

    } else if (partida->ganador == 2) {

        printf("¡%s! VICTORIA, PARTIDA GANADA.\n",
               partida->jugador2.nombres);
    }
}

// para verificar que un jugador se quedó sin barcos vivos
int fin_juego(Partida *partida) {

    if(partida->jugador1.barcos_vivos <=0 ) {       // si el n° de barcos vivos  del jugador1 es menor igual a cero entonces
        partida->ganador=2;                         // el ganador de la partida es el jugador 2
        return 1;                                   // retorna 1, indicando que hay ganador
    } else if(partida->jugador2.barcos_vivos <=0) {    
        partida->ganador=1;                          // el ganador será el jugador1 si el el jugador2 se quedó sin barcos vivos
        return 1;
    }
    return 0; // indica que el juego continua
}

