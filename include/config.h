#ifndef BATALLA_NAVAL_CONFIG_H
#define BATALLA_NAVAL_CONFIG_H

/*
 * config.h - Parametros configurables del juego.
 *
 * El tamano del tablero y la flota se definen aqui para poder
 * cambiarlos sin tocar la logica de board.c / game.c / ai.c.
 * Todos los modulos deben incluir este archivo en lugar de
 * repetir literales 10 o 5 en el codigo.
 */

/* Tablero cuadrado inicial de 10 x 10. */
#define FIL 10
#define COL 10

/* Flota: cantidad de barcos y sus longitudes (mayor a menor). */
#define BARCOS_MODO_NORMAL 5

/*Nombres de usuarios de hasta 20 caracteres*/
#define MAX_NOMBRE_DE_USUARIO 21

/* Ruta por defecto del ranking (relativa a la raiz del proyecto). */
/* El programa debe ejecutarse desde la raiz para que exista ./data/. */
#define CONFIG_RANKING_RUTA "data/ranking.dat"
#define CONFIG_REGLAMENTO_RUTA "files/Reglamento.txt"


#endif
