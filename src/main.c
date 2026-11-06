/**
 * @file main.c
 * @brief Punto de entrada del juego Batalla Naval.
 *
 * La tabla de responsabilidades del documento general dice que main.c debe
 * mantenerse pequeno: inicializa la aplicacion y llama a app_run(). Toda la
 * inteligencia vive en otros modulos:
 *   - app.c     coordina pantallas, menu, turnos y fin de partida;
 *   - ui_cli.c  habla con el jugador (muestra/pide);
 *   - tablero.c aplica las reglas de tablero y colocacion;
 *   - ranking.c guarda las victorias;
 *   - game.c ya esta integrado; ai.c se integrara en la proxima etapa.
 *
 * @note Recorrido de una ejecucion: main -> app_run (presentacion,
 *       reglamento, menu) -> ... Cuando exista el motor: partida -> victoria
 *       -> ranking.
 */

#include "app.h"

/**
 * @brief Inicia el programa y delega todo el flujo en app_run().
 *
 * @return El codigo de salida que devuelve app_run().
 * @note No se leen argumentos: el programa trabaja con rutas relativas a la
 *       raiz del proyecto y no necesita configuracion por linea de comandos.
 */
int main(void)
{
    /* Delegacion total: main no conoce menus, reglas ni persistencia. */
    return app_run();
}
