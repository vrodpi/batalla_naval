/**
 * @file game.c
 * @brief Implementacion del motor de partida.
 *
 * Conserva la estructura del motor que trajeron los companeros y la adapta
 * al tablero propio y al estandar C23. Las reglas de colocacion y disparo se
 * delegan en tablero.c; aca solo se coordinan turnos, contadores y victoria.
 *
 * @note Los barcos se guardan en `Jugador.barco` con su longitud, posicion y
 *       estado. `game_colocar_barco()` mantiene sincronizados el tablero y
 *       ese arreglo, que es lo que usa `barco_hundido()`.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

/**
 * @brief Longitudes de la flota estandar, en el orden en que se colocan.
 */
static const Tamanio FLOTA[CONFIG_NUM_BARCOS] = {
    porta_aviones, buque, submarino, lancha, velero
};

/**
 * @brief Deja un jugador listo para empezar: tableros en agua y flota vacia.
 *
 * @param jugador Jugador que se inicializa.
 * @return No devuelve valor.
 * @note Los barcos quedan con fila y columna -1 para indicar que todavia no
 *       fueron colocados.
 */
static void preparar_jugador(Jugador *jugador)
{
    int i; /* Indice dentro de la flota. */

    if (jugador == NULL) {
        return;
    }

    jugador->nombre[0] = '\0';
    jugador->barcos_vivos = CONFIG_NUM_BARCOS;
    jugador->hits = 0;

    /* La flota arranca con sus longitudes estandar y sin posicion. */
    for (i = 0; i < CONFIG_NUM_BARCOS; i++) {
        jugador->barco[i].tamanio = FLOTA[i];
        jugador->barco[i].orientacion = horizontal;
        jugador->barco[i].fila = -1;
        jugador->barco[i].columna = -1;
        jugador->barco[i].hundido = 0;
    }

    inicializar_tablero(jugador->tableros.tablero,
                        jugador->tableros.tablero_disparos);
}

/**
 * @brief Lee un entero completo desde stdin.
 *
 * @param valor Destino del numero leido.
 * @return 1 si la linea contenia solo un entero; 0 en EOF o entrada invalida.
 * @note Se usa fgets + strtol para no dejar restos en el buffer, a diferencia
 *       de scanf("%d"), que falla sin limpiar la linea cuando el usuario
 *       escribe letras.
 */
static int leer_entero(int *valor)
{
    char linea[32]; /* Linea cruda ingresada. */
    char *fin;      /* Primer caracter no convertido. */
    long numero;    /* Numero convertido por strtol. */

    if (valor == NULL) {
        return 0;
    }

    if (fgets(linea, (int)sizeof(linea), stdin) == NULL) {
        return 0;
    }

    numero = strtol(linea, &fin, 10);

    /* Se toleran espacios y el salto de linea final. */
    while (*fin == ' ' || *fin == '\t' || *fin == '\r' || *fin == '\n') {
        fin++;
    }

    /* Sin digitos o con caracteres extra: no es un entero valido. */
    if (fin == linea || *fin != '\0') {
        return 0;
    }

    /* Evita convertir un long fuera del rango de int. */
    if (numero < INT_MIN || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

/**
 * @brief Inicializa una partida vacia.
 *
 * @param partida Partida que se reinicia.
 * @return No devuelve valor.
 */
void inicializar_partida(Partida *partida)
{
    if (partida == NULL) {
        return;
    }

    partida->turno = PJ1;
    partida->cantidad_turnos = 0;
    partida->modo_de_juego = PVP;
    partida->dificultad = NORMAL;
    partida->victoria = 0;
    partida->ganador = 0;

    preparar_jugador(&partida->jugador1);
    preparar_jugador(&partida->jugador2);

    /* Los nombres se completan desde app.c / ui_cli.c. */
    partida->jugador1.nombre[0] = '\0';
    partida->jugador2.nombre[0] = '\0';
}

/**
 * @brief Coloca un barco de la flota en el tablero propio.
 *
 * @param jugador Jugador duenio de la flota.
 * @param indice Posicion del barco dentro de la flota.
 * @param fila Fila de la proa.
 * @param columna Columna de la proa.
 * @param orientacion horizontal o vertical.
 * @return 1 si se coloco; 0 si la posicion o el indice no son validos.
 */
int game_colocar_barco(Jugador *jugador, int indice, int fila, int columna,
                       Orientacion orientacion)
{
    Tamanio tamanio; /* Longitud del barco que se quiere colocar. */

    if (jugador == NULL || indice < 0 || indice >= CONFIG_NUM_BARCOS) {
        return 0;
    }

    /* Un barco ya colocado no se puede mover sin reiniciar la partida. */
    if (jugador->barco[indice].fila >= 0) {
        return 0;
    }

    tamanio = jugador->barco[indice].tamanio;

    if (!rango(fila, columna, orientacion, tamanio)) {
        return 0;
    }

    if (!libre(fila, columna, orientacion, tamanio,
               jugador->tableros.tablero)) {
        return 0;
    }

    ingresar_barco(fila, columna, orientacion, tamanio,
                   jugador->tableros.tablero);

    /* Guardar la posicion para que barco_hundido() pueda recorrerla. */
    jugador->barco[indice].orientacion = orientacion;
    jugador->barco[indice].fila = fila;
    jugador->barco[indice].columna = columna;
    jugador->barco[indice].hundido = 0;
    return 1;
}

/**
 * @brief Aplica un disparo sobre un tablero concreto.
 *
 * @param fila_disparo Fila del disparo.
 * @param columna_disparo Columna del disparo.
 * @param tablero Tablero que recibe el disparo.
 * @return Resultado del disparo.
 */
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo,
                            char tablero[CONFIG_FILAS][CONFIG_COLUMNAS])
{
    char celda; /* Contenido actual de la casilla. */

    if (tablero == NULL) {
        return disp_NO_valido;
    }

    if (fila_disparo < 0 || fila_disparo >= CONFIG_FILAS) {
        return disp_NO_valido;
    }
    if (columna_disparo < 0 || columna_disparo >= CONFIG_COLUMNAS) {
        return disp_NO_valido;
    }

    celda = tablero[fila_disparo][columna_disparo];

    if (celda == (char)agua) {
        tablero[fila_disparo][columna_disparo] = (char)disparo_a_agua;
        return disp_agua;
    }

    if (celda == (char)barco) {
        tablero[fila_disparo][columna_disparo] = (char)impacto;
        return disp_impacto;
    }

    if (celda == (char)disparo_a_agua || celda == (char)impacto) {
        return disp_repetido;
    }

    /* Cualquier otro caracter indica un tablero corrupto. */
    return disp_NO_valido;
}

/**
 * @brief Marca los barcos que ya recibieron todos sus impactos.
 *
 * @param defensor Jugador que recibio los disparos.
 * @return No devuelve valor.
 */
void barco_hundido(Jugador *defensor)
{
    int i; /* Indice dentro de la flota. */

    if (defensor == NULL) {
        return;
    }

    for (i = 0; i < CONFIG_NUM_BARCOS; i++) {
        Barco *barco_actual; /* Barco que se esta revisando. */
        int largo;           /* Longitud del barco. */
        int j;               /* Avance a lo largo del barco. */
        int hundido;         /* Bandera: todos sus casilleros impactados. */

        barco_actual = &defensor->barco[i];

        /* Un barco ya hundido o que no fue colocado no se revisa. */
        if (barco_actual->hundido != 0 || barco_actual->fila < 0
            || barco_actual->columna < 0) {
            continue;
        }

        largo = (int)barco_actual->tamanio;
        if (largo <= 0) {
            continue;
        }

        hundido = 1;

        for (j = 0; j < largo; j++) {
            int fila = barco_actual->fila;
            int columna = barco_actual->columna;

            if (barco_actual->orientacion == horizontal) {
                columna += j;
            } else {
                fila += j;
            }

            /* Una posicion fuera del tablero no puede estar impactada. */
            if (fila < 0 || fila >= CONFIG_FILAS || columna < 0
                || columna >= CONFIG_COLUMNAS) {
                hundido = 0;
                break;
            }

            if (defensor->tableros.tablero[fila][columna] != (char)impacto) {
                hundido = 0;
                break;
            }
        }

        if (hundido != 0) {
            barco_actual->hundido = 1;
            if (defensor->barcos_vivos > 0) {
                defensor->barcos_vivos--;
            }
            printf("\nBARCO HUNDIDO!\n");
        }
    }
}

/**
 * @brief Aplica un disparo del jugador del turno actual.
 *
 * @param partida Partida en curso.
 * @param fila Fila del disparo.
 * @param columna Columna del disparo.
 * @return Resultado del disparo.
 */
Rdo_Disparo game_realizar_disparo(Partida *partida, int fila, int columna)
{
    Jugador *atacante; /* Jugador que dispara. */
    Jugador *defensor; /* Jugador que recibe el disparo. */
    Rdo_Disparo rdo;   /* Resultado de recibir_disparo. */

    if (partida == NULL || partida->victoria != 0) {
        return disp_NO_valido;
    }

    if (partida->turno == PJ1) {
        atacante = &partida->jugador1;
        defensor = &partida->jugador2;
    } else {
        atacante = &partida->jugador2;
        defensor = &partida->jugador1;
    }

    rdo = recibir_disparo(fila, columna, defensor->tableros.tablero);

    if (rdo == disp_agua) {
        atacante->tableros.tablero_disparos[fila][columna]
            = (char)disparo_a_agua;
    } else if (rdo == disp_impacto) {
        atacante->tableros.tablero_disparos[fila][columna] = (char)impacto;
        atacante->hits++;
        barco_hundido(defensor);
    } else {
        /* Invalido o repetido: no se consume el turno. */
        return rdo;
    }

    partida->cantidad_turnos++;
    partida->turno = (partida->turno == PJ1) ? PJ2 : PJ1;
    (void)fin_juego(partida);
    return rdo;
}

/**
 * @brief Indica si la partida termino y registra al ganador.
 *
 * @param partida Partida consultada.
 * @return 1 si hay ganador; 0 si la partida continua.
 */
int fin_juego(Partida *partida)
{
    if (partida == NULL) {
        return 0;
    }

    if (partida->jugador1.barcos_vivos <= 0) {
        partida->ganador = 2;
        partida->victoria = 1;
        return 1;
    }

    if (partida->jugador2.barcos_vivos <= 0) {
        partida->ganador = 1;
        partida->victoria = 1;
        return 1;
    }

    return 0;
}

/**
 * @brief Pide coordenadas por consola y aplica disparos hasta acertar.
 *
 * @param atacante Jugador que dispara.
 * @param defensor Jugador que recibe el disparo.
 * @return No devuelve valor.
 */
void coordenadas_de_ataque(Jugador *atacante, Jugador *defensor)
{
    int fila;         /* Fila ingresada, base uno. */
    int columna;      /* Columna ingresada, base uno. */
    Rdo_Disparo rdo;  /* Resultado del disparo. */

    if (atacante == NULL || defensor == NULL) {
        return;
    }

    rdo = disp_NO_valido;

    while (rdo == disp_NO_valido || rdo == disp_repetido) {
        printf("\nINGRESE COORDENADAS DE ATAQUE:\n");

        printf("Fila (1 a %d): ", CONFIG_FILAS);
        fflush(stdout);
        if (!leer_entero(&fila)) {
            if (feof(stdin)) {
                return;
            }
            printf("Entrada invalida. Intente de nuevo.\n");
            continue;
        }

        printf("Columna (1 a %d): ", CONFIG_COLUMNAS);
        fflush(stdout);
        if (!leer_entero(&columna)) {
            if (feof(stdin)) {
                return;
            }
            printf("Entrada invalida. Intente de nuevo.\n");
            continue;
        }

        rdo = recibir_disparo(fila - 1, columna - 1,
                              defensor->tableros.tablero);

        if (rdo == disp_agua) {
            atacante->tableros.tablero_disparos[fila - 1][columna - 1]
                = (char)disparo_a_agua;
        } else if (rdo == disp_impacto) {
            atacante->tableros.tablero_disparos[fila - 1][columna - 1]
                = (char)impacto;
            atacante->hits++;
            barco_hundido(defensor);
        } else if (rdo == disp_repetido) {
            printf("Esa casilla ya fue disparada. Intente otra.\n");
        } else {
            printf("Coordenada invalida. Intente de nuevo.\n");
        }
    }
}

/**
 * @brief Ejecuta la partida completa hasta que haya un ganador.
 *
 * @param partida Partida en curso.
 * @return No devuelve valor.
 */
void jugar(Partida *partida)
{
    Jugador *atacante; /* Jugador del turno. */
    Jugador *defensor; /* Rival. */

    if (partida == NULL) {
        return;
    }

    while (fin_juego(partida) == 0) {
        printf("TURNO NUMERO %d\n", partida->cantidad_turnos + 1);

        if (partida->turno == PJ1) {
            atacante = &partida->jugador1;
            defensor = &partida->jugador2;
        } else {
            atacante = &partida->jugador2;
            defensor = &partida->jugador1;
        }

        printf("Turno de: %s\n", atacante->nombre);
        mostrar_tablero(atacante->tableros.tablero,
                        atacante->tableros.tablero_disparos);

        coordenadas_de_ataque(atacante, defensor);

        /* Si la entrada termino, no tiene sentido seguir alternando turnos. */
        if (feof(stdin)) {
            printf("Entrada terminada: partida interrumpida.\n");
            return;
        }

        partida->turno = (partida->turno == PJ1) ? PJ2 : PJ1;
        partida->cantidad_turnos++;
    }

    printf("        FIN DEL JUEGO      \n");

    if (partida->ganador == 1) {
        printf("VICTORIA DE %s\n", partida->jugador1.nombre);
    } else if (partida->ganador == 2) {
        printf("VICTORIA DE %s\n", partida->jugador2.nombre);
    }
}
