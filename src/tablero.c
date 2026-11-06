/**
 * @file tablero.c
 * @brief Reglas de tablero y colocacion de barcos.
 *
 * Mantiene el estado de las casillas y decide si un barco puede colocarse.
 * No pide datos por teclado ni muestra menus: eso es ui_cli.c. No decide
 * turnos ni victoria: eso sera game.c.
 *
 * @note Convencion interna: filas y columnas se cuentan desde 0. El tablero
 *       propio guarda '~' (agua) y '#' (barco); el tablero de disparos guarda
 *       '~' (sin disparar), '/' (agua) y 'X' (impacto).
 * @note Regla de colocacion: un barco es valido si su casilla inicial esta
 *       dentro del tablero, si entra completo con la orientacion elegida
 *       (rango) y si todas sus casillas y las de alrededor estan en agua
 *       (libre). El anillo exterior implementa la regla "dejar un espacio
 *       entre barcos".
 * @note Complejidad: rango es O(1); libre recorre a lo sumo un rectangulo de
 *       (largo + 2) x 3 casillas, O(largo), con largo <= 5 por defecto.
 * @note Los tamanios salen de config.h.
 */

#include <stdio.h>

#include "tablero.h"

/**
 * @brief Llena los dos tableros con agua ('~').
 *
 * @param posicion Tablero propio (barcos).
 * @param posicion_disparos Mapa de disparos contra el rival.
 * @return No devuelve valor.
 * @note Si cualquiera de los dos punteros es NULL, no se modifica nada.
 */
void inicializar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                         char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int i; /* Indice de fila. */
    int j; /* Indice de columna. */

    /* Validacion defensiva: evita desreferenciar punteros nulos. */
    if (posicion == NULL || posicion_disparos == NULL) {
        return;
    }

    /* Recorrido completo de ambas matrices. */
    for (i = 0; i < CONFIG_FILAS; i++) {
        for (j = 0; j < CONFIG_COLUMNAS; j++) {
            posicion[i][j] = (char)agua;
            posicion_disparos[i][j] = (char)agua;
        }
    }
}

/**
 * @brief Verifica que un barco entre completo dentro del tablero.
 *
 * @param filas Fila de la proa (base cero).
 * @param columnas Columna de la proa (base cero).
 * @param orientacion horizontal o vertical.
 * @param tamanio Longitud del barco.
 * @return 1 si la colocacion es valida; 0 en caso contrario.
 * @note Solo mira los limites: no consulta el contenido del tablero.
 */
int rango(int filas, int columnas, Orientacion orientacion, Tamanio tamanio)
{
    int largo; /* Longitud efectiva del barco. */

    /* Una orientacion desconocida no se puede validar. */
    if (orientacion != horizontal && orientacion != vertical) {
        return 0;
    }

    /* La casilla inicial debe existir dentro del tablero. */
    if (filas < 0 || filas >= CONFIG_FILAS) {
        return 0;
    }
    if (columnas < 0 || columnas >= CONFIG_COLUMNAS) {
        return 0;
    }

    /* Un barco de longitud cero o negativa no tiene sentido. */
    largo = (int)tamanio;
    if (largo <= 0) {
        return 0;
    }

    /*
     * Si empieza en la columna c y mide `largo`, la ultima casilla es
     * c + largo - 1. Para que no se salga: c + largo <= CONFIG_COLUMNAS.
     * Lo mismo para las filas en vertical.
     */
    if (orientacion == horizontal) {
        return (columnas + largo <= CONFIG_COLUMNAS);
    }
    return (filas + largo <= CONFIG_FILAS);
}

/**
 * @brief Verifica que el barco quepa y que no se superponga ni toque a otro.
 *
 * @param filas Fila de la proa.
 * @param columnas Columna de la proa.
 * @param orientacion horizontal o vertical.
 * @param tamanio Longitud del barco.
 * @param posicion Tablero propio que se consulta (no se modifica).
 * @return 1 si el lugar esta libre; 0 si no.
 * @note La regla actual exige al menos una casilla de agua alrededor del
 *       barco, incluidas las diagonales.
 */
int libre(int filas, int columnas, Orientacion orientacion, Tamanio tamanio,
          char posicion[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int largo;    /* Longitud efectiva. */
    int f;        /* Fila recorrida. */
    int c;        /* Columna recorrida. */
    int fila_ini; /* Primera fila del cuerpo. */
    int fila_fin; /* Ultima fila del cuerpo. */
    int col_ini;  /* Primera columna del cuerpo. */
    int col_fin;  /* Ultima columna del cuerpo. */

    /* Sin tablero no se puede decidir nada. */
    if (posicion == NULL) {
        return 0;
    }

    /* Primero se descartan las colocaciones que se salen del tablero. */
    if (!rango(filas, columnas, orientacion, tamanio)) {
        return 0;
    }

    /* Rectangulo que ocupa el cuerpo del barco. */
    largo = (int)tamanio;
    if (orientacion == horizontal) {
        fila_ini = filas;
        fila_fin = filas;
        col_ini = columnas;
        col_fin = columnas + largo - 1;
    } else {
        fila_ini = filas;
        fila_fin = filas + largo - 1;
        col_ini = columnas;
        col_fin = columnas;
    }

    /*
     * Recorrer el rectangulo expandido un casillero en todas las
     * direcciones. Incluye el cuerpo del barco (debe ser agua) y el anillo
     * exterior (evita que dos barcos se toquen, incluso en diagonal).
     * Las casillas fuera del tablero simplemente se ignoran.
     */
    for (f = fila_ini - 1; f <= fila_fin + 1; f++) {
        for (c = col_ini - 1; c <= col_fin + 1; c++) {
            /* El anillo puede salirse del tablero: se omite esa casilla. */
            if (f < 0 || f >= CONFIG_FILAS || c < 0 || c >= CONFIG_COLUMNAS) {
                continue;
            }
            /* Cualquier cosa distinta de agua bloquea la colocacion. */
            if (posicion[f][c] != (char)agua) {
                return 0;
            }
        }
    }
    return 1;
}

/**
 * @brief Marca con '#' las casillas del barco.
 *
 * @param filas Fila de la proa.
 * @param columnas Columna de la proa.
 * @param orientacion horizontal o vertical.
 * @param tamanio Longitud del barco.
 * @param posicion Tablero propio que se modifica.
 * @return No devuelve valor.
 * @note Presupone que rango() y libre() ya dieron un resultado valido. Si no,
 *       no modifica nada.
 */
void ingresar_barco(int filas, int columnas, Orientacion orientacion,
                    Tamanio tamanio, char posicion[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int i;    /* Avance a lo largo del barco. */
    int largo; /* Longitud efectiva. */

    /* Sin tablero no hay nada que marcar. */
    if (posicion == NULL) {
        return;
    }

    /*
     * Validacion defensiva: el llamador deberia haber llamado a rango() y
     * libre(). Si no lo hizo, no se escribe fuera del tablero.
     */
    if (!rango(filas, columnas, orientacion, tamanio)) {
        return;
    }

    /* Marcar una casilla por unidad de longitud. */
    largo = (int)tamanio;
    for (i = 0; i < largo; i++) {
        if (orientacion == horizontal) {
            posicion[filas][columnas + i] = (char)barco;
        } else {
            posicion[filas + i][columnas] = (char)barco;
        }
    }
}

/**
 * @brief Imprime los dos tableros en la consola.
 *
 * @param posicion Tablero propio (flota).
 * @param posicion_disparos Mapa de disparos contra el rival.
 * @return No devuelve valor.
 * @note Las filas se muestran como 1..10 y las columnas como A..J.
 */
void mostrar_tablero(char posicion[CONFIG_FILAS][CONFIG_COLUMNAS],
                     char posicion_disparos[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    int i; /* Indice de fila. */
    int k; /* Indice de columna. */

    /* Sin matrices no se puede dibujar: se avisa y se sale. */
    if (posicion == NULL || posicion_disparos == NULL) {
        printf("Tablero no inicializado.\n");
        return;
    }

    /* Encabezado del tablero propio y letras de columna. */
    printf("\n--- MI FLOTA ---\n   ");
    for (k = 0; k < CONFIG_COLUMNAS; k++) {
        printf("%c ", 'A' + k);
    }
    printf("\n");

    /* Filas del tablero propio, numeradas desde 1. */
    for (i = 0; i < CONFIG_FILAS; i++) {
        printf("%2d ", i + 1);
        for (k = 0; k < CONFIG_COLUMNAS; k++) {
            printf("%c ", posicion[i][k]);
        }
        printf("\n");
    }

    /* Encabezado del mapa de disparos y letras de columna. */
    printf("\n--- DISPAROS A FLOTA ENEMIGA ---\n   ");
    for (k = 0; k < CONFIG_COLUMNAS; k++) {
        printf("%c ", 'A' + k);
    }
    printf("\n");

    /* Filas del mapa de disparos, numeradas desde 1. */
    for (i = 0; i < CONFIG_FILAS; i++) {
        printf("%2d ", i + 1);
        for (k = 0; k < CONFIG_COLUMNAS; k++) {
            printf("%c ", posicion_disparos[i][k]);
        }
        printf("\n");
    }
}
