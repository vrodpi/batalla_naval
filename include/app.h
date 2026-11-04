/**
 * @file app.h
 * @brief Punto de entrada de la capa de aplicacion.
 *
 * @note app.c coordina el flujo de pantallas: presentacion, reglamento,
 *       menu y (cuando exista el motor) la partida y el cierre con ranking.
 *       main.c solo llama a app_run(); no contiene menus ni reglas.
 * @note Esta separacion permite que una futura interfaz grafica reemplace a
 *       ui_cli.c sin tocar main.c ni el motor del juego.
 * @note ESTADO ACTUAL: el esqueleto muestra presentacion, reglamento y menu.
 *       El motor de partida (game/ai) y el registro automatico del ganador
 *       en ranking.c quedan pendientes de integracion.
 */

#ifndef BATALLA_NAVAL_APP_H
#define BATALLA_NAVAL_APP_H

/**
 * @brief Ejecuta la aplicacion completa.
 *
 * Coordina la presentacion, la lectura del reglamento, el menu principal y
 * la pantalla posterior a la eleccion del modo. Es el unico punto de entrada
 * que main.c necesita conocer.
 *
 * @return 0 si la aplicacion termino normalmente; distinto de 0 si hubo un
 *         error fatal.
 * @note No recibe argumentos: la configuracion se toma de config.h y las
 *       rutas son relativas a la raiz del proyecto.
 */
int app_run(void);

#endif /* BATALLA_NAVAL_APP_H */
