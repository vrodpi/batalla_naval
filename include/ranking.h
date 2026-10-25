#ifndef BATALLA_NAVAL_RANKING_H
#define BATALLA_NAVAL_RANKING_H

/*
 * ranking.h - Ranking de victorias por jugador.
 *
 * ---------------------------------------------------------------------------
 * RESPONSABILIDAD
 * ---------------------------------------------------------------------------
 * Mantener en memoria el ranking (nombre + victorias), ordenarlo y
 * persistirlo en "data/ranking.dat". Es el modulo del integrante 4
 * (ranking, datos y compilacion). Solo depende de <stdio.h>: no usa nada
 * especifico de Windows ni de Linux, salvo el modo binario de fopen, que
 * es estandar.
 *
 * Separacion logica / interfaz (Anexo A de la consigna):
 * - Este modulo es LOGICA + persistencia: valida, ordena y guarda. No
 *   pide datos por teclado.
 * - La interfaz (ui_cli.c / futura ui_gui.c) pide el nombre, llama a
 *   ranking_registrar_victoria y muestra los mensajes que devuelve
 *   ranking_mensaje_error. Nunca toca `entradas` directamente.
 *
 * ---------------------------------------------------------------------------
 * FORMATO DEL ARCHIVO: BINARIO (VOLCADO DIRECTO DEL STRUCT)
 * ---------------------------------------------------------------------------
 * data/ranking.dat contiene exactamente sizeof(Ranking) bytes: una copia
 * cruda del struct Ranking, escrita con fwrite. Layout asumido (x86_64,
 * int de 4 bytes, sin relleno; ver asserts en ranking.c):
 *
 *   offset    0: entradas[0..99]        100 x 36 bytes = 3600 bytes
 *                 +0   char nombre[32]  ASCII, '\0' final, resto en cero
 *                 +32  int  victorias   4 bytes (little-endian en x86)
 *   offset 3600: cantidad               int, 4 bytes
 *   ------------------------------------------------------------------
 *   TOTAL: 3604 bytes
 *
 * Se abre SIEMPRE en modo binario ("wb" para guardar, "rb" para leer).
 * En modo texto, Windows traduce el byte 0x0A a 0x0D 0x0A y corrompe
 * cualquier entero que contenga ese valor (por ejemplo, 10 victorias).
 *
 * ---------------------------------------------------------------------------
 * POR QUE BINARIO (decision del grupo) Y QUE SE ASUME
 * ---------------------------------------------------------------------------
 * El equipo adopto el formato binario por indicacion de la catedra
 * ("la informacion importante va en archivos binarios"). Es una copia
 * cruda del struct: NO tiene magic number, version ni checksum.
 *
 * Consecuencias que deben mencionarse en la defensa:
 * - Solo es portable entre plataformas/compiladores donde el struct mida
 *   3604 bytes y el int sea de 4 bytes little-endian (Windows x64 y
 *   Linux x86_64 cumplen). En otra arquitectura el archivo no se puede
 *   intercambiar. Los asserts de ranking.c hacen fallar la compilacion
 *   si el layout cambia.
 * - No protege contra edicion deliberada: quien tiene el codigo fuente
 *   puede reescribir el archivo con 3 lineas de C. El binario solo evita
 *   edits accidentales en un editor de texto.
 * - Un archivo de una version futura o corrupto se detecta unicamente
 *   por validacion de campos (tamano, cantidad, nombres, topes), no por
 *   un checksum.
 * - Contradice la recomendacion §8 del documento general, que sugeria
 *   texto explicito. Queda documentado como decision consciente.
 *
 * ---------------------------------------------------------------------------
 * REGLAS DE DATOS (no cambian respecto de la version texto)
 * ---------------------------------------------------------------------------
 * - Orden: victorias descendente; empate -> nombre ascendente insensible
 *   a mayusculas; desempate final sensible para determinismo.
 * - Nombres que solo difieren en mayusculas son el MISMO jugador. Se
 *   conserva la primera grafia guardada.
 * - Nombre valido: 1 a 31 caracteres tras recortar espacios; sin '\n',
 *   '\r' ni caracteres de control. El ';' SI se permite: en binario no
 *   hay separadores de campo (a diferencia del formato texto anterior).
 * - Al guardar se normalizan a cero los bytes no usados (slots libres y
 *   cola de cada nombre), para que el archivo sea deterministico y no
 *   filtre contenido de la pila.
 * - Al cargar, una entrada invalida se saltea y se devuelve
 *   RANKING_ERROR_FORMATO, pero las validas se conservan.
 *
 * ---------------------------------------------------------------------------
 * USO TIPICO DESDE app.c (fin de partida)
 * ---------------------------------------------------------------------------
 *   Ranking r;
 *   ranking_inicializar(&r);
 *   rc = ranking_cargar(&r, CONFIG_RANKING_RUTA);
 *   if (rc == RANKING_ERROR_ARCHIVO) {  / * primera partida * / }
 *   ranking_registrar_victoria(&r, ganador);
 *   ranking_guardar(&r, CONFIG_RANKING_RUTA);
 *   ranking_mostrar(&r);
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

/* Inicializa un ranking vacio (cantidad = 0). */
void ranking_inicializar(Ranking *r);

/* Cantidad de jugadores distintos (0 si r es NULL). */
int ranking_cantidad(const Ranking *r);

/*
 * Acceso de solo lectura al puesto `indice` (0 = lider), o NULL si el
 * indice esta fuera de rango.
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
 * Carga el ranking desde el archivo binario `ruta`.
 * - Si el archivo no existe: ranking vacio + RANKING_ERROR_ARCHIVO
 *   (el llamador lo trata como "primera partida", no como error fatal).
 * - Si el archivo esta vacio, truncado, tiene bytes de mas, o su
 *   cantidad/nombre/victorias son invalidos: RANKING_ERROR_FORMATO.
 *   Las entradas validas que se hayan podido rescatar quedan cargadas.
 * - Devuelve RANKING_ERROR_ARGUMENTO si r o ruta son NULL.
 */
int ranking_cargar(Ranking *r, const char *ruta);

/*
 * Guarda el ranking completo en `ruta` (sobrescribe, modo binario).
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
