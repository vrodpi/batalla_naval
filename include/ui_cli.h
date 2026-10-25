#ifndef BATALLA_NAVAL_UI_CLI_H
#define BATALLA_NAVAL_UI_CLI_H

/*
 * ui_cli.h - Interfaz de consola (entrada/salida del jugador).
 *
 * ---------------------------------------------------------------------------
 * RESPONSABILIDAD
 * ---------------------------------------------------------------------------
 * Este modulo SOLO habla con el usuario: limpia la pantalla, pide datos,
 * muestra textos y menus, y espera confirmaciones. No aplica reglas del
 * juego ni modifica el tablero ni el ranking.
 *
 * La separacion es la del Anexo A:
 *   - app.c decide QUE pantalla sigue;
 *   - ui_cli.c muestra y pregunta;
 *   - tablero.c / ranking.c aplican las reglas de su dominio.
 *
 * Cuando exista una interfaz grafica (ui_gui.c), ofrecera las mismas
 * operaciones publicas y app.c podra cambiar de interfaz sin tocar el
 * motor ni el ranking.
 *
 * ---------------------------------------------------------------------------
 * ESTANDAR
 * ---------------------------------------------------------------------------
 * C99 (-std=c99 -pedantic -Wall -Wextra). Este modulo se compila con el
 * estandar unico del proyecto, sin extensiones del compilador.
 *
 * ---------------------------------------------------------------------------
 * CONTRATO DE ENTRADA
 * ---------------------------------------------------------------------------
 * Las funciones que leen del teclado consumen la linea completa (incluido
 * el '\n'). Asi no quedan caracteres pendientes que hagan que la siguiente
 * pregunta se responda sola. Si se alcanza EOF, devuelven 0 y dejan el
 * buffer vacio para que el llamador pueda salir sin bucles infinitos.
 */

#include <stddef.h>

/*
 * Modos de juego del menu principal.
 *
 * Los valores 1 y 2 se imprimen tal cual en el menu. MODO_SALIR vale 0
 * para que el bucle de seleccion pueda terminar con una condicion simple.
 */
typedef enum {
    MODO_SALIR = 0,
    MODO_JUGADOR_VS_JUGADOR = 1,
    MODO_JUGADOR_VS_MAQUINA = 2
} ModoJuego;

/* Limpia la consola (cls en Windows, clear en Linux/Unix). */
void limpiar_pantalla_cli(void);

/* Muestra "Presione ENTER..." y consume una linea completa. */
void esperar_usuario(void);

/*
 * Pide el nombre de un jugador y lo guarda en `destino` (tamano `tam`,
 * incluido el '\0'). `numero_de_jugador` es 0 o 1 y solo se usa para
 * numerar el mensaje.
 *
 * Devuelve 1 si leyo una linea, 0 si se alcanzo EOF o los argumentos son
 * invalidos. No valida el contenido: de eso se encarga ranking.c.
 */
int pedir_nombre_jugador(int numero_de_jugador, char *destino, size_t tam);

/* Imprime un saludo para el nombre indicado (NULL no imprime nada). */
void saludar_jugador(const char *nombre_jugador);

/*
 * Pide el nombre del siguiente jugador humano y lo agrega a la lista
 * interna de la partida. Como maximo registra CONFIG_JUGADORES_PARTIDA.
 */
void ingresar_jugador(void);

/* Copia a stdout el contenido de un archivo de texto; informa si falla. */
void mostrar_archivo_de_texto(const char *ruta_al_archivo);

/* Pantalla inicial: pide el primer jugador y muestra el reglamento. */
void mostrar_presentacion_y_reglamento(void);

/* Imprime las opciones del menu principal (no lee nada). */
void mostrar_menu_principal(void);

/* Imprime una linea horizontal de separacion. */
void imprimir_linea(void);

/*
 * Muestra el menu y devuelve la opcion valida elegida (ModoJuego).
 * Si el usuario escribe algo invalido, avisa y vuelve a preguntar.
 * Si se alcanza EOF, devuelve MODO_SALIR para no colgar el programa.
 */
ModoJuego elegir_modo_de_juego(void);

#endif
