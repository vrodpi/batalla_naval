/**
 * @file app.c
 * @brief Coordinador del flujo de pantallas de Batalla Naval.
 *
 * Decide QUE pantalla sigue y en que orden. No implementa reglas del juego
 * ni escribe en el tablero ni en el ranking: para eso llama a los modulos
 * especializados. La interfaz concreta (ui_cli.c) solo muestra y pregunta.
 *
 * Flujo actual (esqueleto integrado):
 *   1. presentacion + reglamento + nombre del primer jugador;
 *   2. menu principal (jugador vs jugador / jugador vs maquina / salir);
 *   3. si elige un modo, informa la eleccion y espera ENTER.
 *
 * @note ESTADO ACTUAL: el motor de partida (game.c / ai.c) todavia no existe.
 *       Por eso el modo elegido no inicia una partida real; se deja preparado
 *       el punto exacto donde se conectara. Cuando el motor detecte una
 *       victoria, app.c debera ejecutar la secuencia:
 *       ranking_inicializar, ranking_cargar, ranking_registrar_victoria,
 *       ranking_guardar y ranking_mostrar.
 * @note Esa secuencia es la unica via valida: la interfaz no suma victorias
 *       ni toca el arreglo interno del ranking (ver docs/contrato-ranking.md).
 * @note Estandar unico: C99 (-std=c99 -pedantic -Wall -Wextra).
 */

#include <stdio.h>

#include "app.h"
#include "ui_cli.h"

/**
 * @brief Ejecuta la aplicacion completa.
 *
 * Muestra la presentacion y el reglamento, pide el nombre del primer jugador,
 * presenta el menu principal y atiende la opcion elegida. Mientras el motor
 * no exista, informa el modo seleccionado y espera ENTER.
 *
 * @return 0 si el flujo termino normalmente. En el esqueleto actual siempre
 *         devuelve 0.
 * @note No recibe argumentos: las rutas y los tamanios se toman de config.h.
 * @note Toda la entrada/salida se delega en ui_cli.c; aca solo se decide el
 *       orden de las pantallas.
 */
int app_run(void)
{
    ModoJuego modo; /* Opcion elegida en el menu principal. */

    /* 1. Presentacion: pide el nombre y muestra el reglamento. */
    mostrar_presentacion_y_reglamento();

    /* 2. Menu principal: elegir_modo_de_juego() ya valida la entrada. */
    modo = elegir_modo_de_juego();

    /* 3. Salida explicita: no se inicia ninguna partida. */
    if (modo == MODO_SALIR) {
        printf("Hasta luego.\n");
        return 0;
    }

    /*
     * 4. Modo elegido. Cuando exista el motor, aqui se llamara a la funcion
     *    que inicia la partida (game_iniciar o equivalente).
     */
    if (modo == MODO_JUGADOR_VS_JUGADOR) {
        printf("Modo seleccionado: Jugador vs Jugador.\n");
    } else {
        /* Cualquier modo distinto de salir y de vs-jugador es vs-maquina. */
        printf("Modo seleccionado: Jugador vs Maquina.\n");
    }

    /* 5. Aviso temporal y pausa para que el usuario lea el resultado. */
    printf("El motor de la partida se integra en la proxima etapa.\n");
    esperar_usuario();

    return 0;
}
