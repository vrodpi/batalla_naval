/*
 * demo_ranking.c - Ejemplo de integracion del ranking segun la consigna.
 *
 * Flujo que luego vivira en app.c + ui_cli.c:
 *  1. pide el nombre, saluda y presenta reglas,
 *  2. simula el fin de una partida (ganador),
 *  3. carga ranking, suma 1 victoria, guarda,
 *  4. muestra el ranking y espera una tecla antes de salir.
 *
 * Uso:
 *   demo_ranking            (pide el nombre por teclado)
 *   demo_ranking "Ana"      (usa el nombre dado, util para pruebas)
 */

#include <stdio.h>
#include <string.h>

#include "ranking.h"
#include "config.h"

/* Lee una linea completa por teclado (portable, sin scanf). */
static void leer_linea(char *dest, int tam)
{
    if (fgets(dest, tam, stdin) == NULL) {
        dest[0] = '\0';
        return;
    }
    /* Quita salto de linea. */
    {
        int largo;
        largo = (int)strlen(dest);
        while (largo > 0 && (dest[largo - 1] == '\n' || dest[largo - 1] == '\r')) {
            dest[largo - 1] = '\0';
            largo--;
        }
    }
}

static void esperar_tecla(void)
{
    char buf[16];
    printf("\nPresione ENTER para finalizar...");
    fflush(stdout);
    leer_linea(buf, (int)sizeof(buf));
}

int main(int argc, char *argv[])
{
    Ranking ranking;
    char ganador[RANKING_NOMBRE_MAX];
    int rc;

    printf("=== BATALLA NAVAL ===\n");
    printf("Reglas resumidas: tablero de %dx%d, %d barcos por jugador\n",
           CONFIG_FILAS, CONFIG_COLUMNAS, CONFIG_NUM_BARCOS);
    printf("de longitudes 5, 4, 3, 2 y 1. Gana quien hunde toda la flota rival.\n\n");

    if (argc > 1) {
        strncpy(ganador, argv[1], (int)sizeof(ganador) - 1);
        ganador[sizeof(ganador) - 1] = '\0';
    } else {
        printf("Ingrese el nombre del ganador de la partida: ");
        fflush(stdout);
        leer_linea(ganador, (int)sizeof(ganador));
    }

    printf("\nHola, %s. Gracias por jugar.\n", ganador);

    ranking_inicializar(&ranking);
    rc = ranking_cargar(&ranking, CONFIG_RANKING_RUTA);
    if (rc == RANKING_ERROR_ARCHIVO) {
        printf("(Aviso: no existia %s, se inicia un ranking vacio.)\n",
               CONFIG_RANKING_RUTA);
    } else if (rc == RANKING_ERROR_FORMATO) {
        printf("(Aviso: %s tenia lineas invalidas; se cargaron las validas.)\n",
               CONFIG_RANKING_RUTA);
    } else if (rc != RANKING_OK) {
        printf("No se pudo leer el ranking: %s\n", ranking_mensaje_error(rc));
    }

    rc = ranking_registrar_victoria(&ranking, ganador);
    if (rc != RANKING_OK) {
        printf("No se pudo registrar la victoria: %s\n",
               ranking_mensaje_error(rc));
        esperar_tecla();
        return 1;
    }

    rc = ranking_guardar(&ranking, CONFIG_RANKING_RUTA);
    if (rc != RANKING_OK) {
        printf("No se pudo guardar el ranking en %s: %s\n",
               CONFIG_RANKING_RUTA, ranking_mensaje_error(rc));
    } else {
        printf("Ranking guardado en %s.\n\n", CONFIG_RANKING_RUTA);
    }

    ranking_mostrar(&ranking);
    esperar_tecla();
    return 0;
}
