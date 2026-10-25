#ifndef BATALLA_NAVAL_APP_H
#define BATALLA_NAVAL_APP_H

/*
 * app.h - Punto de entrada de la capa de aplicacion.
 *
 * ---------------------------------------------------------------------------
 * RESPONSABILIDAD
 * ---------------------------------------------------------------------------
 * app.c coordina el flujo de pantallas: presentacion, reglamento, menu y
 * (cuando exista el motor) la partida y el cierre con ranking. main.c solo
 * llama a app_run(); no contiene menus ni reglas.
 *
 * Esta separacion permite que una futura interfaz grafica reemplace a
 * ui_cli.c sin tocar main.c ni el motor del juego.
 *
 * ESTADO ACTUAL: el esqueleto muestra presentacion, reglamento y menu.
 * El motor de partida (game/ai) y el registro automatico del ganador en
 * ranking.c quedan pendientes de integracion.
 */

/*
 * Ejecuta la aplicacion completa.
 * Devuelve 0 si termino normalmente, distinto de 0 si hubo un error fatal.
 */
int app_run(void);

#endif
