/**
 * @file app.c
 * @brief Coordinador del flujo de pantallas de Batalla Naval.
 *
 * Decide QUE pantalla sigue y en que orden. No implementa reglas del juego
 * ni escribe en el tablero ni en el ranking: para eso llama a los modulos
 * especializados. La interfaz concreta (ui_cli.c) solo muestra y pregunta.
 *
 * Flujo actual:
 *   1. presentacion + reglamento + nombre del primer jugador;
 *   2. menu principal con modo, dificultad, ranking y reglamento;
 *   3. nombre del segundo jugador (o "IA") y resumen de la partida.
 *
 * @note El motor de partida (game.c) ya esta integrado y probado. Falta
 *       conectar la colocacion manual de la flota y la IA para iniciar una
 *       partida completa desde este flujo.
 * @note Cuando el motor detecte una victoria, app.c debera ejecutar la
 *       secuencia ranking_inicializar, ranking_cargar,
 *       ranking_registrar_victoria, ranking_guardar y ranking_mostrar.
 */

#include <stdio.h>
#include <string.h>

#include "app.h"
#include "game.h"
#include "ui_cli.h"

/**
 * @brief Ejecuta la aplicacion completa.
 *
 * @return 0 si el flujo termino normalmente.
 * @note Toda la entrada/salida se delega en ui_cli.c; aca solo se decide el
 *       orden de las pantallas y se completa el estado de la partida.
 */
int app_run(void)
{
    Partida partida; /* Estado de la partida que se esta preparando. */
    int hay_modo;    /* 1 si el usuario eligio un modo; 0 si salio. */

    inicializar_partida(&partida);

    /* 1. Presentacion: pide el nombre del jugador 1 y muestra el reglamento. */
    mostrar_presentacion_y_reglamento(partida.jugador1.nombre,
                                      sizeof(partida.jugador1.nombre));

    /* 2. Menu principal: modo, dificultad, ranking o reglamento. */
    hay_modo = elegir_modo_de_juego(&partida.modo_de_juego,
                                    &partida.dificultad);
    if (!hay_modo) {
        printf("Hasta luego.\n");
        return 0;
    }

    /* 3. Nombre del segundo jugador o de la maquina. */
    if (partida.modo_de_juego == PVP) {
        limpiar_pantalla_cli();
        imprimir_linea();
        printf("JUGADOR 2\n");
        imprimir_linea();

        if (!pedir_nombre_jugador(1, partida.jugador2.nombre,
                                  sizeof(partida.jugador2.nombre))
            || partida.jugador2.nombre[0] == '\0') {
            (void)snprintf(partida.jugador2.nombre,
                           sizeof(partida.jugador2.nombre), "Jugador 2");
        }
    } else {
        (void)snprintf(partida.jugador2.nombre,
                       sizeof(partida.jugador2.nombre), "IA");
    }

    /* 4. Resumen de la configuracion elegida. */
    limpiar_pantalla_cli();
    imprimir_linea();
    printf("MODO: %s\n", partida.modo_de_juego == PVP
                             ? "Jugador vs Jugador"
                             : "Jugador vs Maquina");
    printf("DIFICULTAD: %s\n",
           partida.dificultad == NORMAL ? "Normal" : "Tormenta");
    printf("JUGADOR 1: %s\n", partida.jugador1.nombre);
    printf("JUGADOR 2: %s\n", partida.jugador2.nombre);
    imprimir_linea();
    printf("El motor de partida ya esta integrado y probado.\n");
    printf("La colocacion manual de la flota y la IA quedan para la proxima etapa.\n");
    esperar_usuario();

    return 0;
}
