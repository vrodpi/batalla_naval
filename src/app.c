/*
 * app.c - Coordinador del flujo de pantallas de Batalla Naval.
 *
 * ---------------------------------------------------------------------------
 * RESPONSABILIDAD
 * ---------------------------------------------------------------------------
 * Decide QUE pantalla sigue y en que orden. No implementa reglas del juego
 * ni escribe en el tablero ni en el ranking: para eso llama a los modulos
 * especializados. La interfaz concreta (ui_cli.c) solo muestra y pregunta.
 *
 * Flujo actual (esqueleto integrado):
 *   1. presentacion + reglamento + nombre del primer jugador;
 *   2. menu principal (jugador vs jugador / jugador vs maquina / salir);
 *   3. si elige un modo, informa la eleccion y espera ENTER.
 *
 * ESTADO ACTUAL: el motor de partida (game.c / ai.c) todavia no existe.
 * Por eso el modo elegido no inicia una partida real; se deja preparado el
 * punto exacto donde se conectara. Cuando el motor detecte una victoria,
 * app.c debera:
 *
 *   ranking_inicializar(&ranking);
 *   rc = ranking_cargar(&ranking, CONFIG_RANKING_RUTA);
 *   ranking_registrar_victoria(&ranking, ganador);
 *   ranking_guardar(&ranking, CONFIG_RANKING_RUTA);
 *   ranking_mostrar(&ranking);
 *
 * Esa secuencia es la unica via valida: la interfaz no suma victorias ni
 * toca el arreglo interno del ranking (ver docs/contrato-ranking.md).
 *
 * ESTANDAR: C99 (-std=c99 -pedantic -Wall -Wextra).
 */

#include <stdio.h>

#include "app.h"
#include "ui_cli.h"

int app_run(void)
{
    ModoJuego modo;

    /* 1. Presentacion: pide el nombre y muestra el reglamento. */
    mostrar_presentacion_y_reglamento();

    /* 2. Menu principal: elegir_modo_de_juego() ya valida la entrada. */
    modo = elegir_modo_de_juego();

    if (modo == MODO_SALIR) {
        printf("Hasta luego.\n");
        return 0;
    }

    /*
     * 3. Modo elegido. Cuando exista el motor, aqui se llamara a la
     * funcion que inicia la partida (game_iniciar o equivalente).
     */
    if (modo == MODO_JUGADOR_VS_JUGADOR) {
        printf("Modo seleccionado: Jugador vs Jugador.\n");
    } else {
        printf("Modo seleccionado: Jugador vs Maquina.\n");
    }
    printf("El motor de la partida se integra en la proxima etapa.\n");
    esperar_usuario();

    return 0;
}
