/*
 * main.c - Punto de entrada del juego Batalla Naval.
 *
 * ---------------------------------------------------------------------------
 * POR QUE ESTE ARCHIVO ES INTENCIONALMENTE MINIMO
 * ---------------------------------------------------------------------------
 * La tabla de responsabilidades del documento general dice: "main.c: punto
 * de entrada. Inicializa la aplicacion y llama a app_run(). Debe
 * mantenerse pequeno". Toda la inteligencia vive en otros modulos:
 *   - app.c     coordina pantallas, menu, turnos y fin de partida;
 *   - ui_cli.c  habla con el jugador (muestra/pide);
 *   - tablero.c aplica las reglas de tablero y colocacion;
 *   - ranking.c guarda las victorias;
 *   - game.c / ai.c se integraran en la proxima etapa.
 *
 * Si main.c creciera (menus, bucles, reglas), mezclaria capas y violaria
 * el Anexo A; ademas seria lo primero que un docente pediria adelgazar
 * en la defensa ("¿que funciones elegiste separar y por que?").
 *
 * ESTANDAR: C99 (-std=c99 -pedantic -Wall -Wextra).
 *
 * Recorrido de una ejecucion:
 *   main -> app_run (presentacion, reglamento, menu) -> ...
 *   Cuando exista el motor: partida -> victoria -> ranking.
 */

#include "app.h"

int main(void)
{
    return app_run();
}
