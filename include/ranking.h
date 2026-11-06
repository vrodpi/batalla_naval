/**
 * @file ranking.h
 * @brief Ranking de victorias por jugador (contrato publico).
 *
 * Mantiene en memoria el ranking (nombre + victorias), lo ordena y lo
 * persiste en "data/ranking.dat". Es el modulo del integrante 4 (ranking,
 * datos y compilacion). Solo depende de <stdio.h>: no usa nada especifico de
 * Windows ni de Linux, salvo el modo binario de fopen, que es estandar.
 *
 * @note Separacion logica / interfaz: este modulo valida, ordena y guarda.
 *       La interfaz pide el nombre, llama a ranking_registrar_victoria y
 *       muestra los mensajes de ranking_mensaje_error. Nunca toca `entradas`
 *       directamente.
 *
 * @section formato Formato binario del archivo
 * data/ranking.dat contiene exactamente sizeof(Ranking) bytes: una copia
 * cruda del struct Ranking, escrita con fwrite. Layout asumido (x86_64,
 * int de 4 bytes, sin relleno; ver static_assert en ranking.c):
 *
 *   offset    0: entradas[0..99]        100 x 36 bytes = 3600 bytes
 *                 +0   char nombre[32]  ASCII, '\0' final, resto en cero
 *                 +32  int  victorias   4 bytes (little-endian en x86)
 *   offset 3600: cantidad               int, 4 bytes
 *   ------------------------------------------------------------------
 *   TOTAL: 3604 bytes
 *
 * @note Se abre SIEMPRE en modo binario ("wb" para guardar, "rb" para leer).
 *       En modo texto, Windows traduce el byte 0x0A a 0x0D 0x0A y corrompe
 *       cualquier entero que contenga ese valor (por ejemplo, 10 victorias).
 * @note El equipo adopto el formato binario por indicacion de la catedra.
 *       Es una copia cruda del struct: NO tiene magic number, version ni
 *       checksum. Solo es portable donde el struct mida 3604 bytes y el int
 *       sea de 4 bytes little-endian. Un archivo corrupto se detecta por
 *       validacion de campos (tamano, cantidad, nombres, topes).
 *
 * @section reglas Reglas de datos
 * - Orden: victorias descendente; empate -> nombre ascendente insensible a
 *   mayusculas; desempate final sensible para determinismo.
 * - Nombres que solo difieren en mayusculas son el MISMO jugador. Se
 *   conserva la primera grafia guardada.
 * - Nombre valido: 1 a 31 caracteres tras recortar espacios; sin '\n',
 *   '\r' ni caracteres de control. El ';' SI se permite porque en binario no
 *   hay separadores de campo.
 * - Al guardar se normalizan a cero los bytes no usados, para que el archivo
 *   sea deterministico y no filtre contenido de la pila.
 * - Al cargar, una entrada invalida se saltea y se devuelve
 *   RANKING_ERROR_FORMATO, pero las validas se conservan.
 *
 * @section uso Uso tipico desde app.c (fin de partida)
 * @code
 * Ranking r;
 * ranking_inicializar(&r);
 * rc = ranking_cargar(&r, CONFIG_RANKING_RUTA);
 * if (rc == RANKING_ERROR_ARCHIVO) { // primera partida
 *     ...
 * }
 * ranking_registrar_victoria(&r, ganador);
 * ranking_guardar(&r, CONFIG_RANKING_RUTA);
 * ranking_mostrar(&r);
 * @endcode
 */

#ifndef BATALLA_NAVAL_RANKING_H
#define BATALLA_NAVAL_RANKING_H

#include <stdio.h>

/**
 * @brief Largo maximo del nombre de un jugador, incluido el '\0'.
 * @note Debe coincidir con CONFIG_NOMBRE_MAX (include/config.h).
 */
#define RANKING_NOMBRE_MAX 32

/**
 * @brief Cantidad maxima de jugadores distintos que admite el ranking.
 */
#define RANKING_MAX_JUGADORES 100

/**
 * @brief Ruta por defecto del archivo binario del ranking.
 */
#define RANKING_RUTA_DEFAULT "data/ranking.dat"

/**
 * @brief Tope de victorias por jugador para evitar desbordes de int.
 */
#define RANKING_VICTORIAS_MAX 1000000

/**
 * @brief Codigo de retorno: operacion exitosa.
 */
#define RANKING_OK 0

/**
 * @brief Codigo de retorno: puntero o argumento invalido.
 */
#define RANKING_ERROR_ARGUMENTO 1

/**
 * @brief Codigo de retorno: nombre vacio, demasiado largo o con controles.
 */
#define RANKING_ERROR_NOMBRE_INVALIDO 2

/**
 * @brief Codigo de retorno: ranking lleno (100 jugadores).
 */
#define RANKING_ERROR_LLENO 3

/**
 * @brief Codigo de retorno: no se pudo abrir, leer o escribir el archivo.
 */
#define RANKING_ERROR_ARCHIVO 4

/**
 * @brief Codigo de retorno: archivo binario invalido o corrupto.
 */
#define RANKING_ERROR_FORMATO 5

/**
 * @brief Entrada del ranking: un jugador y sus victorias.
 * @note El nombre debe estar terminado en '\0' dentro de los 32 bytes.
 *       El layout de esta struct forma parte del formato binario en disco.
 */
typedef struct {
    char nombre[RANKING_NOMBRE_MAX]; /**< Nombre terminado en '\0'. */
    int victorias;                   /**< Cantidad de partidas ganadas. */
} RankingEntrada;

/**
 * @brief Ranking completo en memoria.
 * @note Solo las primeras `cantidad` entradas son validas. El arreglo se
 *       mantiene ordenado despues de registrar o cargar.
 */
typedef struct {
    RankingEntrada entradas[RANKING_MAX_JUGADORES]; /**< Jugadores guardados. */
    int cantidad;                                   /**< Entradas validas. */
} Ranking;

/**
 * @brief Inicializa un ranking vacio.
 *
 * @param r Ranking que se quiere reiniciar.
 * @return No devuelve valor.
 * @note Deja cantidad = 0. Si r es NULL no hace nada.
 */
void ranking_inicializar(Ranking *r);

/**
 * @brief Devuelve la cantidad de jugadores distintos.
 *
 * @param r Ranking consultado.
 * @return Cantidad de entradas validas; 0 si r es NULL.
 * @note No modifica el ranking; es una consulta de solo lectura.
 */
int ranking_cantidad(const Ranking *r);

/**
 * @brief Acceso de solo lectura al puesto `indice` (0 = lider).
 *
 * @param r Ranking consultado.
 * @param indice Posicion solicitada, desde 0.
 * @return Puntero a la entrada interna; NULL si el indice esta fuera de
 *         rango o si r es NULL.
 * @note El puntero apunta a memoria interna del Ranking: no debe liberarse
 *       ni modificarse. Toda modificacion pasa por registrar_victoria.
 */
const RankingEntrada *ranking_obtener(const Ranking *r, int indice);

/**
 * @brief Busca un jugador por nombre.
 *
 * La comparacion ignora mayusculas y espacios de los extremos.
 *
 * @param r Ranking donde buscar.
 * @param nombre Nombre a buscar.
 * @return Indice de la entrada o -1 si no existe o el nombre es invalido.
 * @note La comparacion usa la misma normalizacion que registrar_victoria:
 *       recorta extremos y baja mayusculas antes de comparar.
 */
int ranking_buscar(const Ranking *r, const char *nombre);

/**
 * @brief Devuelve las victorias de un jugador.
 *
 * @param r Ranking consultado.
 * @param nombre Nombre del jugador.
 * @return Victorias registradas; 0 si no figura, si el nombre es invalido o
 *         si r es NULL.
 * @note No modifica el ranking; es una consulta de solo lectura.
 */
int ranking_obtener_victorias(const Ranking *r, const char *nombre);

/**
 * @brief Suma una victoria al jugador indicado; si no existe, lo crea.
 *
 * @param r Ranking que se modifica.
 * @param nombre Nombre del ganador.
 * @return RANKING_OK, RANKING_ERROR_ARGUMENTO,
 *         RANKING_ERROR_NOMBRE_INVALIDO o RANKING_ERROR_LLENO.
 * @note Ordena el ranking antes de volver. Es la unica via publica para
 *       modificar las entradas.
 */
int ranking_registrar_victoria(Ranking *r, const char *nombre);

/**
 * @brief Ordena el ranking in situ.
 *
 * Criterio: victorias descendente; a igual cantidad, nombre ascendente
 * insensible a mayusculas; desempate final sensible para determinismo.
 *
 * @param r Ranking que se ordena.
 * @return No devuelve valor.
 * @note Usa ordenamiento por insercion: con n <= 100 el costo O(n^2) es
 *       despreciable y el codigo queda corto y facil de seguir.
 */
void ranking_ordenar(Ranking *r);

/**
 * @brief Carga el ranking desde el archivo binario `ruta`.
 *
 * @param r Ranking que se reconstruye; se vacia antes de cargar.
 * @param ruta Ruta del archivo binario.
 * @return RANKING_OK si se leyo completo; RANKING_ERROR_ARCHIVO si el
 *         archivo no existe o fallo la E/S; RANKING_ERROR_FORMATO si el
 *         archivo esta vacio, truncado, tiene bytes de mas o contiene
 *         campos invalidos; RANKING_ERROR_ARGUMENTO si r o ruta son NULL.
 * @note Si el archivo no existe, el llamador debe tratarlo como "primera
 *       partida", no como error fatal. Las entradas validas que se hayan
 *       podido rescatar quedan cargadas (carga parcial).
 */
int ranking_cargar(Ranking *r, const char *ruta);

/**
 * @brief Guarda el ranking completo en `ruta`.
 *
 * @param r Ranking que se escribe (no se modifica).
 * @param ruta Ruta del archivo binario; se sobrescribe.
 * @return RANKING_OK, RANKING_ERROR_ARGUMENTO o RANKING_ERROR_ARCHIVO si no
 *         pudo crear o escribir el archivo.
 * @note Normaliza a cero los bytes no usados para que el archivo sea
 *       deterministico y no filtre memoria del proceso.
 */
int ranking_guardar(const Ranking *r, const char *ruta);

/**
 * @brief Traduce un codigo de retorno a texto en espanol.
 *
 * @param codigo Codigo devuelto por otra funcion del modulo.
 * @return Puntero a un texto constante; nunca NULL.
 * @note La interfaz puede imprimir el resultado directamente con %s.
 */
const char *ranking_mensaje_error(int codigo);

/**
 * @brief Muestra el ranking por stdout.
 *
 * @param r Ranking que se imprime.
 * @return No devuelve valor.
 * @note Para pruebas o GUI conviene usar ranking_cantidad/ranking_obtener y
 *       formatear en la capa de interfaz.
 */
void ranking_mostrar(const Ranking *r);

/**
 * @brief Muestra el ranking en el FILE indicado.
 *
 * @param salida Flujo de salida ya abierto.
 * @param r Ranking que se imprime.
 * @return RANKING_OK, o RANKING_ERROR_ARGUMENTO si salida o r son NULL.
 * @note Existe la variante con FILE para poder redirigir la salida en las
 *       pruebas sin depender de stdout.
 */
int ranking_mostrar_en(FILE *salida, const Ranking *r);

#endif /* BATALLA_NAVAL_RANKING_H */
