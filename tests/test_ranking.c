/*
 * test_ranking.c - Pruebas automaticas del ranking binario BNR1 v1.
 *
 * ===========================================================================
 * 1. ESTRATEGIA
 * ===========================================================================
 * Sin frameworks externos: `pruebas` cuenta aserciones, `fallos` cuenta
 * las que fallaron y chequear() imprime "ok"/"FALLO". main() devuelve 0
 * si todo paso o 1 si hubo fallos, para que `make test` lo detecte.
 *
 * Las pruebas verifican el CONTRATO PUBLICO de ranking.h, no detalles
 * internos. Para probar el formato binario se construyen archivos a mano
 * con un CRC32 INDEPENDIENTE (crc32_test) y se comprueba que el modulo
 * los acepte/rechace. Asi un error en la implementacion del CRC no queda
 * oculto por un roundtrip que use la misma funcion para escribir y leer.
 *
 * Cada bateria prueba_* cubre un aspecto:
 *   - CRC de control y formato basico;
 *   - registro, mayusculas/espacios y validacion de nombres;
 *   - ';' permitido (en binario no es separador);
 *   - orden y desempate;
 *   - roundtrip con verificacion byte a byte del archivo;
 *   - archivo vacio, inexistente, texto viejo, version/magic;
 *   - CRC corrupto, truncado, tamano extra, cantidad invalida;
 *   - nombres no canonicos, duplicados, victorias fuera de rango;
 *   - tope de victorias y capacidad de 100 jugadores;
 *   - errores de API y presentacion.
 *
 * Los archivos de prueba usan data/ranking_test_*.dat (ignorados por
 * .gitignore) y se borran con remove() al terminar cada caso.
 */

#include <stdio.h>
#include <string.h>

#include "ranking.h"

#define TEST_BIN "data/ranking_test_bin.dat"
#define TEST_TMP "data/ranking_test_bin.dat.tmp"

/* Contadores del mini-framework. */
static int fallos = 0;
static int pruebas = 0;

/* Registra una asercion. `nombre` describe la invariante verificada. */
static void chequear(int condicion, const char *nombre)
{
    pruebas++;
    if (condicion) {
        printf("ok   - %s\n", nombre);
    } else {
        printf("FALLO- %s\n", nombre);
        fallos++;
    }
}

/* ===========================================================================
 * HELPERS DE ARCHIVO
 * =========================================================================== */

/* Crea/sobrescribe `ruta` con `n` bytes de `buf` (n=0 crea vacio). */
static int escribir_bytes(const char *ruta, const unsigned char *buf, int n)
{
    FILE *f;
    size_t escritos;

    f = fopen(ruta, "wb");
    if (f == NULL) {
        return 0;
    }
    if (n > 0) {
        escritos = fwrite(buf, 1, (size_t)n, f);
        if (escritos != (size_t)n) {
            fclose(f);
            return 0;
        }
    }
    if (fclose(f) != 0) {
        return 0;
    }
    return 1;
}

/* Lee hasta `max` bytes; devuelve la cantidad o -1 si falla. */
static int leer_bytes(const char *ruta, unsigned char *buf, int max)
{
    FILE *f;
    long tam;
    size_t leidos;

    f = fopen(ruta, "rb");
    if (f == NULL) {
        return -1;
    }
    if (fseek(f, 0L, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    tam = ftell(f);
    if (tam < 0L || tam > (long)max) {
        fclose(f);
        return -1;
    }
    if (fseek(f, 0L, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }
    leidos = fread(buf, 1, (size_t)tam, f);
    if (fclose(f) != 0) {
        return -1;
    }
    if (leidos != (size_t)tam) {
        return -1;
    }
    return (int)tam;
}

/* Devuelve 1 si el archivo existe. */
static int existe_archivo(const char *ruta)
{
    FILE *f;
    f = fopen(ruta, "rb");
    if (f == NULL) {
        return 0;
    }
    fclose(f);
    return 1;
}

/* Escribe texto plano (para simular el ranking.dat de la version vieja). */
static int escribir_texto(const char *ruta, const char *texto)
{
    return escribir_bytes(ruta, (const unsigned char *)texto,
                          (int)strlen(texto));
}

/* ===========================================================================
 * HELPERS DE FORMATO BINARIO (implementacion INDEPENDIENTE del modulo)
 * =========================================================================== */

static void poner_u16(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v & 0xFFU);
    p[1] = (unsigned char)((v >> 8) & 0xFFU);
}

static void poner_u32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)(v & 0xFFUL);
    p[1] = (unsigned char)((v >> 8) & 0xFFUL);
    p[2] = (unsigned char)((v >> 16) & 0xFFUL);
    p[3] = (unsigned char)((v >> 24) & 0xFFUL);
}

static unsigned int tomar_u16(const unsigned char *p)
{
    return ((unsigned int)p[0]) | (((unsigned int)p[1]) << 8);
}

static unsigned long tomar_u32(const unsigned char *p)
{
    return ((unsigned long)p[0]) |
           (((unsigned long)p[1]) << 8) |
           (((unsigned long)p[2]) << 16) |
           (((unsigned long)p[3]) << 24);
}

/*
 * crc32_test: misma especificacion que el formato (IEEE 802.3), pero
 * escrita aparte para que las pruebas no dependan de ranking.c.
 * Valor de control conocido: "123456789" -> 0xCBF43926.
 */
static unsigned long crc32_test(const unsigned char *datos, int n)
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
 * construir_archivo: arma un BNR1 a mano con los nombres/victorias dados.
 * - `version` permite probar versiones desconocidas.
 * - `alterar_crc` permite simular un archivo corrupto.
 * - Los nombres se truncan a 31 bytes (el llamador decide si eso es un
 *   caso valido o invalido).
 * Devuelve 1 si pudo escribir.
 */
static int construir_archivo(const char *ruta, int version, int count,
                             const char *nombres[], const unsigned long *wins,
                             int alterar_crc)
{
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int i;
    int j;
    int off;
    int total;
    unsigned long crc;

    if (count < 0 || count > RANKING_MAX_JUGADORES) {
        return 0;
    }
    if (count > 0 && (nombres == NULL || wins == NULL)) {
        return 0;
    }
    buf[0] = (unsigned char)'B';
    buf[1] = (unsigned char)'N';
    buf[2] = (unsigned char)'R';
    buf[3] = (unsigned char)'1';
    poner_u16(buf + 4, (unsigned int)version);
    poner_u16(buf + 6, (unsigned int)count);

    off = RANKING_CABECERA_BYTES;
    for (i = 0; i < count; i++) {
        for (j = 0; j < RANKING_NOMBRE_BYTES; j++) {
            buf[off + j] = 0;
        }
        for (j = 0; nombres[i][j] != '\0' && j < RANKING_NOMBRE_BYTES - 1; j++) {
            buf[off + j] = (unsigned char)nombres[i][j];
        }
        poner_u32(buf + off + RANKING_NOMBRE_BYTES, wins[i]);
        off += RANKING_ENTRADA_BYTES;
    }
    crc = crc32_test(buf, off);
    if (alterar_crc) {
        crc ^= 1UL;
    }
    poner_u32(buf + off, crc);
    total = off + RANKING_CHECKSUM_BYTES;
    return escribir_bytes(ruta, buf, total);
}

/* Cambia un byte del archivo (para corromper magic, count, padding, etc.). */
static int mutar_byte(const char *ruta, int indice, unsigned char valor)
{
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int n;

    n = leer_bytes(ruta, buf, (int)sizeof(buf));
    if (n < 0 || indice < 0 || indice >= n) {
        return 0;
    }
    buf[indice] = valor;
    return escribir_bytes(ruta, buf, n);
}

/* Recalcula y reescribe el CRC (mantiene el archivo formalmente valido). */
static int recalcular_crc(const char *ruta)
{
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int n;
    unsigned long crc;

    n = leer_bytes(ruta, buf, (int)sizeof(buf));
    if (n < RANKING_ARCHIVO_MIN_BYTES) {
        return 0;
    }
    crc = crc32_test(buf, n - RANKING_CHECKSUM_BYTES);
    poner_u32(buf + n - RANKING_CHECKSUM_BYTES, crc);
    return escribir_bytes(ruta, buf, n);
}

/* Agrega un byte al final (simula un archivo con basura extra). */
static int agregar_byte(const char *ruta, unsigned char valor)
{
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES + 1];
    int n;

    n = leer_bytes(ruta, buf, (int)sizeof(buf) - 1);
    if (n < 0) {
        return 0;
    }
    buf[n] = valor;
    return escribir_bytes(ruta, buf, n + 1);
}

/* Quita el ultimo byte (simula un archivo truncado). */
static int quitar_byte_final(const char *ruta)
{
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int n;

    n = leer_bytes(ruta, buf, (int)sizeof(buf));
    if (n <= 0) {
        return 0;
    }
    return escribir_bytes(ruta, buf, n - 1);
}

/* ===========================================================================
 * BATERIAS DE PRUEBA
 * =========================================================================== */

/* CRC de control: valida la implementacion independiente usada por los
   constructores de archivos. Si esto falla, todas las pruebas de formato
   dejan de ser confiables. */
static void prueba_crc_conocido(void)
{
    chequear(crc32_test((const unsigned char *)"123456789", 9) == 0xCBF43926UL,
             "CRC32 de control 123456789 = CBF43926");
}

/* Circuito minimo: inicializar, registrar, acumular y no duplicar. */
static void prueba_basica(void)
{
    Ranking r;
    int rc;

    ranking_inicializar(&r);
    chequear(ranking_cantidad(&r) == 0, "inicializar deja cantidad 0");

    rc = ranking_registrar_victoria(&r, "Ana");
    chequear(rc == RANKING_OK, "registrar Ana ok");
    chequear(ranking_cantidad(&r) == 1, "cantidad 1 tras Ana");
    chequear(ranking_obtener_victorias(&r, "Ana") == 1, "Ana tiene 1");

    rc = ranking_registrar_victoria(&r, "Ana");
    chequear(rc == RANKING_OK, "segunda victoria Ana ok");
    chequear(ranking_obtener_victorias(&r, "Ana") == 2, "Ana tiene 2");
}

/* Identidad: mayusculas y espacios de los extremos no crean jugadores
   distintos; se conserva la primera grafia. */
static void prueba_mayusculas_espacios(void)
{
    Ranking r;

    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Luis");
    ranking_registrar_victoria(&r, "luis");
    ranking_registrar_victoria(&r, "  LUIS  ");
    chequear(ranking_cantidad(&r) == 1, "mayusculas/espacios son mismo jugador");
    chequear(ranking_obtener_victorias(&r, "lUiS") == 3, "3 victorias acumuladas");
    chequear(ranking_buscar(&r, "LUIS") == 0, "buscar insensible encuentra");
    chequear(ranking_obtener(&r, 0) != NULL &&
             strcmp(ranking_obtener(&r, 0)->nombre, "Luis") == 0,
             "se conserva la primera grafia");
}

/* Nombres invalidos: vacio, solo espacios, controles, demasiado largo,
   NULL. Ninguno debe dejar basura en el ranking. */
static void prueba_nombres_invalidos(void)
{
    Ranking r;
    int rc;
    char largo[80];
    int i;

    ranking_inicializar(&r);
    rc = ranking_registrar_victoria(&r, "");
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "nombre vacio invalido");
    rc = ranking_registrar_victoria(&r, "   ");
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "solo espacios invalido");
    rc = ranking_registrar_victoria(&r, "A\tB");
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "tab interno invalido");
    rc = ranking_registrar_victoria(&r, "A\nB");
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "salto de linea invalido");
    rc = ranking_registrar_victoria(&r, NULL);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "nombre NULL argumento");
    rc = ranking_registrar_victoria(NULL, "Ana");
    chequear(rc == RANKING_ERROR_ARGUMENTO, "ranking NULL argumento");

    for (i = 0; i < 79; i++) {
        largo[i] = 'x';
    }
    largo[79] = '\0';
    rc = ranking_registrar_victoria(&r, largo);
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "nombre de 79 invalido");
    chequear(ranking_cantidad(&r) == 0, "nada se agrego con invalidos");
}

/* Limites exactos de nombre: 31 valido, 32 invalido. */
static void prueba_limites_nombre(void)
{
    Ranking r;
    char n31[32];
    char n32[33];
    int i;
    int rc;

    for (i = 0; i < 31; i++) {
        n31[i] = 'a';
    }
    n31[31] = '\0';
    for (i = 0; i < 32; i++) {
        n32[i] = 'b';
    }
    n32[32] = '\0';

    ranking_inicializar(&r);
    rc = ranking_registrar_victoria(&r, n31);
    chequear(rc == RANKING_OK, "nombre de 31 aceptado");
    rc = ranking_registrar_victoria(&r, n32);
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "nombre de 32 rechazado");
    chequear(ranking_cantidad(&r) == 1, "solo quedo el de 31");
}

/* En binario el ';' es un caracter comun: no hay separadores. */
static void prueba_punto_y_coma(void)
{
    Ranking r;
    Ranking r2;
    int rc;

    ranking_inicializar(&r);
    rc = ranking_registrar_victoria(&r, "A;B");
    chequear(rc == RANKING_OK, "nombre con ';' aceptado en binario");
    chequear(ranking_buscar(&r, "a;b") == 0, "busqueda con ';' insensible");
    rc = ranking_guardar(&r, TEST_BIN);
    chequear(rc == RANKING_OK, "guardar nombre con ';' ok");
    rc = ranking_cargar(&r2, TEST_BIN);
    chequear(rc == RANKING_OK, "cargar nombre con ';' ok");
    chequear(ranking_obtener_victorias(&r2, "A;B") == 1, "';' sobrevive roundtrip");
    remove(TEST_BIN);
}

/* Orden: victorias desc; empate alfabetico insensible. */
static void prueba_orden(void)
{
    Ranking r;
    const RankingEntrada *e;

    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Zoe");
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Luis");
    ranking_registrar_victoria(&r, "Luis");
    ranking_registrar_victoria(&r, "Luis");

    chequear(ranking_cantidad(&r) == 3, "orden: 3 jugadores");
    e = ranking_obtener(&r, 0);
    chequear(e != NULL && strcmp(e->nombre, "Luis") == 0 && e->victorias == 3,
             "orden: primero Luis 3");
    e = ranking_obtener(&r, 1);
    chequear(e != NULL && strcmp(e->nombre, "Ana") == 0 && e->victorias == 2,
             "orden: segundo Ana 2");
    e = ranking_obtener(&r, 2);
    chequear(e != NULL && strcmp(e->nombre, "Zoe") == 0 && e->victorias == 1,
             "orden: tercero Zoe 1");

    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "beto");
    ranking_registrar_victoria(&r, "Ana");
    e = ranking_obtener(&r, 0);
    chequear(e != NULL && strcmp(e->nombre, "Ana") == 0,
             "empate: Ana antes que beto");
}

/*
 * Roundtrip principal con inspeccion byte a byte:
 *   Ana 2, Luis 1 -> archivo de 84 bytes:
 *   "BNR1" | v1 | count=2 | Ana(32)+2 | Luis(32)+1 | CRC.
 * Verifica magic, version, cantidad, cada entrada y CRC; despues carga
 * en OTRO Ranking y compara valores. Tambien exige que no quede .tmp.
 */
static void prueba_guardar_cargar(void)
{
    Ranking r;
    Ranking r2;
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int n;
    int rc;

    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Luis");

    rc = ranking_guardar(&r, TEST_BIN);
    chequear(rc == RANKING_OK, "guardar roundtrip ok");
    chequear(!existe_archivo(TEST_TMP), "no queda archivo .tmp");

    n = leer_bytes(TEST_BIN, buf, (int)sizeof(buf));
    chequear(n == RANKING_ARCHIVO_MIN_BYTES + 2 * RANKING_ENTRADA_BYTES,
             "tamano exacto 12 + 2*36 = 84");
    chequear(memcmp(buf, "BNR1", 4) == 0, "magic BNR1");
    chequear(tomar_u16(buf + 4) == RANKING_ARCHIVO_VERSION, "version 1");
    chequear(tomar_u16(buf + 6) == 2, "cantidad 2 en cabecera");
    chequear(memcmp(buf + 8, "Ana", 3) == 0 && buf[11] == 0,
             "entrada 0: nombre Ana con NUL");
    chequear(tomar_u32(buf + 8 + RANKING_NOMBRE_BYTES) == 2,
             "entrada 0: victorias 2 LE");
    chequear(memcmp(buf + 8 + RANKING_ENTRADA_BYTES, "Luis", 4) == 0,
             "entrada 1: nombre Luis");
    chequear(tomar_u32(buf + 8 + RANKING_ENTRADA_BYTES + RANKING_NOMBRE_BYTES) == 1,
             "entrada 1: victorias 1 LE");
    chequear(tomar_u32(buf + n - RANKING_CHECKSUM_BYTES) ==
             crc32_test(buf, n - RANKING_CHECKSUM_BYTES),
             "CRC32 final coincide con el contenido");

    ranking_inicializar(&r2);
    rc = ranking_cargar(&r2, TEST_BIN);
    chequear(rc == RANKING_OK, "cargar roundtrip ok");
    chequear(ranking_cantidad(&r2) == 2, "roundtrip cantidad 2");
    chequear(ranking_obtener_victorias(&r2, "ana") == 2, "roundtrip Ana 2");
    chequear(ranking_obtener_victorias(&r2, "LUIS") == 1, "roundtrip Luis 1");
    remove(TEST_BIN);
}

/* Ranking vacio: archivo valido de 12 bytes, con count=0 y CRC. */
static void prueba_guardar_vacio(void)
{
    Ranking r;
    Ranking r2;
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int n;
    int rc;

    ranking_inicializar(&r);
    rc = ranking_guardar(&r, TEST_BIN);
    chequear(rc == RANKING_OK, "guardar vacio ok");
    n = leer_bytes(TEST_BIN, buf, (int)sizeof(buf));
    chequear(n == RANKING_ARCHIVO_MIN_BYTES, "vacio mide 12 bytes");
    chequear(memcmp(buf, "BNR1", 4) == 0, "vacio tiene magic");
    chequear(tomar_u16(buf + 6) == 0, "vacio tiene count 0");
    chequear(tomar_u32(buf + 8) == crc32_test(buf, 8), "vacio tiene CRC");
    rc = ranking_cargar(&r2, TEST_BIN);
    chequear(rc == RANKING_OK, "cargar vacio ok");
    chequear(ranking_cantidad(&r2) == 0, "cargar vacio deja 0");
    remove(TEST_BIN);
}

/* Primera ejecucion: el archivo no existe. */
static void prueba_archivo_inexistente(void)
{
    Ranking r;
    int rc;

    ranking_inicializar(&r);
    rc = ranking_cargar(&r, "data/no_existe_12345.dat");
    chequear(rc == RANKING_ERROR_ARCHIVO, "inexistente -> ERROR_ARCHIVO");
    chequear(ranking_cantidad(&r) == 0, "inexistente deja vacio");
}

/* Archivo de 0 bytes: no es un BNR1 valido. */
static void prueba_archivo_vacio(void)
{
    Ranking r;
    unsigned char dummy;
    int rc;

    dummy = 0;
    chequear(escribir_bytes(TEST_BIN, &dummy, 0) == 1,
             "crear archivo de 0 bytes (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "archivo de 0 bytes -> FORMATO");
    chequear(ranking_cantidad(&r) == 0, "0 bytes deja vacio");
    remove(TEST_BIN);
}

/*
 * Version/magic: version 2, magic alterado y el ranking.dat de texto
 * viejo. Los tres deben distinguirse de un archivo truncado.
 */
static void prueba_version_y_texto_viejo(void)
{
    Ranking r;
    unsigned char corto[4];
    int rc;

    chequear(construir_archivo(TEST_BIN, 2, 0, NULL, NULL, 0) == 1,
             "crear version 2 (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_VERSION, "version 2 -> ERROR_VERSION");
    remove(TEST_BIN);

    chequear(construir_archivo(TEST_BIN, 1, 0, NULL, NULL, 0) == 1,
             "crear archivo valido (setup)");
    chequear(mutar_byte(TEST_BIN, 0, (unsigned char)'X') == 1,
             "alterar magic (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_VERSION, "magic XNR1 -> ERROR_VERSION");
    remove(TEST_BIN);

    chequear(escribir_texto(TEST_BIN, "Ana;1\n") == 1,
             "crear ranking de texto viejo (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_VERSION, "texto viejo -> ERROR_VERSION");
    remove(TEST_BIN);

    corto[0] = (unsigned char)'B';
    corto[1] = (unsigned char)'N';
    corto[2] = (unsigned char)'R';
    corto[3] = (unsigned char)'1';
    chequear(escribir_bytes(TEST_BIN, corto, 4) == 1,
             "crear magic solo (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "solo magic -> FORMATO");
    remove(TEST_BIN);
}

/* CRC alterado: el archivo tiene el tamano correcto pero el checksum no. */
static void prueba_crc_malo(void)
{
    Ranking r;
    const char *nombres[1];
    unsigned long wins[1];
    int rc;

    nombres[0] = "Ana";
    wins[0] = 1;
    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 1) == 1,
             "crear archivo con CRC malo (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_INTEGRIDAD, "CRC malo -> ERROR_INTEGRIDAD");
    chequear(ranking_cantidad(&r) == 0, "CRC malo deja vacio");
    remove(TEST_BIN);
}

/* Tamanos: byte extra, truncado, cantidad imposible y archivo gigante. */
static void prueba_tamanos_y_cantidad(void)
{
    Ranking r;
    const char *nombres[1];
    unsigned long wins[1];
    unsigned char grande[4000];
    int i;
    int rc;

    nombres[0] = "Ana";
    wins[0] = 1;

    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear 1 entrada (setup)");
    chequear(agregar_byte(TEST_BIN, 0) == 1, "agregar byte extra (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "byte extra -> FORMATO");
    remove(TEST_BIN);

    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear 1 entrada de nuevo (setup)");
    chequear(quitar_byte_final(TEST_BIN) == 1, "truncar 1 byte (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "truncado -> FORMATO");
    remove(TEST_BIN);

    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear 1 entrada para count (setup)");
    chequear(mutar_byte(TEST_BIN, 6, 101) == 1, "count=101 (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "count 101 -> FORMATO");
    remove(TEST_BIN);

    for (i = 0; i < 4000; i++) {
        grande[i] = 0;
    }
    chequear(escribir_bytes(TEST_BIN, grande, 4000) == 1,
             "crear archivo de 4000 bytes (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "archivo > maximo -> FORMATO");
    remove(TEST_BIN);
}

/*
 * Entradas semanticamente invalidas con CRC correcto: nombre vacio,
 * control, espacios extremos, duplicado, victorias fuera de rango y
 * relleno distinto de cero. El CRC valido prueba que el rechazo es por
 * validacion de contenido, no por checksum.
 */
static void prueba_entradas_invalidas(void)
{
    Ranking r;
    const char *nombres[2];
    unsigned long wins[2];
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int n;
    int rc;

    nombres[0] = "";
    wins[0] = 1;
    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear nombre vacio (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "nombre vacio -> FORMATO");
    remove(TEST_BIN);

    nombres[0] = "A\tB";
    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear nombre con tab (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "nombre con control -> FORMATO");
    remove(TEST_BIN);

    nombres[0] = "Ana ";
    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear nombre con espacio final (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "nombre no canonico -> FORMATO");
    remove(TEST_BIN);

    nombres[0] = "Ana";
    nombres[1] = "ana";
    wins[0] = 1;
    wins[1] = 2;
    chequear(construir_archivo(TEST_BIN, 1, 2, nombres, wins, 0) == 1,
             "crear duplicados (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "duplicados -> FORMATO");
    remove(TEST_BIN);

    nombres[0] = "Ana";
    wins[0] = (unsigned long)RANKING_VICTORIAS_MAX + 1UL;
    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear victorias fuera de rango (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "victorias > maximo -> FORMATO");
    remove(TEST_BIN);

    /* Relleno no canonico: "Ana" + NUL, pero un byte posterior no es 0.
       Se recalcula el CRC para aislar la validacion de contenido. */
    nombres[0] = "Ana";
    wins[0] = 1;
    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear Ana para padding (setup)");
    n = leer_bytes(TEST_BIN, buf, (int)sizeof(buf));
    chequear(n > 0, "leer Ana para padding (setup)");
    buf[RANKING_CABECERA_BYTES + 4] = (unsigned char)'X';
    chequear(escribir_bytes(TEST_BIN, buf, n) == 1, "meter basura en padding (setup)");
    chequear(recalcular_crc(TEST_BIN) == 1, "recalcular CRC (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_FORMATO, "padding no cero -> FORMATO");
    remove(TEST_BIN);
}

/* Tope de victorias: un jugador en el maximo no desborda. */
static void prueba_tope_victorias(void)
{
    Ranking r;
    const char *nombres[1];
    unsigned long wins[1];
    int rc;

    nombres[0] = "Ana";
    wins[0] = (unsigned long)RANKING_VICTORIAS_MAX;
    chequear(construir_archivo(TEST_BIN, 1, 1, nombres, wins, 0) == 1,
             "crear Ana en el tope (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, TEST_BIN);
    chequear(rc == RANKING_OK, "cargar Ana en el tope ok");
    chequear(ranking_obtener_victorias(&r, "Ana") == RANKING_VICTORIAS_MAX,
             "Ana tiene el maximo");
    rc = ranking_registrar_victoria(&r, "Ana");
    chequear(rc == RANKING_OK, "registrar sobre el tope no falla");
    chequear(ranking_obtener_victorias(&r, "Ana") == RANKING_VICTORIAS_MAX,
             "el tope no desborda");
    remove(TEST_BIN);
}

/* Capacidad: 100 jugadores, archivo maximo de 3612 bytes, ida y vuelta. */
static void prueba_capacidad(void)
{
    Ranking r;
    Ranking r2;
    char nombre[16];
    unsigned char buf[RANKING_ARCHIVO_MAX_BYTES];
    int i;
    int n;
    int rc;

    ranking_inicializar(&r);
    rc = RANKING_OK;
    for (i = 0; i < RANKING_MAX_JUGADORES; i++) {
        sprintf(nombre, "J%03d", i);
        rc = ranking_registrar_victoria(&r, nombre);
        if (rc != RANKING_OK) {
            break;
        }
    }
    chequear(rc == RANKING_OK, "llenar hasta MAX ok");
    chequear(ranking_cantidad(&r) == RANKING_MAX_JUGADORES, "cantidad == MAX");
    rc = ranking_registrar_victoria(&r, "UnoMas");
    chequear(rc == RANKING_ERROR_LLENO, "uno mas alla de MAX -> LLENO");

    rc = ranking_guardar(&r, TEST_BIN);
    chequear(rc == RANKING_OK, "guardar 100 jugadores ok");
    n = leer_bytes(TEST_BIN, buf, (int)sizeof(buf));
    chequear(n == RANKING_ARCHIVO_MAX_BYTES, "archivo de 100 mide 3612");
    chequear(tomar_u16(buf + 6) == RANKING_MAX_JUGADORES, "cabecera count 100");
    chequear(tomar_u32(buf + n - RANKING_CHECKSUM_BYTES) ==
             crc32_test(buf, n - RANKING_CHECKSUM_BYTES),
             "CRC del archivo maximo ok");

    ranking_inicializar(&r2);
    rc = ranking_cargar(&r2, TEST_BIN);
    chequear(rc == RANKING_OK, "cargar 100 jugadores ok");
    chequear(ranking_cantidad(&r2) == RANKING_MAX_JUGADORES, "roundtrip 100");
    chequear(ranking_obtener_victorias(&r2, "J000") == 1, "J000 presente");
    chequear(ranking_obtener_victorias(&r2, "J099") == 1, "J099 presente");
    remove(TEST_BIN);
}

/* Errores de API: NULL, struct inconsistente, ruta sin directorio. */
static void prueba_errores_api(void)
{
    Ranking r;
    int rc;

    ranking_inicializar(&r);
    rc = ranking_cargar(NULL, TEST_BIN);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "cargar NULL ranking -> ARGUMENTO");
    rc = ranking_cargar(&r, NULL);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "cargar NULL ruta -> ARGUMENTO");
    rc = ranking_guardar(NULL, TEST_BIN);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "guardar NULL ranking -> ARGUMENTO");
    rc = ranking_guardar(&r, NULL);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "guardar NULL ruta -> ARGUMENTO");

    r.cantidad = 1;
    strcpy(r.entradas[0].nombre, "");
    r.entradas[0].victorias = 1;
    rc = ranking_guardar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "guardar nombre vacio -> ARGUMENTO");
    strcpy(r.entradas[0].nombre, "Ana");
    r.entradas[0].victorias = -1;
    rc = ranking_guardar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "guardar victorias negativas -> ARGUMENTO");
    r.entradas[0].victorias = RANKING_VICTORIAS_MAX + 1;
    rc = ranking_guardar(&r, TEST_BIN);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "guardar victorias > max -> ARGUMENTO");

    ranking_inicializar(&r);
    rc = ranking_guardar(&r, "data/no_existe_dir/ranking.dat");
    chequear(rc == RANKING_ERROR_ARCHIVO, "guardar en directorio inexistente -> ARCHIVO");

    chequear(ranking_cantidad(NULL) == 0, "cantidad NULL -> 0");
    chequear(ranking_buscar(NULL, "Ana") == -1, "buscar NULL -> -1");
    chequear(ranking_obtener_victorias(NULL, "Ana") == 0, "victorias NULL -> 0");
    chequear(ranking_obtener(&r, 0) == NULL, "obtener fuera de rango -> NULL");
    chequear(ranking_obtener(&r, -1) == NULL, "obtener negativo -> NULL");
    ranking_inicializar(NULL);
    ranking_ordenar(NULL);
    chequear(1, "inicializar/ordenar NULL no rompen");
}

/* Presentacion: salida valida, NULL y traductor de errores. */
static void prueba_mostrar(void)
{
    Ranking r;
    int rc;
    int i;

    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    rc = ranking_mostrar_en(stdout, &r);
    chequear(rc == RANKING_OK, "mostrar_en ok");
    rc = ranking_mostrar_en(NULL, &r);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "mostrar_en NULL salida -> ARGUMENTO");
    rc = ranking_mostrar_en(stdout, NULL);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "mostrar_en NULL ranking -> ARGUMENTO");

    for (i = 0; i <= 7; i++) {
        if (ranking_mensaje_error(i) == NULL) {
            chequear(0, "mensaje_error devolvio NULL");
            return;
        }
    }
    chequear(1, "mensaje_error no NULL para todos los codigos");
    chequear(ranking_mensaje_error(99) != NULL, "mensaje_error desconocido no NULL");
}

int main(void)
{
    printf("== test_ranking (BNR1 v1) ==\n");
    prueba_crc_conocido();
    prueba_basica();
    prueba_mayusculas_espacios();
    prueba_nombres_invalidos();
    prueba_limites_nombre();
    prueba_punto_y_coma();
    prueba_orden();
    prueba_guardar_cargar();
    prueba_guardar_vacio();
    prueba_archivo_inexistente();
    prueba_archivo_vacio();
    prueba_version_y_texto_viejo();
    prueba_crc_malo();
    prueba_tamanos_y_cantidad();
    prueba_entradas_invalidas();
    prueba_tope_victorias();
    prueba_capacidad();
    prueba_errores_api();
    prueba_mostrar();
    printf("------------------\n");
    printf("Pruebas: %d, Fallos: %d\n", pruebas, fallos);
    if (fallos == 0) {
        printf("TODO OK\n");
        return 0;
    }
    printf("HAY FALLOS\n");
    return 1;
}
