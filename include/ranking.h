#ifndef BATALLA_NAVAL_RANKING_H
#define BATALLA_NAVAL_RANKING_H

/*
 * ranking.h - Ranking de victorias por jugador (formato binario BNR1 v1).
 *
 * ===========================================================================
 * 1. QUE HACE ESTE MODULO
 * ===========================================================================
 * Mantiene en memoria cuantas partidas gano cada jugador, ordena el ranking
 * y lo persiste en un archivo BINARIO versionado. Es el modulo del integrante
 * 4 (ranking, datos y compilacion).
 *
 * La interfaz (ui_cli.c / futura ui_gui.c) NO implementa reglas: pide el
 * nombre, llama a ranking_registrar_victoria(), y traduce el codigo de
 * retorno con ranking_mensaje_error(). Toda validacion, orden y persistencia
 * vive en ranking.c.
 *
 * ===========================================================================
 * 2. POR QUE BINARIO Y NO TEXTO
 * ===========================================================================
 * El grupo decidio la "Opcion B": serializacion binaria explicita, con
 * version, campos de ancho fijo, endianness definido y checksum. Ventajas
 * frente a un fwrite() del struct:
 *   - No depende del padding que agregue cada compilador.
 *   - No depende del tamano de int/long de cada plataforma.
 *   - No depende del endianness de la CPU: se escribe little-endian a mano.
 *   - Incluye version: si el formato cambia, se detecta y se informa.
 *   - Incluye CRC32: detecta archivos truncados o alterados por accidente.
 *
 * IMPORTANTE (modelo de amenaza): el binario NO es seguridad. El codigo
 * fuente se entrega y el jugador es dueno del archivo; cualquiera puede
 * recompilar el formato y recalcular el CRC. El checksum protege contra
 * corrupcion accidental (corte, disco, edicion torpe), no contra trampa
 * deliberada. El anti-trampa real requeriria un servidor, fuera del alcance
 * de este trabajo. Esta distincion esta documentada en docs/contrato-ranking.md.
 *
 * ===========================================================================
 * 3. FORMATO BNR1 v1 (little-endian, offsets en bytes)
 * ===========================================================================
 *   Offset            Tamano   Campo
 *   ----------------  -------  -------------------------------------------
 *   0                 4        Magic: 'B' 'N' 'R' '1'
 *   4                 2        Version: 1 (uint16 little-endian)
 *   6                 2        Cantidad de jugadores (uint16 LE, 0..100)
 *   8                 36*n     Entradas, ordenadas de mejor a peor:
 *                                 +0   32  Nombre (NUL + relleno con 0)
 *                                 +32   4  Victorias (uint32 LE, 0..1000000)
 *   8+36*n            4        CRC32 LE de todos los bytes anteriores
 *
 *   Tamano total = 12 + 36*n.  Minimo (n=0): 12 bytes.
 *   Maximo (n=100): 3612 bytes.
 *
 * CRC32: polinomio IEEE 802.3 reflejado 0xEDB88320, init 0xFFFFFFFF,
 * XOR final 0xFFFFFFFF. Valor de control para "123456789": 0xCBF43926.
 * El CRC se calcula sobre TODO el archivo excepto los 4 bytes finales.
 *
 * Nombre: 1..31 bytes visibles, terminado en NUL y rellenado con ceros
 * hasta 32 bytes. Se rechazan caracteres de control (< 32). El ';' ya NO
 * es especial: en binario no hay separadores, cada nombre ocupa 32 bytes.
 * Para que el archivo sea canonico, el nombre guardado no lleva espacios
 * al principio ni al final (se recortan antes de escribir y se exige lo
 * mismo al leer).
 *
 * ===========================================================================
 * 4. ESCRITURA ATOMICA
 * ===========================================================================
 * ranking_guardar() NO escribe directamente sobre ranking.dat. Escribe
 * primero en "ranking.dat.tmp" (mismo directorio, mismo filesystem) y
 * despues hace rename() sobre el destino. En Linux rename() es atomico:
 * si el proceso muere a mitad de escritura, el ranking anterior queda
 * intacto y solo se pierde el temporal. No hay fsync(): la atomicidad es
 * frente a procesos, no frente a un corte de energia (ver documento).
 *
 * ===========================================================================
 * 5. PORTABILIDAD
 * ===========================================================================
 * Este header y ranking.c usan solo <stdio.h>, <string.h> y <ctype.h>.
 * No incluyen windows.h, unistd.h, conio.h ni stdint.h. El formato fija
 * 32 bits escribiendo bytes a mano con unsigned long, por lo que sigue
 * siendo ANSI C (C89) y no depende de que exista uint32_t.
 *
 * El destino de compilacion acordado es Linux con GCC/Clang. La escritura
 * atomica usa rename() de <stdio.h>, que en Linux reemplaza el destino.
 *
 * ===========================================================================
 * 6. USO TIPICO (fin de partida)
 * ===========================================================================
 *   Ranking r;
 *   int rc;
 *   ranking_inicializar(&r);
 *   rc = ranking_cargar(&r, CONFIG_RANKING_RUTA);
 *   if (rc == RANKING_ERROR_ARCHIVO) {
 *       // primera partida: no existia el archivo, se sigue con r vacio
 *   }
 *   rc = ranking_registrar_victoria(&r, ganador);
 *   if (rc == RANKING_OK) {
 *       rc = ranking_guardar(&r, CONFIG_RANKING_RUTA);
 *   }
 *   ranking_mostrar(&r);
 */

#include <stdio.h>

/* Limites del modelo en memoria. RANKING_NOMBRE_MAX incluye el '\0'. */
#define RANKING_NOMBRE_MAX 32
#define RANKING_MAX_JUGADORES 100
#define RANKING_RUTA_DEFAULT "data/ranking.dat"
#define RANKING_VICTORIAS_MAX 1000000

/* Constantes del formato binario BNR1 v1. */
#define RANKING_ARCHIVO_MAGIC "BNR1"
#define RANKING_ARCHIVO_VERSION 1
#define RANKING_NOMBRE_BYTES 32
#define RANKING_VICTORIAS_BYTES 4
#define RANKING_CABECERA_BYTES 8
#define RANKING_ENTRADA_BYTES (RANKING_NOMBRE_BYTES + RANKING_VICTORIAS_BYTES)
#define RANKING_CHECKSUM_BYTES 4
#define RANKING_ARCHIVO_MIN_BYTES (RANKING_CABECERA_BYTES + RANKING_CHECKSUM_BYTES)
#define RANKING_ARCHIVO_MAX_BYTES \
    (RANKING_CABECERA_BYTES + (RANKING_MAX_JUGADORES * RANKING_ENTRADA_BYTES) + RANKING_CHECKSUM_BYTES)

/* Codigos de retorno. 0 es exito; el resto son errores. */
#define RANKING_OK 0
#define RANKING_ERROR_ARGUMENTO 1      /* puntero NULL o struct inconsistente */
#define RANKING_ERROR_NOMBRE_INVALIDO 2/* nombre vacio, largo o con control */
#define RANKING_ERROR_LLENO 3          /* ya hay 100 jugadores */
#define RANKING_ERROR_ARCHIVO 4        /* no se pudo abrir/leer/escribir */
#define RANKING_ERROR_FORMATO 5        /* estructura binaria invalida */
#define RANKING_ERROR_INTEGRIDAD 6     /* CRC32 no coincide */
#define RANKING_ERROR_VERSION 7        /* magic o version desconocidos */

typedef struct {
    char nombre[RANKING_NOMBRE_MAX];
    int victorias;
} RankingEntrada;

typedef struct {
    RankingEntrada entradas[RANKING_MAX_JUGADORES];
    int cantidad;
} Ranking;

/* -------------------------------------------------------------------------
 * API publica
 * ------------------------------------------------------------------------- */

/* Deja el ranking vacio. Llamar antes de cualquier otro uso. */
void ranking_inicializar(Ranking *r);

/* Cantidad de jugadores (0 si r es NULL). */
int ranking_cantidad(const Ranking *r);

/*
 * Acceso de solo lectura al puesto `indice` (0 = lider).
 * Devuelve NULL si el indice esta fuera de rango o r es NULL.
 * El puntero apunta a memoria interna: no liberar ni modificar.
 */
const RankingEntrada *ranking_obtener(const Ranking *r, int indice);

/*
 * Busca un nombre con la regla de identidad del modulo: recorta espacios
 * y compara sin distinguir mayusculas. Devuelve el indice o -1.
 */
int ranking_buscar(const Ranking *r, const char *nombre);

/* Victorias de un jugador; 0 si no figura o si los argumentos son NULL. */
int ranking_obtener_victorias(const Ranking *r, const char *nombre);

/*
 * Suma UNA victoria al jugador; si no existe lo crea con 1.
 * Mantiene el orden y aplica tope en RANKING_VICTORIAS_MAX.
 * Retorna RANKING_OK, RANKING_ERROR_ARGUMENTO,
 * RANKING_ERROR_NOMBRE_INVALIDO o RANKING_ERROR_LLENO.
 */
int ranking_registrar_victoria(Ranking *r, const char *nombre);

/* Ordena in situ: victorias desc; empate, nombre asc insensible. */
void ranking_ordenar(Ranking *r);

/*
 * Carga un archivo BNR1 v1 desde `ruta`.
 * - Deja el ranking VACIO ante cualquier error (nunca deja datos a medias).
 * - RANKING_ERROR_ARCHIVO: no existe o no se pudo leer. Para la primera
 *   partida esto NO es fatal: significa "ranking vacio inicial".
 * - RANKING_ERROR_VERSION: no empieza con "BNR1" o la version no es 1.
 *   Un ranking.dat de la version texto anterior cae aqui.
 * - RANKING_ERROR_FORMATO: tamano, cantidad, nombre o victorias invalidos.
 * - RANKING_ERROR_INTEGRIDAD: el CRC32 no coincide (archivo corrupto).
 * - RANKING_ERROR_ARGUMENTO: r o ruta NULL.
 */
int ranking_cargar(Ranking *r, const char *ruta);

/*
 * Guarda el ranking en formato BNR1 v1 de forma atomica:
 * escribe `ruta`.tmp y luego rename() sobre `ruta`.
 * Si falla, el archivo anterior queda intacto y se borra el temporal.
 * Retorna RANKING_OK, RANKING_ERROR_ARGUMENTO o RANKING_ERROR_ARCHIVO.
 */
int ranking_guardar(const Ranking *r, const char *ruta);

/* Texto en espanol para mostrar al usuario a partir de un codigo. */
const char *ranking_mensaje_error(int codigo);

/* Muestra la tabla ordenada por stdout. */
void ranking_mostrar(const Ranking *r);

/* Igual que ranking_mostrar pero sobre un FILE (util para tests/GUI). */
int ranking_mostrar_en(FILE *salida, const Ranking *r);

#endif
