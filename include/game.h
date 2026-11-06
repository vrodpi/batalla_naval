/**
 * @file game.h
 * @brief Motor de la partida: turnos, disparos y condicion de victoria.
 *
 * Coordina el estado de una partida (dos jugadores, sus tableros, la flota y
 * el turno actual) y aplica las reglas de disparo y fin de juego. No pide
 * datos por teclado: la interfaz captura las coordenadas y app.c decide
 * cuando llamar al motor.
 *
 * @note La flota y los tableros salen de config.h; el motor no repite
 *       literales como 10 o 5.
 * @note Flujo previsto:
 *       1. inicializar_partida();
 *       2. game_colocar_barco() por cada barco de la flota;
 *       3. game_realizar_disparo() en cada turno;
 *       4. fin_juego() indica si la partida termino.
 */

#ifndef BATALLA_NAVAL_GAME_H
#define BATALLA_NAVAL_GAME_H

#include "config.h"
#include "tablero.h"

/**
 * @brief Cantidad de barcos de la flota estandar.
 * @note Se mantiene el nombre que usaba el motor original para que las
 *       funciones heredadas conserven su lectura.
 */
#define BARCOS_MODO_NORMAL CONFIG_NUM_BARCOS

/**
 * @brief Modo de juego elegido en el menu principal.
 */
typedef enum {
    PVP = 0,  /**< Jugador contra jugador. */
    PVIA = 1  /**< Jugador contra la maquina. */
} Modo_de_juego;

/**
 * @brief Dificultad de la partida.
 */
typedef enum {
    NORMAL = 0,  /**< Modo normal. */
    TORMENTA = 1 /**< Modo tormenta. */
} Dificultad;

/**
 * @brief Turno actual de la partida.
 */
typedef enum {
    PJ1 = 0, /**< Le toca al jugador 1. */
    PJ2 = 1  /**< Le toca al jugador 2. */
} Turno;

/**
 * @brief Estado completo de un jugador.
 */
typedef struct {
    Tableros tableros;              /**< Flota propia y disparos al rival. */
    char nombre[CONFIG_NOMBRE_MAX]; /**< Nombre terminado en '\0'. */
    Barco barco[CONFIG_NUM_BARCOS]; /**< Flota y posicion de cada barco. */
    int barcos_vivos;               /**< Barcos que siguen a flote. */
    int hits;                       /**< Impactos acertados contra el rival. */
} Jugador;

/**
 * @brief Estado completo de una partida.
 */
typedef struct {
    Jugador jugador1;            /**< Primer jugador. */
    Jugador jugador2;            /**< Segundo jugador o maquina. */
    Turno turno;                 /**< A quien le toca disparar. */
    int cantidad_turnos;         /**< Turnos completados. */
    Modo_de_juego modo_de_juego; /**< PVP o PVIA. */
    Dificultad dificultad;       /**< NORMAL o TORMENTA. */
    int victoria;                /**< 1 cuando la partida termino. */
    int ganador;                 /**< 0 sin ganador; 1 o 2 con ganador. */
} Partida;

/**
 * @brief Inicializa una partida vacia.
 *
 * Deja ambos tableros en agua, la flota con sus longitudes estandar y sin
 * colocar, los contadores en cero y el turno en PJ1.
 *
 * @param partida Partida que se quiere reiniciar.
 * @return No devuelve valor.
 * @note Si `partida` es NULL no hace nada.
 */
void inicializar_partida(Partida *partida);

/**
 * @brief Coloca un barco de la flota en el tablero propio.
 *
 * Valida con rango() y libre(), marca las casillas con ingresar_barco() y
 * guarda la posicion en el arreglo `barco` del jugador.
 *
 * @param jugador Jugador duenio de la flota.
 * @param indice Posicion del barco dentro de la flota (0..CONFIG_NUM_BARCOS-1).
 * @param fila Fila de la proa (base cero).
 * @param columna Columna de la proa (base cero).
 * @param orientacion horizontal o vertical.
 * @return 1 si el barco se coloco; 0 si el indice, la posicion o el espacio
 *         no son validos, o si el barco ya estaba colocado.
 * @note No modifica el tablero si la colocacion es invalida.
 */
int game_colocar_barco(Jugador *jugador, int indice, int fila, int columna,
                       Orientacion orientacion);

/**
 * @brief Aplica un disparo del jugador del turno actual.
 *
 * Actualiza el tablero del rival, marca el disparo en el mapa del atacante,
 * suma impactos, revisa barcos hundidos y cambia el turno solo si el disparo
 * fue valido y no repetido.
 *
 * @param partida Partida en curso.
 * @param fila Fila del disparo (base cero).
 * @param columna Columna del disparo (base cero).
 * @return Resultado del disparo (agua, impacto, repetido o invalido).
 * @note Si la partida ya termino, devuelve disp_NO_valido.
 */
Rdo_Disparo game_realizar_disparo(Partida *partida, int fila, int columna);

/**
 * @brief Aplica un disparo sobre un tablero concreto.
 *
 * Convierte el agua en '/' y el barco en 'X'. No conoce al atacante ni
 * cambia turnos: eso lo decide game_realizar_disparo().
 *
 * @param fila_disparo Fila del disparo (base cero).
 * @param columna_disparo Columna del disparo (base cero).
 * @param tablero Tablero que recibe el disparo.
 * @return disp_agua, disp_impacto, disp_repetido o disp_NO_valido.
 */
Rdo_Disparo recibir_disparo(int fila_disparo, int columna_disparo,
                            char tablero[CONFIG_FILAS][CONFIG_COLUMNAS]);

/**
 * @brief Marca como hundidos los barcos que ya recibieron todos sus impactos.
 *
 * Recorre la flota del defensor y descuenta `barcos_vivos` una sola vez por
 * barco. Los barcos que todavia no fueron colocados se ignoran.
 *
 * @param defensor Jugador que recibio los disparos.
 * @return No devuelve valor.
 */
void barco_hundido(Jugador *defensor);

/**
 * @brief Indica si la partida termino y registra al ganador.
 *
 * @param partida Partida consultada.
 * @return 1 si un jugador se quedo sin barcos; 0 si la partida continua.
 * @note Si termina, deja `ganador` en 1 o 2 y `victoria` en 1.
 */
int fin_juego(Partida *partida);

/**
 * @brief Pide coordenadas por consola y aplica disparos hasta acertar.
 *
 * Funcion heredada del motor original. Lee fila y columna con entrada
 * segura, reintenta si la coordenada es invalida o repetida y actualiza los
 * tableros del atacante y del defensor.
 *
 * @param atacante Jugador que dispara.
 * @param defensor Jugador que recibe el disparo.
 * @return No devuelve valor.
 * @note Si se alcanza EOF, la funcion termina sin colgarse.
 */
void coordenadas_de_ataque(Jugador *atacante, Jugador *defensor);

/**
 * @brief Ejecuta la partida completa hasta que haya un ganador.
 *
 * Muestra el tablero del atacante, pide coordenadas y alterna los turnos.
 * Presupone que la flota ya fue colocada con game_colocar_barco().
 *
 * @param partida Partida en curso.
 * @return No devuelve valor.
 */
void jugar(Partida *partida);

#endif /* BATALLA_NAVAL_GAME_H */
