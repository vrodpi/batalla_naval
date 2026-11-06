/**
 * @file ui_cli.h
 * @brief Interfaz de consola (entrada/salida del jugador).
 *
 * Este modulo SOLO habla con el usuario: limpia la pantalla, pide datos,
 * muestra textos y menus, y espera confirmaciones. No aplica reglas del
 * juego ni modifica el tablero ni el ranking.
 *
 * @note Separacion de capas: app.c decide QUE pantalla sigue; ui_cli.c
 *       muestra y pregunta; tablero.c / game.c / ranking.c aplican las reglas.
 * @note Contrato de entrada: las funciones que leen del teclado consumen la
 *       linea completa (incluido el '\n'). Si se alcanza EOF, devuelven un
 *       codigo de error o un valor por defecto para que el llamador pueda
 *       salir sin bucles infinitos.
 */

#ifndef BATALLA_NAVAL_UI_CLI_H
#define BATALLA_NAVAL_UI_CLI_H

#include <stddef.h>

#include "game.h"

/**
 * @brief Limpia la consola.
 *
 * En Windows usa `cls`; en Linux/Unix usa `clear` si hay terminal. Si no hay
 * terminal disponible, imprime lineas en blanco.
 *
 * @return No devuelve valor.
 */
void limpiar_pantalla_cli(void);

/**
 * @brief Muestra "Presione ENTER..." y consume una linea completa.
 * @return No devuelve valor.
 */
void esperar_usuario(void);

/**
 * @brief Imprime una linea horizontal de separacion.
 * @return No devuelve valor.
 */
void imprimir_linea(void);

/**
 * @brief Pide una opcion de menu por consola.
 *
 * @return La opcion elegida (0..4) o -1 si la entrada es invalida o se
 *         alcanzo EOF.
 * @note La validacion del rango concreto la hace cada menu.
 */
int pedir_opcion_al_usuario(void);

/**
 * @brief Pide el nombre de un jugador y lo guarda en `destino`.
 *
 * @param numero_de_jugador Numero base cero usado solo para numerar el mensaje.
 * @param destino Buffer donde se guarda el nombre terminado en '\0'.
 * @param tam Tamano total del buffer, incluido el '\0'.
 * @return 1 si leyo una linea; 0 si se alcanzo EOF o los argumentos son
 *         invalidos.
 */
int pedir_nombre_jugador(int numero_de_jugador, char *destino, size_t tam);

/**
 * @brief Imprime un saludo para el nombre indicado.
 * @param nombre_jugador Nombre a saludar.
 * @return No devuelve valor.
 */
void saludar_jugador(const char *nombre_jugador);

/**
 * @brief Copia a stdout el contenido de un archivo de texto.
 *
 * @param ruta_al_archivo Ruta relativa o absoluta del archivo.
 * @return No devuelve valor.
 */
void mostrar_archivo_de_texto(const char *ruta_al_archivo);

/**
 * @brief Muestra la pantalla inicial y el reglamento.
 *
 * Pide el nombre del primer jugador, lo guarda en `nombre`, muestra el
 * reglamento y espera ENTER.
 *
 * @param nombre Buffer donde se guarda el nombre del jugador 1.
 * @param tam Tamano total del buffer.
 * @return No devuelve valor.
 * @note Si no hay entrada, guarda "Jugador 1" para no dejar el nombre vacio.
 */
void mostrar_presentacion_y_reglamento(char *nombre, size_t tam);

/**
 * @brief Imprime las opciones del menu principal.
 * @return No devuelve valor.
 */
void mostrar_menu_principal(void);

/**
 * @brief Imprime las opciones del menu de dificultad.
 * @return No devuelve valor.
 */
void mostrar_menu_de_dificultad(void);

/**
 * @brief Muestra el menu de dificultad y guarda la opcion elegida.
 *
 * @param dificultad Destino de la dificultad elegida.
 * @return 1 si se eligio NORMAL o TORMENTA; 0 si el usuario volvio atras o
 *         se alcanzo EOF.
 */
int elegir_dificultad(Dificultad *dificultad);

/**
 * @brief Muestra el menu principal y guarda modo y dificultad.
 *
 * Incluye las opciones de ranking y reglamento, que se muestran sin salir
 * del menu. La opcion 0 termina el programa.
 *
 * @param modo Destino del modo elegido (PVP o PVIA).
 * @param dificultad Destino de la dificultad elegida.
 * @return 1 si se eligio un modo valido; 0 si el usuario salio o se alcanzo
 *         EOF.
 */
int elegir_modo_de_juego(Modo_de_juego *modo, Dificultad *dificultad);

/**
 * @brief Pide una fila del tablero.
 *
 * @return Fila base cero (0..CONFIG_FILAS-1) o -1 si la entrada es invalida.
 */
int solicitar_entrada_de_fila(void);

/**
 * @brief Pide una columna del tablero.
 *
 * Acepta letras de A a J (mayusculas o minusculas).
 *
 * @return Columna base cero (0..CONFIG_COLUMNAS-1) o -1 si la entrada es
 *         invalida.
 */
int solicitar_entrada_de_columna(void);

#endif /* BATALLA_NAVAL_UI_CLI_H */
