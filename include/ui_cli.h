/**
 * @file ui_cli.h
 * @brief Interfaz de consola (entrada/salida del jugador).
 *
 * Este modulo SOLO habla con el usuario: limpia la pantalla, pide datos,
 * muestra textos y menus, y espera confirmaciones. No aplica reglas del
 * juego ni modifica el tablero ni el ranking.
 *
 * @note Separacion de capas: app.c decide QUE pantalla sigue; ui_cli.c
 *       muestra y pregunta; tablero.c / ranking.c aplican las reglas.
 * @note Cuando exista una interfaz grafica (ui_gui.c), ofrecera las mismas
 *       operaciones publicas y app.c podra cambiar de interfaz sin tocar el
 *       motor ni el ranking.
 * @note Estandar unico: C99 (-std=c99 -pedantic -Wall -Wextra).
 * @note Contrato de entrada: las funciones que leen del teclado consumen la
 *       linea completa (incluido el '\n'). Si se alcanza EOF, devuelven 0 y
 *       dejan el buffer vacio para que el llamador pueda salir sin bucles
 *       infinitos.
 */

#ifndef BATALLA_NAVAL_UI_CLI_H
#define BATALLA_NAVAL_UI_CLI_H

#include <stddef.h>

/**
 * @brief Modos de juego del menu principal.
 * @note Los valores 1 y 2 se imprimen tal cual en el menu. MODO_SALIR vale 0
 *       para que el bucle de seleccion pueda terminar con una condicion
 *       simple.
 */
typedef enum {
    MODO_SALIR = 0,                /**< Salir del juego. */
    MODO_JUGADOR_VS_JUGADOR = 1,   /**< Dos jugadores humanos. */
    MODO_JUGADOR_VS_MAQUINA = 2    /**< Un humano contra la maquina. */
} ModoJuego;

/**
 * @brief Limpia la consola.
 *
 * En Windows usa `cls`; en Linux/Unix usa `clear` si hay terminal. Si no hay
 * terminal disponible, imprime lineas en blanco.
 *
 * @return No devuelve valor.
 * @note No se puede garantizar el resultado de `system`, por eso se ignora
 *       deliberadamente con (void).
 */
void limpiar_pantalla_cli(void);

/**
 * @brief Muestra "Presione ENTER..." y consume una linea completa.
 * @return No devuelve valor.
 * @note Usa fflush(stdout) para que el mensaje aparezca antes de bloquearse.
 */
void esperar_usuario(void);

/**
 * @brief Pide el nombre de un jugador y lo guarda en `destino`.
 *
 * @param numero_de_jugador Numero base cero usado solo para numerar el
 *        mensaje (0 muestra "jugador 1").
 * @param destino Buffer donde se guarda el nombre terminado en '\0'.
 * @param tam Tamano total del buffer, incluido el '\0'.
 * @return 1 si leyo una linea; 0 si se alcanzo EOF o los argumentos son
 *         invalidos.
 * @note Recorta espacios de los extremos, pero no valida el contenido: de
 *       eso se encarga ranking.c.
 */
int pedir_nombre_jugador(int numero_de_jugador, char *destino, size_t tam);

/**
 * @brief Imprime un saludo para el nombre indicado.
 *
 * @param nombre_jugador Nombre a saludar.
 * @return No devuelve valor.
 * @note Si el puntero es NULL no imprime nada.
 */
void saludar_jugador(const char *nombre_jugador);

/**
 * @brief Pide el nombre del siguiente jugador humano y lo registra.
 *
 * Agrega el nombre a la lista interna de la partida. Como maximo registra
 * CONFIG_JUGADORES_PARTIDA jugadores.
 *
 * @return No devuelve valor.
 * @note Repite la pregunta si el nombre queda vacio y sale sin registrar si
 *       se alcanza EOF.
 */
void ingresar_jugador(void);

/**
 * @brief Copia a stdout el contenido de un archivo de texto.
 *
 * @param ruta_al_archivo Ruta relativa o absoluta del archivo.
 * @return No devuelve valor.
 * @note Si la ruta es NULL o el archivo no se puede abrir, informa el
 *       problema en pantalla.
 */
void mostrar_archivo_de_texto(const char *ruta_al_archivo);

/**
 * @brief Muestra la pantalla inicial y el reglamento.
 *
 * Limpia la pantalla, pide el primer jugador, muestra el archivo de
 * reglamento y espera ENTER.
 *
 * @return No devuelve valor.
 * @note Depende de CONFIG_REGLAMENTO_RUTA; el programa debe ejecutarse desde
 *       la raiz del proyecto.
 */
void mostrar_presentacion_y_reglamento(void);

/**
 * @brief Imprime las opciones del menu principal.
 * @return No devuelve valor.
 * @note Solo muestra; no lee ni valida ninguna opcion.
 */
void mostrar_menu_principal(void);

/**
 * @brief Imprime una linea horizontal de separacion.
 * @return No devuelve valor.
 * @note El ancho se define internamente con UI_ANCHO_LINEA.
 */
void imprimir_linea(void);

/**
 * @brief Muestra el menu y devuelve la opcion valida elegida.
 *
 * @return El ModoJuego elegido (MODO_SALIR, MODO_JUGADOR_VS_JUGADOR o
 *         MODO_JUGADOR_VS_MAQUINA).
 * @note Si la entrada es invalida, avisa y vuelve a preguntar. Si se alcanza
 *       EOF, devuelve MODO_SALIR para no colgar el programa.
 */
ModoJuego elegir_modo_de_juego(void);

#endif /* BATALLA_NAVAL_UI_CLI_H */
