/**
 * @file ranking.h
 * @brief Contrato publico del ranking de victorias.
 * @note El formato binario y las decisiones de validacion se documentan en
 *       ranking.c y docs/contrato-ranking.md.
 */

#ifndef BATALLA_NAVAL_RANKING_H
#define BATALLA_NAVAL_RANKING_H

#include <stdio.h>
#include "config.h"

#define RANKING_NOMBRE_MAX 32        /* Incluye el '\0'. */
#define RANKING_MAX_JUGADORES 100
#define RANKING_RUTA_DEFAULT "data/ranking.dat"
#define RANKING_VICTORIAS_MAX 1000000

/* Codigos de retorno: 0 exito; 1 argumento; 2 nombre; 3 lleno; 4 E/S; 5 formato. */
#define RANKING_OK 0
#define RANKING_ERROR_ARGUMENTO 1
#define RANKING_ERROR_NOMBRE_INVALIDO 2
#define RANKING_ERROR_LLENO 3
#define RANKING_ERROR_ARCHIVO 4
#define RANKING_ERROR_FORMATO 5

typedef struct {
    char nombre[MAX_NOMBRE_DE_USUARIO]; /**< ASCII terminado en '\0'. */
    int victorias;                   /**< Partidas ganadas. */
} RankingEntrada;

typedef struct {
    RankingEntrada entradas[RANKING_MAX_JUGADORES]; /**< Solo las primeras `cantidad` son validas. */
    int cantidad;                                   /**< Entradas validas. */
} Ranking;

/* Consulta. */
void ranking_inicializar(Ranking *r);
int ranking_cantidad(const Ranking *r);
const RankingEntrada *ranking_obtener(const Ranking *r, int indice);
int ranking_buscar(const Ranking *r, const char *nombre);
int ranking_obtener_victorias(const Ranking *r, const char *nombre);

/* Modificacion. */
int ranking_registrar_victoria(Ranking *r, const char *nombre);
void ranking_ordenar(Ranking *r);

/* Persistencia binaria. */
int ranking_cargar(Ranking *r, const char *ruta);
int ranking_guardar(const Ranking *r, const char *ruta);

/* Presentacion. */
const char *ranking_mensaje_error(int codigo);
void ranking_mostrar(const Ranking *r);
int ranking_mostrar_en(FILE *salida, const Ranking *r);

#endif /* BATALLA_NAVAL_RANKING_H */
