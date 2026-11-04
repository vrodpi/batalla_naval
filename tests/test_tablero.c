/**
 * @file test_tablero.c
 * @brief Pruebas automaticas del modulo tablero.
 *
 * Estrategia: sin frameworks externos, un contador de aserciones y una
 * funcion chequear() que imprime "ok" o "FALLO".
 *
 * @note Se prueban las reglas puras del tablero, sin pedir datos por teclado:
 *       inicializacion, limites, superposicion, contacto, marcado y validacion
 *       defensiva. El dibujo por pantalla se verifica visualmente al ejecutar
 *       la app; no se captura stdout para no complicar la suite.
 * @note Compilar: gcc -std=c99 -pedantic -Wall -Wextra -Iinclude
 *       tests/test_tablero.c src/tablero.c -o build/test_tablero.
 *       Ejecutar desde la raiz; devuelve 0 si todo pasa, 1 si algo falla.
 */

#include <stdio.h>
#include <string.h>

#include "tablero.h"
#include "config.h"

/**
 * @brief Cantidad total de aserciones ejecutadas por la suite.
 */
static int pruebas = 0;

/**
 * @brief Cantidad de aserciones que fallaron.
 */
static int fallos = 0;

/**
 * @brief Registra una asercion y muestra el nombre del caso.
 *
 * @param condicion Resultado de la condicion verificada.
 * @param nombre Texto descriptivo del caso.
 * @return No devuelve valor.
 * @note Actualiza los contadores globales pruebas y fallos.
 */
static void chequear(int condicion, const char *nombre)
{
    /* Toda asercion ejecutada cuenta. */
    pruebas++;
    if (condicion) {
        printf("ok   - %s\n", nombre);
    } else {
        /* Un fallo se muestra y se acumula para el codigo de salida. */
        printf("FALLO- %s\n", nombre);
        fallos++;
    }
}

/**
 * @brief Comprueba que inicializar_tablero deje todo en agua.
 *
 * @return No devuelve valor.
 * @note Despues de inicializar_tablero(), las dos matrices deben tener '~' en
 *       todas sus casillas.
 */
static void prueba_inicializacion(void)
{
    char posicion[CONFIG_FILAS][CONFIG_COLUMNAS]; /* Tablero propio. */
    char disparos[CONFIG_FILAS][CONFIG_COLUMNAS]; /* Tablero de disparos. */
    int i;                                        /* Fila. */
    int j;                                        /* Columna. */
    int todo_agua;                                /* Bandera de resultado. */

    /* Inicializar y asumir que todo esta bien hasta encontrar lo contrario. */
    inicializar_tablero(posicion, disparos);
    todo_agua = 1;

    /* Recorrer ambas matrices completas. */
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

/**
 * @brief Prueba los casos de borde de la validacion de limites.
 *
 * @return No devuelve valor.
 * @note Las filas y columnas internas van de 0 a 9. Se cubren horizontal y
 *       vertical, ultima fila/columna, coordenadas negativas, orientacion
 *       desconocida y tamanio cero.
 */
static void prueba_rango(void)
{
    /* Primer bloque: el barco entra desde el origen en ambas orientaciones. */
    chequear(rango(0, 0, horizontal, porta_aviones) == 1,
             "horizontal 5 entra desde (0,0)");
    chequear(rango(0, 0, vertical, porta_aviones) == 1,
             "vertical 5 entra desde (0,0)");
    chequear(rango(0, 6, horizontal, porta_aviones) == 0,
             "horizontal 5 no entra desde columna 6");
    chequear(rango(6, 0, vertical, porta_aviones) == 0,
             "vertical 5 no entra desde fila 6");

    /* Ultima fila y ultima columna. */
    chequear(rango(9, 9, horizontal, velero) == 1,
             "barco de 1 entra en la ultima casilla");
    chequear(rango(9, 9, horizontal, lancha) == 0,
             "barco de 2 no entra en la ultima casilla");
    chequear(rango(9, 0, vertical, velero) == 1,
             "vertical 1 entra en la ultima fila");
    chequear(rango(9, 0, vertical, lancha) == 0,
             "vertical 2 no entra en la ultima fila");

    /* Coordenadas negativas y fuera del tablero. */
    chequear(rango(-1, 0, horizontal, velero) == 0,
             "fila negativa invalida");
    chequear(rango(0, -1, horizontal, velero) == 0,
             "columna negativa invalida");
    chequear(rango(CONFIG_FILAS, 0, horizontal, velero) == 0,
             "fila fuera del tablero invalida");
    chequear(rango(0, CONFIG_COLUMNAS, horizontal, velero) == 0,
             "columna fuera del tablero invalida");

    /* Parametros invalidos y borde exacto de la ultima fila. */
    chequear(rango(0, 0, (Orientacion)99, velero) == 0,
             "orientacion desconocida invalida");
    chequear(rango(0, 0, horizontal, (Tamanio)0) == 0,
             "tamanio cero invalido");
    chequear(rango(5, 0, vertical, porta_aviones) == 1,
             "vertical 5 entra justo hasta la fila 9");
    chequear(rango(6, 0, vertical, porta_aviones) == 0,
             "vertical 5 se pasa de la fila 9");
}

/**
 * @brief Prueba superposicion y la regla de separacion entre barcos.
 *
 * @return No devuelve valor.
 * @note Se coloca un portaaviones horizontal en (0,0), que ocupa las columnas
 *       0..4 de la fila 0. Otro barco en la misma fila choca; uno en la fila 1
 *       toca el barco; uno en la fila 2 esta separado por agua y es valido; uno
 *       en diagonal a la popa (1,5) tambien toca y se rechaza.
 */
static void prueba_libre(void)
{
    char posicion[CONFIG_FILAS][CONFIG_COLUMNAS]; /* Tablero propio. */
    char disparos[CONFIG_FILAS][CONFIG_COLUMNAS]; /* Tablero de disparos. */

    /* Tablero vacio: el primer barco siempre entra. */
    inicializar_tablero(posicion, disparos);
    chequear(libre(0, 0, horizontal, porta_aviones, posicion) == 1,
             "tablero vacio: el primer barco entra");

    /* Colocar el portaaviones y comprobar sus extremos. */
    ingresar_barco(0, 0, horizontal, porta_aviones, posicion);
    chequear(posicion[0][0] == (char)barco
             && posicion[0][4] == (char)barco
             && posicion[0][5] == (char)agua,
             "ingresar_barco marca exactamente 5 casillas");

    /* Superposicion, contacto lateral, separacion y diagonal. */
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

/**
 * @brief Prueba el marcado vertical/horizontal y la validacion defensiva.
 *
 * @return No devuelve valor.
 * @note Verifica que una colocacion fuera de rango no modifique el tablero.
 */
static void prueba_ingresar(void)
{
    char posicion[CONFIG_FILAS][CONFIG_COLUMNAS]; /* Tablero propio. */
    char disparos[CONFIG_FILAS][CONFIG_COLUMNAS]; /* Tablero de disparos. */
    char copia[CONFIG_FILAS][CONFIG_COLUMNAS];    /* Respaldo para comparar. */

    /* Marcado vertical de un buque de longitud 4. */
    inicializar_tablero(posicion, disparos);
    ingresar_barco(4, 1, vertical, buque, posicion);
    chequear(posicion[4][1] == (char)barco
             && posicion[7][1] == (char)barco
             && posicion[8][1] == (char)agua,
             "ingresar_barco vertical marca 4 casillas");

    /* Una colocacion invalida no debe cambiar ni una casilla. */
    memcpy(copia, posicion, sizeof(posicion));
    ingresar_barco(-1, 0, horizontal, velero, posicion);
    ingresar_barco(6, 0, vertical, porta_aviones, posicion);
    chequear(memcmp(copia, posicion, sizeof(posicion)) == 0,
             "colocacion invalida no modifica el tablero");
}

/**
 * @brief Ejecuta todas las baterias del tablero y resume el resultado.
 *
 * @return 0 si todas las aserciones pasaron; 1 si hubo al menos un fallo.
 * @note El codigo de salida permite que `make test-tablero` detecte el
 *       resultado sin analizar el texto impreso.
 */
int main(void)
{
    /* Encabezado de la suite. */
    printf("== test_tablero ==\n");

    /* Cada bateria cubre una regla independiente del tablero. */
    prueba_inicializacion();
    prueba_rango();
    prueba_libre();
    prueba_ingresar();

    /* Resumen final y codigo de salida para make/CI. */
    printf("------------------\n");
    printf("Pruebas: %d, Fallos: %d\n", pruebas, fallos);
    if (fallos == 0) {
        printf("TODO OK\n");
        return 0;
    }
    printf("HAY FALLOS\n");
    return 1;
}
