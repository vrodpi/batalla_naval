/*
 * test_tablero.c - Pruebas automaticas del modulo tablero.
 *
 * ---------------------------------------------------------------------------
 * ESTRATEGIA
 * ---------------------------------------------------------------------------
 * Igual que test_ranking.c: sin frameworks externos, un contador de
 * aserciones y una funcion chequear() que imprime "ok" o "FALLO".
 *
 * Se prueban las reglas puras del tablero, sin pedir datos por teclado:
 *   - inicializacion de las dos matrices con agua;
 *   - limites del tablero (rango) en horizontal y vertical;
 *   - deteccion de superposicion y de barcos que se tocan;
 *   - marcado de las casillas con '#' (ingresar_barco);
 *   - validacion defensiva: una colocacion invalida no modifica nada.
 *
 * El dibujo por pantalla (mostrar_tablero) se verifica visualmente al
 * ejecutar la app; no se captura stdout para no complicar la suite.
 *
 * Compilar: gcc -std=c99 -pedantic -Wall -Wextra -Iinclude \
 *               tests/test_tablero.c src/tablero.c -o build/test_tablero
 * Ejecutar desde la raiz: ./build/test_tablero
 * Devuelve 0 si todo pasa, 1 si algo falla.
 */

#include <stdio.h>
#include <string.h>

#include "tablero.h"
#include "config.h"

static int pruebas = 0;
static int fallos = 0;

/* Registra una asercion y muestra el nombre del caso. */
static void chequear(int condicion, const char *nombre)
{
    pruebas++;
    if (condicion) {
        printf("ok   - %s\n", nombre);
    } else {
        printf("FALLO- %s\n", nombre);
        fallos++;
    }
}

/*
 * prueba_inicializacion: despues de inicializar_tablero(), las dos
 * matrices deben tener '~' en todas sus casillas.
 */
static void prueba_inicializacion(void)
{
    char posicion[CONFIG_FILAS][CONFIG_COLUMNAS];
    char disparos[CONFIG_FILAS][CONFIG_COLUMNAS];
    int i;
    int j;
    int todo_agua;

    inicializar_tablero(posicion, disparos);
    todo_agua = 1;
    for (i = 0; i < CONFIG_FILAS; i++) {
        for (j = 0; j < CONFIG_COLUMNAS; j++) {
            if (posicion[i][j] != (char)agua
                || disparos[i][j] != (char)agua) {
                todo_agua = 0;
            }
        }
    }
    chequear(todo_agua == 1, "inicializar deja los dos tableros en agua");
}

/*
 * prueba_rango: casos de borde de la validacion de limites.
 * Recordar que las filas/columnas internas van de 0 a 9.
 */
static void prueba_rango(void)
{
    chequear(rango(0, 0, horizontal, porta_aviones) == 1,
             "horizontal 5 entra desde (0,0)");
    chequear(rango(0, 0, vertical, porta_aviones) == 1,
             "vertical 5 entra desde (0,0)");
    chequear(rango(0, 6, horizontal, porta_aviones) == 0,
             "horizontal 5 no entra desde columna 6");
    chequear(rango(6, 0, vertical, porta_aviones) == 0,
             "vertical 5 no entra desde fila 6");
    chequear(rango(9, 9, horizontal, velero) == 1,
             "barco de 1 entra en la ultima casilla");
    chequear(rango(9, 9, horizontal, lancha) == 0,
             "barco de 2 no entra en la ultima casilla");
    chequear(rango(9, 0, vertical, velero) == 1,
             "vertical 1 entra en la ultima fila");
    chequear(rango(9, 0, vertical, lancha) == 0,
             "vertical 2 no entra en la ultima fila");
    chequear(rango(-1, 0, horizontal, velero) == 0,
             "fila negativa invalida");
    chequear(rango(0, -1, horizontal, velero) == 0,
             "columna negativa invalida");
    chequear(rango(CONFIG_FILAS, 0, horizontal, velero) == 0,
             "fila fuera del tablero invalida");
    chequear(rango(0, CONFIG_COLUMNAS, horizontal, velero) == 0,
             "columna fuera del tablero invalida");
    chequear(rango(0, 0, (Orientacion)99, velero) == 0,
             "orientacion desconocida invalida");
    chequear(rango(0, 0, horizontal, (Tamanio)0) == 0,
             "tamanio cero invalido");
    chequear(rango(5, 0, vertical, porta_aviones) == 1,
             "vertical 5 entra justo hasta la fila 9");
    chequear(rango(6, 0, vertical, porta_aviones) == 0,
             "vertical 5 se pasa de la fila 9");
}

/*
 * prueba_libre: superposicion y regla de separacion.
 *
 * Se coloca un portaaviones horizontal en (0,0), que ocupa las columnas
 * 0..4 de la fila 0. A partir de ahi:
 *   - otro barco en la misma fila choca;
 *   - uno en la fila 1 toca el barco (la regla actual exige un agua);
 *   - uno en la fila 2 esta separado por una fila de agua y es valido;
 *   - uno en diagonal a la popa (1,5) tambien toca y se rechaza.
 */
static void prueba_libre(void)
{
    char posicion[CONFIG_FILAS][CONFIG_COLUMNAS];
    char disparos[CONFIG_FILAS][CONFIG_COLUMNAS];

    inicializar_tablero(posicion, disparos);
    chequear(libre(0, 0, horizontal, porta_aviones, posicion) == 1,
             "tablero vacio: el primer barco entra");

    ingresar_barco(0, 0, horizontal, porta_aviones, posicion);
    chequear(posicion[0][0] == (char)barco
             && posicion[0][4] == (char)barco
             && posicion[0][5] == (char)agua,
             "ingresar_barco marca exactamente 5 casillas");

    chequear(libre(0, 0, horizontal, lancha, posicion) == 0,
             "no se superpone con el barco existente");
    chequear(libre(1, 0, horizontal, lancha, posicion) == 0,
             "no se toca el barco por abajo");
    chequear(libre(2, 0, horizontal, lancha, posicion) == 1,
             "con una fila de agua en medio si se puede");
    chequear(libre(1, 5, vertical, velero, posicion) == 0,
             "no se toca el barco en diagonal");
    chequear(libre(0, 5, horizontal, velero, posicion) == 0,
             "no se toca el barco por la popa");
}

/*
 * prueba_ingresar: marcado vertical y horizontal, y verificacion de que
 * una colocacion fuera de rango no modifica el tablero.
 */
static void prueba_ingresar(void)
{
    char posicion[CONFIG_FILAS][CONFIG_COLUMNAS];
    char disparos[CONFIG_FILAS][CONFIG_COLUMNAS];
    char copia[CONFIG_FILAS][CONFIG_COLUMNAS];

    inicializar_tablero(posicion, disparos);
    ingresar_barco(4, 1, vertical, buque, posicion);
    chequear(posicion[4][1] == (char)barco
             && posicion[7][1] == (char)barco
             && posicion[8][1] == (char)agua,
             "ingresar_barco vertical marca 4 casillas");

    memcpy(copia, posicion, sizeof(posicion));
    ingresar_barco(-1, 0, horizontal, velero, posicion);
    ingresar_barco(6, 0, vertical, porta_aviones, posicion);
    chequear(memcmp(copia, posicion, sizeof(posicion)) == 0,
             "colocacion invalida no modifica el tablero");
}

int main(void)
{
    printf("== test_tablero ==\n");
    prueba_inicializacion();
    prueba_rango();
    prueba_libre();
    prueba_ingresar();
    printf("------------------\n");
    printf("Pruebas: %d, Fallos: %d\n", pruebas, fallos);
    if (fallos == 0) {
        printf("TODO OK\n");
        return 0;
    }
    printf("HAY FALLOS\n");
    return 1;
}
