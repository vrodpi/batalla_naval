#ifndef BATALLA_NAVAL_RANKING_H
#define BATALLA_NAVAL_RANKING_H

/*
 * ranking.h - Ranking de victorias por jugador.
 *
 * Responsabilidad: mantener en memoria el ranking (nombre + victorias),
 * ordenarlo, y persistirlo en "data/ranking.dat" con un formato de texto
 * portable (no se escribe el struct binario con fwrite).
 *
 * Formato del archivo (una entrada por linea):
 *     nombre;victorias\n
 * Ejemplo:
 *     Ana;3
 *     Luis;1
 *
 * Reglas acordadas por el grupo (propuesta para cerrar "decisiones
 * pendientes", ver docs/contrato-ranking.md):
 * - Orden: victorias descendente, luego nombre ascendente insensible
 *   a mayusculas (desempate determinista).
 * - Nombres que solo difieren en mayusculas/minusculas son el MISMO
 *   jugador (comparacion insensible a mayusculas). Se conserva la
 *   primera grafia guardada.
 * - Nombre valido: 1 a 31 caracteres visibles luego de recortar
 *   espacios de los extremos; no puede contener ';', ni '\n' ni '\r'.
 *
 * Separacion logica/interfaz (Anexo A):
 * - Este modulo es LOGICA + persistencia: valida, ordena y guarda.
 *   No pide datos por teclado (salvo ranking_mostrar, que solo lista).
 * - La interfaz (ui_cli.c / futura ui_gui.c) pide el nombre, llama a
 *   ranking_registrar_victoria, y decide que mensajes mostrar a partir
 *   del codigo de retorno y de ranking_mensaje_error.
 *
 * Portabilidad: solo usa <stdio.h>, <string.h>, <ctype.h>.
 * No incluye windows.h, unistd.h ni conio.h.
 */

#include <stdio.h>

#define RANKING_NOMBRE_MAX 32
#define RANKING_MAX_JUGADORES 100
#define RANKING_RUTA_DEFAULT "data/ranking.dat"
#define RANKING_VICTORIAS_MAX 1000000

/* Codigos de retorno. 0 es exito. */
#define RANKING_OK 0
#define RANKING_ERROR_ARGUMENTO 1
#define RANKING_ERROR_NOMBRE_INVALIDO 2
#define RANKING_ERROR_LLENO 3
#define RANKING_ERROR_ARCHIVO 4
#define RANKING_ERROR_FORMATO 5

typedef struct {
    char nombre[RANKING_NOMBRE_MAX];
    int victorias;
} RankingEntrada;

typedef struct {
    RankingEntrada entradas[RANKING_MAX_JUGADORES];
    int cantidad;
} Ranking;

/* Inicializa un ranking vacio. */
void ranking_inicializar(Ranking *r);

/* Cantidad de jugadores distintos (0 si r es NULL). */
int ranking_cantidad(const Ranking *r);

/*
 * Devuelve puntero interno a la entrada indice (0 = mejor posicion),
 * o NULL si indice fuera de rango. El puntero es de solo lectura.
 */
const RankingEntrada *ranking_obtener(const Ranking *r, int indice);

/*
 * Busca un nombre (insensible a mayusculas, ignorando espacios de los
 * extremos). Devuelve el indice o -1 si no existe.
 */
int ranking_buscar(const Ranking *r, const char *nombre);

/* Victorias de un jugador, o 0 si no figura. */
int ranking_obtener_victorias(const Ranking *r, const char *nombre);

/*
 * Suma una victoria al jugador indicado; si no existe, lo crea.
 * Ordena el ranking antes de volver.
 * Devuelve RANKING_OK, RANKING_ERROR_ARGUMENTO,
 * RANKING_ERROR_NOMBRE_INVALIDO o RANKING_ERROR_LLENO.
 */
int ranking_registrar_victoria(Ranking *r, const char *nombre);

/* Ordena in situ: victorias desc, luego nombre asc (insensible). */
void ranking_ordenar(Ranking *r);

/*
 * Carga el ranking desde ruta (formato "nombre;victorias").
 * - Si el archivo no existe: deja el ranking vacio y devuelve
 *   RANKING_ERROR_ARCHIVO (el llamador puede tratarlo como "ranking
 *   vacio inicial", no como error fatal).
 * - Lineas en blanco se ignoran. Lineas malformadas se saltean y al
 *   final se devuelve RANKING_ERROR_FORMATO (pero se conservan las
 *   lineas validas). Duplicados insensibles a mayusculas se fusionan
 *   sumando victorias.
 * - Devuelve RANKING_ERROR_ARGUMENTO si r o ruta son NULL.
 */
int ranking_cargar(Ranking *r, const char *ruta);

/*
 * Guarda el ranking en ruta (texto "nombre;victorias\n").
 * Devuelve RANKING_OK o RANKING_ERROR_ARCHIVO si no pudo crear o
 * escribir el archivo. No modifica el ranking en memoria.
 */
int ranking_guardar(const Ranking *r, const char *ruta);

/* Texto descriptivo en espanol para un codigo de retorno. */
const char *ranking_mensaje_error(int codigo);

/*
 * Muestra el ranking por stdout (lista ordenada). No modifica nada.
 * Para pruebas o GUI, preferir ranking_cantidad/ranking_obtener y
 * formatear en la capa de interfaz.
 */
void ranking_mostrar(const Ranking *r);

/* Igual que ranking_mostrar pero sobre el FILE indicado. */
int ranking_mostrar_en(FILE *salida, const Ranking *r);

#endif
