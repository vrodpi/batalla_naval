/*
 * ranking.c - Ranking de victorias por jugador (logica + persistencia BNR1).
 *
 * ===========================================================================
 * 1. VISION GENERAL
 * ===========================================================================
 * Este archivo implementa el contrato de ranking.h. Se ocupa de:
 *   - decidir que nombres son validos y cuando dos nombres son el mismo
 *     jugador (sin distinguir mayusculas ni espacios de los extremos);
 *   - sumar victorias y mantener el orden del ranking;
 *   - serializar el ranking en un archivo binario versionado (BNR1 v1);
 *   - verificar el CRC32 al leer y escribir de forma atomica.
 *
 * No pide datos por teclado ni imprime menus: eso es responsabilidad de
 * ui_cli.c. La unica funcion que imprime es ranking_mostrar(), a pedido
 * explicito de la consigna (pantalla de ranking).
 *
 * ===========================================================================
 * 2. MODELO EN MEMORIA
 * ===========================================================================
 *   typedef struct { char nombre[32]; int victorias; } RankingEntrada;
 *   typedef struct { RankingEntrada entradas[100]; int cantidad; } Ranking;
 *
 * - El Ranking vive en la pila de app.c; no hay malloc/free.
 * - INVARIANTE 1: solo las primeras `cantidad` posiciones son validas.
 * - INVARIANTE 2: despues de registrar o cargar, el arreglo queda
 *   ordenado de mejor a peor. Por eso mostrar() solo recorre e imprime.
 * - `nombre` conserva la grafia original (recortada). Las comparaciones
 *   usan comparar_insensible(), que baja a minusculas con tolower().
 *
 * ===========================================================================
 * 3. FORMATO BINARIO BNR1 v1 (little-endian)
 * ===========================================================================
 *   offset  tam     campo
 *   0       4       'B' 'N' 'R' '1'
 *   4       2       version = 1            (uint16 LE)
 *   6       2       cantidad 0..100        (uint16 LE)
 *   8       36*n    entradas:
 *                     +0   32  nombre (NUL + relleno de ceros)
 *                     +32   4  victorias 0..1000000 (uint32 LE)
 *   8+36*n  4       CRC32 LE de los bytes [0, 8+36*n)
 *
 * Tamano total = 12 + 36*n; maximo 3612 bytes (n=100).
 * No se hace fwrite() del struct: se arma un buffer byte a byte. Asi el
 * archivo no depende del padding, del sizeof(int) ni del endianness de la
 * maquina. El CRC32 es el de IEEE 802.3 (polinomio reflejado 0xEDB88320).
 *
 * ===========================================================================
 * 4. ESCRITURA ATOMICA
 * ===========================================================================
 * ranking_guardar() escribe en "ruta.tmp" y despues rename() sobre "ruta".
 * En Linux rename() reemplaza el destino de forma atomica dentro del mismo
 * filesystem. Si falla cualquier paso, se borra el temporal y el archivo
 * anterior queda intacto. No se hace fsync(): se protege contra procesos
 * interrumpidos, no contra un corte de energia (eso requeriria POSIX).
 *
 * ===========================================================================
 * 5. MAPA DE FUNCIONES
 * ===========================================================================
 * Ayudas internas (static):
 *   recortar_extremos      -> trim de espacios;
 *   comparar_insensible    -> strcasecmp portable;
 *   nombre_normalizado     -> valida y canoniza un nombre;
 *   escribir_u16/u32       -> enteros a bytes little-endian;
 *   leer_u16/u32           -> bytes little-endian a enteros;
 *   crc32_calcular         -> CRC32 IEEE;
 *   escribir_nombre        -> nombre a 32 bytes fijos;
 *   leer_nombre            -> 32 bytes fijos a nombre validado;
 *   construir_ruta_temporal-> "ruta" + ".tmp";
 *   leer_archivo_binario   -> lectura completa y acotada del archivo.
 *
 * API publica: ranking_inicializar, ranking_cantidad, ranking_obtener,
 * ranking_buscar, ranking_obtener_victorias, ranking_ordenar,
 * ranking_registrar_victoria, ranking_cargar, ranking_guardar,
 * ranking_mensaje_error, ranking_mostrar_en, ranking_mostrar.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ranking.h"

/*
 * Buffer maximo para construir "ruta.tmp". 512 alcanza para cualquier
 * ruta razonable del proyecto; si no alcanza, guardar devuelve
 * RANKING_ERROR_ARCHIVO sin tocar el archivo original.
 */
#define RANKING_RUTA_MAX 512

/* ===========================================================================
 * AYUDAS DE TEXTO
 * =========================================================================== */

/*
 * recortar_extremos: copia `origen` en `destino` sin espacios en blanco
 * de los extremos ("  Ana \t" -> "Ana").
 *
 * Algoritmo con dos punteros:
 *   1. `ini` avanza mientras isspace() sea verdadero.
 *   2. Si solo habia espacios, el resultado es "".
 *   3. `fin` retrocede desde el '\0' mientras haya espacios.
 *   4. Se copian los bytes [ini, fin), truncando a tam_destino-1 si no
 *      entran, y siempre se agrega '\0'.
 *
 * isspace() se llama con (unsigned char) porque pasarle un char negativo
 * (letras acentuadas en Latin-1/UTF-8) seria comportamiento indefinido.
 */
static void recortar_extremos(const char *origen, char *destino, int tam_destino)
{
    const char *ini;
    const char *fin;
    int n;
    int i;

    if (origen == NULL || destino == NULL || tam_destino <= 0) {
        return;
    }
    ini = origen;
    while (*ini != '\0' && isspace((unsigned char)*ini)) {
        ini++;
    }
    if (*ini == '\0') {
        destino[0] = '\0';
        return;
    }
    fin = ini + strlen(ini);
    while (fin > ini && isspace((unsigned char)*(fin - 1))) {
        fin--;
    }
    n = (int)(fin - ini);
    if (n >= tam_destino) {
        n = tam_destino - 1;
    }
    for (i = 0; i < n; i++) {
        destino[i] = ini[i];
    }
    destino[n] = '\0';
}

/*
 * comparar_insensible: compara dos strings ignorando mayusculas.
 * Es el equivalente portable de strcasecmp(): ninguna de esas funciones
 * es ANSI C (strcasecmp es POSIX y stricmp es de Windows), asi que se
 * implementa con tolower() de <ctype.h>.
 *
 * Devuelve 0 si son iguales, <0 si a<b, >0 si a>b (convencion strcmp).
 * Los NULL se ordenan antes que cualquier string para no romper.
 */
static int comparar_insensible(const char *a, const char *b)
{
    unsigned char ca;
    unsigned char cb;

    if (a == NULL && b == NULL) {
        return 0;
    }
    if (a == NULL) {
        return -1;
    }
    if (b == NULL) {
        return 1;
    }
    while (*a != '\0' && *b != '\0') {
        ca = (unsigned char)tolower((unsigned char)*a);
        cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) {
            return (ca < cb) ? -1 : 1;
        }
        a++;
        b++;
    }
    if (*a == '\0' && *b == '\0') {
        return 0;
    }
    if (*a == '\0') {
        return -1;
    }
    return 1;
}

/*
 * nombre_normalizado: valida un nombre y devuelve su forma canonica.
 *
 * Reglas (compartidas por registrar, buscar y cargar):
 *  1. Recortar espacios de los extremos.
 *  2. Largo real 1..31 (RANKING_NOMBRE_MAX-1).
 *  3. Sin caracteres de control (< 32): eso incluye '\n', '\r' y '\t'.
 *     El ';' SI se permite: en binario no hay separadores; cada nombre
 *     ocupa 32 bytes fijos. (En la version texto anterior estaba prohibido
 *     porque era el separador del archivo.)
 *
 * Por que se mide dos veces: recortar_extremos() trunca en silencio si el
 * texto no entra en el buffer. Si solo midieramos `tmp`, un nombre de 200
 * caracteres pasaria como valido de 31. La segunda pasada mide el largo
 * real sobre el original (punteros ini/fin, sin copiar) y rechaza todo lo
 * que sea <=0 o >= RANKING_NOMBRE_MAX.
 *
 * Devuelve 1 y completa `salida` si es valido; 0 si no.
 */
static int nombre_normalizado(const char *nombre, char *salida, int tam_salida)
{
    char tmp[RANKING_NOMBRE_MAX];
    int i;
    int largo;

    if (nombre == NULL || salida == NULL || tam_salida <= 0) {
        return 0;
    }
    recortar_extremos(nombre, tmp, (int)sizeof(tmp));

    {
        const char *ini;
        const char *fin;
        int real;

        ini = nombre;
        while (*ini != '\0' && isspace((unsigned char)*ini)) {
            ini++;
        }
        fin = ini + strlen(ini);
        while (fin > ini && isspace((unsigned char)*(fin - 1))) {
            fin--;
        }
        real = (int)(fin - ini);
        if (real <= 0 || real >= RANKING_NOMBRE_MAX) {
            return 0;
        }
    }

    largo = (int)strlen(tmp);
    if (largo <= 0 || largo >= RANKING_NOMBRE_MAX) {
        return 0;
    }
    for (i = 0; i < largo; i++) {
        if ((unsigned char)tmp[i] < 32) {
            return 0;
        }
    }
    for (i = 0; i <= largo; i++) {
        if (i < tam_salida) {
            salida[i] = tmp[i];
        }
    }
    if (tam_salida > 0) {
        salida[tam_salida - 1] = '\0';
    }
    return 1;
}

/* ===========================================================================
 * AYUDAS DE SERIALIZACION BINARIA
 * =========================================================================== */

/*
 * escribir_u16: guarda `v` en 2 bytes little-endian.
 * Se enmascara con 0xFF para no depender del tamano de unsigned int.
 */
static void escribir_u16(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v & 0xFFU);
    p[1] = (unsigned char)((v >> 8) & 0xFFU);
}

/*
 * escribir_u32: guarda `v` en 4 bytes little-endian.
 * `unsigned long` tiene al menos 32 bits en C89; se escriben los 4 bytes
 * bajos y se descarta cualquier bit alto. Asi el formato queda fijo en
 * 32 bits aunque en Linux long sea de 64 bits.
 */
static void escribir_u32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)(v & 0xFFUL);
    p[1] = (unsigned char)((v >> 8) & 0xFFUL);
    p[2] = (unsigned char)((v >> 16) & 0xFFUL);
    p[3] = (unsigned char)((v >> 24) & 0xFFUL);
}

/* leer_u16: reconstruye un unsigned int desde 2 bytes little-endian. */
static unsigned int leer_u16(const unsigned char *p)
{
    return ((unsigned int)p[0]) |
           (((unsigned int)p[1]) << 8);
}

/* leer_u32: reconstruye un unsigned long desde 4 bytes little-endian. */
static unsigned long leer_u32(const unsigned char *p)
{
    return ((unsigned long)p[0]) |
           (((unsigned long)p[1]) << 8) |
           (((unsigned long)p[2]) << 16) |
           (((unsigned long)p[3]) << 24);
}

/*
 * crc32_calcular: CRC32 IEEE 802.3 de `n` bytes.
 *
 * Parametros: polinomio reflejado 0xEDB88320, valor inicial 0xFFFFFFFF
 * y XOR final 0xFFFFFFFF. Se procesa byte por byte, bit por bit, sin
 * tabla: para archivos de hasta 3.6 KB el costo es despreciable y el
 * codigo queda corto y auditable. Valor de control: CRC32("123456789")
 * = 0xCBF43926.
 *
 * El resultado se enmascara a 32 bits porque en Linux unsigned long es
 * de 64 bits; el formato guarda exactamente 4 bytes.
 */
static unsigned long crc32_calcular(const unsigned char *datos, int n)
{
    unsigned long crc;
    int i;
    int bit;

    crc = 0xFFFFFFFFUL;
    for (i = 0; i < n; i++) {
        crc ^= (unsigned long)datos[i];
        for (bit = 0; bit < 8; bit++) {
            if (crc & 1UL) {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return (crc ^ 0xFFFFFFFFUL) & 0xFFFFFFFFUL;
}

/*
 * escribir_nombre: copia un nombre a un campo fijo de 32 bytes.
 * Primero llena todo con 0 (relleno canonico) y luego copia hasta 31
 * caracteres. El escritor ya valido el nombre, asi que el truncado es
 * solo una defensa extra.
 */
static void escribir_nombre(unsigned char *destino, const char *nombre)
{
    int i;
    int n;

    for (i = 0; i < RANKING_NOMBRE_BYTES; i++) {
        destino[i] = 0;
    }
    n = (int)strlen(nombre);
    if (n > RANKING_NOMBRE_BYTES - 1) {
        n = RANKING_NOMBRE_BYTES - 1;
    }
    for (i = 0; i < n; i++) {
        destino[i] = (unsigned char)nombre[i];
    }
}

/*
 * leer_nombre: convierte 32 bytes fijos en un nombre validado.
 *
 * Pasos:
 *  1. Buscar el primer NUL. Debe existir y no estar en la posicion 0
 *     (un nombre vacio no es valido). Si no hay NUL en 32 bytes, el
 *     campo esta corrupto.
 *  2. Exigir que todos los bytes posteriores al NUL sean 0. Esto es el
 *     relleno canonico que escribe escribir_nombre(); si hay basura,
 *     el archivo no lo genero este programa y se rechaza.
 *  3. Copiar a `crudo`, validar con nombre_normalizado() y exigir que
 *     la forma canonica sea identica a `crudo` (sin espacios extremos).
 *     Asi " Ana" o "Ana " se rechazan: el escritor nunca los produce.
 *
 * Devuelve 1 si el nombre es valido y canonico; 0 si no.
 */
static int leer_nombre(const unsigned char *origen, char *destino, int tam_destino)
{
    char crudo[RANKING_NOMBRE_MAX];
    char norm[RANKING_NOMBRE_MAX];
    int fin;
    int i;

    if (origen == NULL || destino == NULL || tam_destino < RANKING_NOMBRE_MAX) {
        return 0;
    }
    fin = -1;
    for (i = 0; i < RANKING_NOMBRE_BYTES; i++) {
        if (origen[i] == 0) {
            fin = i;
            break;
        }
    }
    if (fin < 1) {
        return 0;
    }
    for (i = fin; i < RANKING_NOMBRE_BYTES; i++) {
        if (origen[i] != 0) {
            return 0;
        }
    }
    for (i = 0; i < fin; i++) {
        crudo[i] = (char)origen[i];
    }
    crudo[fin] = '\0';
    if (!nombre_normalizado(crudo, norm, (int)sizeof(norm))) {
        return 0;
    }
    if (strcmp(crudo, norm) != 0) {
        return 0;
    }
    for (i = 0; i <= fin; i++) {
        destino[i] = norm[i];
    }
    return 1;
}

/*
 * construir_ruta_temporal: arma "ruta.tmp" en `destino`.
 * El temporal debe quedar en el MISMO directorio que el destino para que
 * rename() sea atomico (rename entre filesystems distintos no lo es).
 * Devuelve 1 si entra en el buffer; 0 si la ruta es demasiado larga.
 */
static int construir_ruta_temporal(const char *ruta, char *destino, int tam_destino)
{
    int n;
    int i;

    if (ruta == NULL || destino == NULL || tam_destino <= 0) {
        return 0;
    }
    n = (int)strlen(ruta);
    if (n + 4 >= tam_destino) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        destino[i] = ruta[i];
    }
    destino[n] = '.';
    destino[n + 1] = 't';
    destino[n + 2] = 'm';
    destino[n + 3] = 'p';
    destino[n + 4] = '\0';
    return 1;
}

/*
 * leer_archivo_binario: lee el archivo COMPLETO en `buf`, de forma
 * acotada. Como el formato tiene un tamano maximo conocido (3612 bytes),
 * se puede leer todo en memoria y verificar el CRC antes de aceptar una
 * sola entrada. Esto evita dejar el ranking a medio cargar si el archivo
 * esta corrupto.
 *
 * Pasos:
 *  1. Abrir en modo binario "rb" (en Linux es equivalente a "r", pero
 *     deja explicita la intencion y evita traducciones en otros sistemas).
 *  2. fseek al final + ftell para conocer el tamano.
 *  3. Rechazar solo tamanos mayores al maximo. Los archivos chicos se
 *     leen igual para poder distinguir "formato viejo" (magic distinto)
 *     de "archivo truncado" (magic correcto pero faltan bytes).
 *  4. Volver al inicio y fread exacto. Un short read es error de archivo.
 *
 * Devuelve RANKING_OK y `tam_leido`, o un codigo de error.
 */
static int leer_archivo_binario(const char *ruta, unsigned char *buf,
                                int tam_max, int *tam_leido)
{
    FILE *f;
    long tam;
    size_t leidos;

    if (ruta == NULL || buf == NULL || tam_leido == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }
    f = fopen(ruta, "rb");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }
    if (fseek(f, 0L, SEEK_END) != 0) {
        fclose(f);
        return RANKING_ERROR_ARCHIVO;
    }
    tam = ftell(f);
    if (tam < 0L) {
        fclose(f);
        return RANKING_ERROR_ARCHIVO;
    }
    if (tam > (long)tam_max) {
        fclose(f);
        return RANKING_ERROR_FORMATO;
    }
    if (fseek(f, 0L, SEEK_SET) != 0) {
        fclose(f);
        return RANKING_ERROR_ARCHIVO;
    }
    leidos = fread(buf, 1, (size_t)tam, f);
    if (leidos != (size_t)tam) {
        fclose(f);
        return RANKING_ERROR_ARCHIVO;
    }
    if (fclose(f) != 0) {
        return RANKING_ERROR_ARCHIVO;
    }
    *tam_leido = (int)tam;
    return RANKING_OK;
}

/* ===========================================================================
 * API PUBLICA: ESTADO EN MEMORIA
 * =========================================================================== */

/*
 * ranking_inicializar: deja cantidad = 0.
 * Es lo que garantiza el INVARIANTE 1: las posiciones >= cantidad no se
 * leen hasta que registrar/cargar las complete.
 */
void ranking_inicializar(Ranking *r)
{
    if (r == NULL) {
        return;
    }
    r->cantidad = 0;
}

/* ranking_cantidad: cuantos jugadores hay; 0 si r es NULL. */
int ranking_cantidad(const Ranking *r)
{
    if (r == NULL) {
        return 0;
    }
    return r->cantidad;
}

/*
 * ranking_obtener: acceso de solo lectura al puesto `indice`.
 * Valida el rango para que la interfaz nunca lea basura. El puntero
 * devuelto apunta al arreglo interno: no se libera ni se modifica.
 */
const RankingEntrada *ranking_obtener(const Ranking *r, int indice)
{
    if (r == NULL) {
        return NULL;
    }
    if (indice < 0 || indice >= r->cantidad) {
        return NULL;
    }
    return &r->entradas[indice];
}

/*
 * ranking_buscar: busqueda lineal por identidad insensible a mayusculas.
 * Con maximo 100 jugadores, O(n) es despreciable y evita estructuras
 * extra. Un nombre invalido nunca puede estar guardado, por eso devuelve
 * -1 sin recorrer.
 */
int ranking_buscar(const Ranking *r, const char *nombre)
{
    char norm[RANKING_NOMBRE_MAX];
    int i;

    if (r == NULL || nombre == NULL) {
        return -1;
    }
    if (!nombre_normalizado(nombre, norm, (int)sizeof(norm))) {
        return -1;
    }
    for (i = 0; i < r->cantidad; i++) {
        if (comparar_insensible(r->entradas[i].nombre, norm) == 0) {
            return i;
        }
    }
    return -1;
}

/* ranking_obtener_victorias: consulta directa; 0 si no figura. */
int ranking_obtener_victorias(const Ranking *r, const char *nombre)
{
    int idx;

    idx = ranking_buscar(r, nombre);
    if (idx < 0) {
        return 0;
    }
    return r->entradas[idx].victorias;
}

/*
 * ranking_ordenar: ordenamiento por insercion, in situ.
 *
 * Criterio compuesto:
 *   1. mas victorias primero;
 *   2. a igual victorias, nombre ascendente insensible a mayusculas;
 *   3. a igual todo lo anterior, strcmp sensible para determinismo.
 *
 * Invariante: antes de procesar la posicion i, el prefijo [0..i-1] ya
 * esta ordenado. Se guarda entradas[i] en `tmp`, se desplazan a la
 * derecha los anteriores que van despues, y se inserta tmp en el hueco.
 * Con n <= 100 el peor caso (~10.000 comparaciones) es instantaneo.
 * La asignacion de structs es valida en ANSI C.
 */
void ranking_ordenar(Ranking *r)
{
    int i;
    int j;
    RankingEntrada tmp;

    if (r == NULL) {
        return;
    }
    for (i = 1; i < r->cantidad; i++) {
        tmp = r->entradas[i];
        j = i - 1;
        while (j >= 0) {
            int debe_mover;
            int cmp;

            debe_mover = 0;
            if (r->entradas[j].victorias < tmp.victorias) {
                debe_mover = 1;
            } else if (r->entradas[j].victorias == tmp.victorias) {
                cmp = comparar_insensible(r->entradas[j].nombre, tmp.nombre);
                if (cmp > 0) {
                    debe_mover = 1;
                } else if (cmp == 0) {
                    if (strcmp(r->entradas[j].nombre, tmp.nombre) > 0) {
                        debe_mover = 1;
                    }
                }
            }
            if (!debe_mover) {
                break;
            }
            r->entradas[j + 1] = r->entradas[j];
            j--;
        }
        r->entradas[j + 1] = tmp;
    }
}

/*
 * ranking_registrar_victoria: suma UNA victoria al jugador.
 *
 * Flujo:
 *  1. Validar punteros y canonizar el nombre. Si es invalido, no se toca
 *     el ranking y se devuelve RANKING_ERROR_NOMBRE_INVALIDO.
 *  2. Buscar (insensible a mayusculas). Si existe, incrementar con tope
 *     en RANKING_VICTORIAS_MAX: al llegar al maximo se queda ahi en vez
 *     de desbordar el int.
 *  3. Si no existe y hay lugar, copiar el nombre canonico, victorias=1
 *     y cantidad++. Si esta lleno, RANKING_ERROR_LLENO.
 *  4. Reordenar siempre para mantener el INVARIANTE 2.
 *
 * La copia del nombre se hace caracter por caracter; el buffer de destino
 * tiene RANKING_NOMBRE_MAX y el nombre ya fue validado con ese limite.
 */
int ranking_registrar_victoria(Ranking *r, const char *nombre)
{
    char norm[RANKING_NOMBRE_MAX];
    int idx;
    int i;

    if (r == NULL || nombre == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }
    if (!nombre_normalizado(nombre, norm, (int)sizeof(norm))) {
        return RANKING_ERROR_NOMBRE_INVALIDO;
    }
    idx = ranking_buscar(r, norm);
    if (idx >= 0) {
        if (r->entradas[idx].victorias < RANKING_VICTORIAS_MAX) {
            r->entradas[idx].victorias++;
        }
        ranking_ordenar(r);
        return RANKING_OK;
    }
    if (r->cantidad >= RANKING_MAX_JUGADORES) {
        return RANKING_ERROR_LLENO;
    }
    for (i = 0; norm[i] != '\0'; i++) {
        r->entradas[r->cantidad].nombre[i] = norm[i];
    }
    r->entradas[r->cantidad].nombre[i] = '\0';
    r->entradas[r->cantidad].victorias = 1;
    r->cantidad++;
    ranking_ordenar(r);
    return RANKING_OK;
}

/* ===========================================================================
 * API PUBLICA: PERSISTENCIA BINARIA
 * =========================================================================== */

/*
 * ranking_cargar: lee y valida un archivo BNR1 v1.
 *
 * Estrategia "todo o nada": se lee el archivo completo en un buffer
 * acotado, se validan magic/version/cantidad/tamano/CRC y recien despues
 * se parsean las entradas en un Ranking temporal. Si algo falla, el
 * Ranking del llamador queda vacio (nunca a medio llenar). Al final se
 * ordena y se copia el temporal al llamador con una asignacion de struct.
 *
 * Orden de validacion (importa para el mensaje de error):
 *   1. Tamano del archivo (lectura acotada).
 *   2. Magic "BNR1" y version 1 -> RANKING_ERROR_VERSION.
 *   3. Cantidad <= 100 y tamano exacto 12+36*n -> RANKING_ERROR_FORMATO.
 *   4. CRC32 -> RANKING_ERROR_INTEGRIDAD.
 *   5. Cada entrada: nombre canonico, victorias en rango, sin duplicados
 *      insensibles a mayusculas -> RANKING_ERROR_FORMATO.
 *
 * Un ranking.dat de la version texto anterior ("Ana;1\n") no empieza con
 * "BNR1", por lo que se informa RANKING_ERROR_VERSION. No hay migracion
 * automatica: el archivo viejo debe borrarse (o migrarse aparte).
 */
int ranking_cargar(Ranking *r, const char *ruta)
{
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    Ranking tmp;
    int tam;
    int rc;
    int count;
    int i;
    int off;
    unsigned long crc_leido;
    unsigned long crc_calc;

    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }
    r->cantidad = 0;
    rc = leer_archivo_binario(ruta, buf, RANKING_ARCHIVO_MAX_BYTES, &tam);
    if (rc != RANKING_OK) {
        return rc;
    }
    /* Un archivo de menos de 4 bytes no puede tener ni el magic. */
    if (tam < 4) {
        return RANKING_ERROR_FORMATO;
    }
    if (memcmp(buf, RANKING_ARCHIVO_MAGIC, 4) != 0) {
        return RANKING_ERROR_VERSION;
    }
    /* Con el magic correcto, faltan version/cantidad: archivo truncado. */
    if (tam < RANKING_CABECERA_BYTES) {
        return RANKING_ERROR_FORMATO;
    }
    if (leer_u16(buf + 4) != (unsigned int)RANKING_ARCHIVO_VERSION) {
        return RANKING_ERROR_VERSION;
    }
    count = (int)leer_u16(buf + 6);
    if (count < 0 || count > RANKING_MAX_JUGADORES) {
        return RANKING_ERROR_FORMATO;
    }
    if (tam != RANKING_CABECERA_BYTES + (count * RANKING_ENTRADA_BYTES) +
               RANKING_CHECKSUM_BYTES) {
        return RANKING_ERROR_FORMATO;
    }
    crc_leido = leer_u32(buf + tam - RANKING_CHECKSUM_BYTES);
    crc_calc = crc32_calcular(buf, tam - RANKING_CHECKSUM_BYTES);
    if (crc_leido != crc_calc) {
        return RANKING_ERROR_INTEGRIDAD;
    }

    tmp.cantidad = 0;
    off = RANKING_CABECERA_BYTES;
    for (i = 0; i < count; i++) {
        char nombre[RANKING_NOMBRE_MAX];
        unsigned long victorias;
        int k;

        if (!leer_nombre(buf + off, nombre, (int)sizeof(nombre))) {
            return RANKING_ERROR_FORMATO;
        }
        victorias = leer_u32(buf + off + RANKING_NOMBRE_BYTES);
        if (victorias > (unsigned long)RANKING_VICTORIAS_MAX) {
            return RANKING_ERROR_FORMATO;
        }
        for (k = 0; k < tmp.cantidad; k++) {
            if (comparar_insensible(tmp.entradas[k].nombre, nombre) == 0) {
                return RANKING_ERROR_FORMATO;
            }
        }
        strcpy(tmp.entradas[tmp.cantidad].nombre, nombre);
        tmp.entradas[tmp.cantidad].victorias = (int)victorias;
        tmp.cantidad++;
        off += RANKING_ENTRADA_BYTES;
    }
    ranking_ordenar(&tmp);
    *r = tmp;
    return RANKING_OK;
}

/*
 * ranking_guardar: serializa y escribe el ranking de forma atomica.
 *
 * Pasos:
 *  1. Validar argumentos y el INVARIANTE del struct (cantidad en rango,
 *     cada nombre canonico y cada victoria en rango). Si algo no cuadra,
 *     RANKING_ERROR_ARGUMENTO y no se toca el disco.
 *  2. Armar el buffer completo en memoria:
 *       cabecera (magic, version, cantidad),
 *       cada entrada (nombre 32B + victorias 4B LE),
 *       CRC32 de todo lo anterior.
 *  3. Construir "ruta.tmp" en el mismo directorio.
 *  4. fopen(temp, "wb"), fwrite exacto, fclose comprobado.
 *  5. rename(temp, ruta). En Linux es atomico dentro del mismo FS.
 *  6. Ante cualquier fallo: borrar el temporal y devolver ARCHIVO. El
 *     archivo anterior nunca se trunca porque no se abre el destino.
 *
 * No se modifica el ranking en memoria.
 */
int ranking_guardar(const Ranking *r, const char *ruta)
{
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    char temp[RANKING_RUTA_MAX];
    char norm[RANKING_NOMBRE_MAX];
    FILE *f;
    int count;
    int i;
    int off;
    int total;
    unsigned long crc;

    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }
    count = r->cantidad;
    if (count < 0 || count > RANKING_MAX_JUGADORES) {
        return RANKING_ERROR_ARGUMENTO;
    }
    for (i = 0; i < count; i++) {
        if (!nombre_normalizado(r->entradas[i].nombre, norm, (int)sizeof(norm))) {
            return RANKING_ERROR_ARGUMENTO;
        }
        if (strcmp(norm, r->entradas[i].nombre) != 0) {
            return RANKING_ERROR_ARGUMENTO;
        }
        if (r->entradas[i].victorias < 0 ||
            r->entradas[i].victorias > RANKING_VICTORIAS_MAX) {
            return RANKING_ERROR_ARGUMENTO;
        }
    }

    buf[0] = (unsigned char)'B';
    buf[1] = (unsigned char)'N';
    buf[2] = (unsigned char)'R';
    buf[3] = (unsigned char)'1';
    escribir_u16(buf + 4, (unsigned int)RANKING_ARCHIVO_VERSION);
    escribir_u16(buf + 6, (unsigned int)count);

    off = RANKING_CABECERA_BYTES;
    for (i = 0; i < count; i++) {
        escribir_nombre(buf + off, r->entradas[i].nombre);
        escribir_u32(buf + off + RANKING_NOMBRE_BYTES,
                     (unsigned long)r->entradas[i].victorias);
        off += RANKING_ENTRADA_BYTES;
    }
    crc = crc32_calcular(buf, off);
    escribir_u32(buf + off, crc);
    total = off + RANKING_CHECKSUM_BYTES;

    if (!construir_ruta_temporal(ruta, temp, (int)sizeof(temp))) {
        return RANKING_ERROR_ARCHIVO;
    }
    f = fopen(temp, "wb");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }
    if (fwrite(buf, 1, (size_t)total, f) != (size_t)total) {
        fclose(f);
        remove(temp);
        return RANKING_ERROR_ARCHIVO;
    }
    if (fclose(f) != 0) {
        remove(temp);
        return RANKING_ERROR_ARCHIVO;
    }
    if (rename(temp, ruta) != 0) {
        remove(temp);
        return RANKING_ERROR_ARCHIVO;
    }
    return RANKING_OK;
}

/* ===========================================================================
 * API PUBLICA: PRESENTACION
 * =========================================================================== */

/*
 * ranking_mensaje_error: traduce codigos a texto para la interfaz.
 * La logica devuelve numeros; la UI muestra estos mensajes. Un unico
 * lugar para los textos evita duplicarlos en consola y en una futura GUI.
 */
const char *ranking_mensaje_error(int codigo)
{
    switch (codigo) {
        case RANKING_OK:
            return "Sin error";
        case RANKING_ERROR_ARGUMENTO:
            return "Argumento invalido";
        case RANKING_ERROR_NOMBRE_INVALIDO:
            return "Nombre invalido (1 a 31 caracteres, sin controles)";
        case RANKING_ERROR_LLENO:
            return "Ranking lleno (maximo 100 jugadores)";
        case RANKING_ERROR_ARCHIVO:
            return "No se pudo leer o guardar el archivo";
        case RANKING_ERROR_FORMATO:
            return "Archivo con formato invalido";
        case RANKING_ERROR_INTEGRIDAD:
            return "Archivo corrupto: el checksum no coincide";
        case RANKING_ERROR_VERSION:
            return "Archivo de ranking de otra version o formato";
        default:
            return "Error desconocido";
    }
}

/*
 * ranking_mostrar_en: imprime la tabla ordenada en `salida`.
 * No ordena: confia en el INVARIANTE 2. El formato %-31s alinea nombres
 * de hasta 31 caracteres. Si el ranking esta vacio, lo informa.
 * Devuelve RANKING_ERROR_ARGUMENTO si salida o r son NULL.
 */
int ranking_mostrar_en(FILE *salida, const Ranking *r)
{
    int i;

    if (salida == NULL || r == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }
    if (r->cantidad <= 0) {
        fprintf(salida, "Ranking vacio. Todavia no hay victorias registradas.\n");
        return RANKING_OK;
    }
    fprintf(salida, "=== RANKING (%d jugador%s) ===\n", r->cantidad,
            r->cantidad == 1 ? "" : "es");
    for (i = 0; i < r->cantidad; i++) {
        fprintf(salida, "%2d. %-31s %d\n",
                i + 1, r->entradas[i].nombre, r->entradas[i].victorias);
    }
    return RANKING_OK;
}

/* ranking_mostrar: atajo sobre stdout (pantalla final de consola). */
void ranking_mostrar(const Ranking *r)
{
    if (r == NULL) {
        return;
    }
    ranking_mostrar_en(stdout, r);
}
