/*
 * ranking.c - Implementacion del ranking de victorias.
 *
 * ANSI C (C89): solo <stdio.h>, <string.h>, <ctype.h>.
 * Sin fwrite de structs: el archivo es texto "nombre;victorias".
 * Sin dependencias de sistema operativo.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ranking.h"

#define RANKING_LINEA_MAX 256

/* Recorta espacios en extremos. Siempre termina en '\0'. */
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

/* Comparacion insensible a mayusculas. Estilo strcasecmp portable. */
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
 * Normaliza y valida un nombre.
 * Salida: nombre recortado (conserva mayusculas originales).
 * Devuelve 1 si valido, 0 si no.
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

    /* Si el recorte trunco por longitud, el original era demasiado largo. */
    /* Verificamos con el largo del origen recortado sin truncar. */
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
        if (tmp[i] == ';' || tmp[i] == '\n' || tmp[i] == '\r') {
            return 0;
        }
        if ((unsigned char)tmp[i] < 32) {
            return 0;
        }
    }
    /* Copia segura a la salida. */
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

/* Convierte texto a victorias 0..RANKING_VICTORIAS_MAX. 1 si ok. */
static int parsear_victorias(const char *texto, int *valor)
{
    const char *p;
    long acc;
    int digitos;

    if (texto == NULL || valor == NULL) {
        return 0;
    }
    p = texto;
    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '+') {
        p++;
    }
    /* No se admite signo negativo. */
    if (*p == '-') {
        return 0;
    }
    acc = 0;
    digitos = 0;
    while (*p >= '0' && *p <= '9') {
        acc = acc * 10 + (*p - '0');
        if (acc > RANKING_VICTORIAS_MAX) {
            return 0;
        }
        p++;
        digitos++;
    }
    if (digitos == 0) {
        return 0;
    }
    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p != '\0') {
        return 0;
    }
    *valor = (int)acc;
    return 1;
}

/* Quita '\n' y '\r' finales de una linea leida con fgets. */
static void quitar_salto(char *linea)
{
    int largo;
    largo = (int)strlen(linea);
    while (largo > 0 && (linea[largo - 1] == '\n' || linea[largo - 1] == '\r')) {
        linea[largo - 1] = '\0';
        largo--;
    }
}

void ranking_inicializar(Ranking *r)
{
    if (r == NULL) {
        return;
    }
    r->cantidad = 0;
}

int ranking_cantidad(const Ranking *r)
{
    if (r == NULL) {
        return 0;
    }
    return r->cantidad;
}

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

int ranking_obtener_victorias(const Ranking *r, const char *nombre)
{
    int idx;
    idx = ranking_buscar(r, nombre);
    if (idx < 0) {
        return 0;
    }
    return r->entradas[idx].victorias;
}

void ranking_ordenar(Ranking *r)
{
    int i;
    int j;
    RankingEntrada tmp;

    if (r == NULL) {
        return;
    }
    /* Insercion: victorias desc, nombre asc insensible, desempate sensible. */
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

int ranking_cargar(Ranking *r, const char *ruta)
{
    FILE *f;
    char linea[RANKING_LINEA_MAX];
    int hubo_formato;
    int linea_larga;

    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }
    r->cantidad = 0;
    f = fopen(ruta, "r");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }
    hubo_formato = 0;
    while (fgets(linea, (int)sizeof(linea), f) != NULL) {
        char *puntoycoma;
        char nombre[RANKING_NOMBRE_MAX];
        char *texto_vic;
        int vic;
        int idx;
        int k;

        /* Linea truncada: no hay '\n' y no es fin de archivo. */
        linea_larga = 0;
        if (strchr(linea, '\n') == NULL && !feof(f)) {
            /* Verificar si realmente falta el salto (linea > buffer). */
            /* Consumir el resto de la linea. */
            int c;
            /* Solo es larga si el buffer se lleno. */
            if ((int)strlen(linea) == (int)sizeof(linea) - 1) {
                linea_larga = 1;
                c = fgetc(f);
                while (c != '\n' && c != EOF) {
                    c = fgetc(f);
                }
            }
        }
        if (linea_larga) {
            hubo_formato = 1;
            continue;
        }
        quitar_salto(linea);
        /* Ignorar lineas vacias o solo espacios. */
        {
            char rec[RANKING_LINEA_MAX];
            recortar_extremos(linea, rec, (int)sizeof(rec));
            if (rec[0] == '\0') {
                continue;
            }
        }
        puntoycoma = strchr(linea, ';');
        if (puntoycoma == NULL) {
            hubo_formato = 1;
            continue;
        }
        *puntoycoma = '\0';
        texto_vic = puntoycoma + 1;
        if (!nombre_normalizado(linea, nombre, (int)sizeof(nombre))) {
            hubo_formato = 1;
            continue;
        }
        if (!parsear_victorias(texto_vic, &vic)) {
            hubo_formato = 1;
            continue;
        }
        idx = -1;
        for (k = 0; k < r->cantidad; k++) {
            if (comparar_insensible(r->entradas[k].nombre, nombre) == 0) {
                idx = k;
                break;
            }
        }
        if (idx >= 0) {
            /* Fusiona duplicados del archivo sumando (con tope). */
            long suma;
            suma = (long)r->entradas[idx].victorias + (long)vic;
            if (suma > RANKING_VICTORIAS_MAX) {
                suma = RANKING_VICTORIAS_MAX;
            }
            r->entradas[idx].victorias = (int)suma;
        } else {
            if (r->cantidad >= RANKING_MAX_JUGADORES) {
                hubo_formato = 1;
                continue;
            }
            strcpy(r->entradas[r->cantidad].nombre, nombre);
            r->entradas[r->cantidad].victorias = vic;
            r->cantidad++;
        }
    }
    fclose(f);
    ranking_ordenar(r);
    if (hubo_formato) {
        return RANKING_ERROR_FORMATO;
    }
    return RANKING_OK;
}

int ranking_guardar(const Ranking *r, const char *ruta)
{
    FILE *f;
    int i;

    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }
    f = fopen(ruta, "w");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }
    for (i = 0; i < r->cantidad; i++) {
        if (fprintf(f, "%s;%d\n", r->entradas[i].nombre, r->entradas[i].victorias) < 0) {
            fclose(f);
            return RANKING_ERROR_ARCHIVO;
        }
    }
    if (fclose(f) != 0) {
        return RANKING_ERROR_ARCHIVO;
    }
    return RANKING_OK;
}

const char *ranking_mensaje_error(int codigo)
{
    switch (codigo) {
        case RANKING_OK:
            return "Sin error";
        case RANKING_ERROR_ARGUMENTO:
            return "Argumento invalido";
        case RANKING_ERROR_NOMBRE_INVALIDO:
            return "Nombre invalido (1 a 31 caracteres, sin ';')";
        case RANKING_ERROR_LLENO:
            return "Ranking lleno";
        case RANKING_ERROR_ARCHIVO:
            return "No se pudo leer o guardar el archivo";
        case RANKING_ERROR_FORMATO:
            return "Archivo con lineas invalidas (se cargaron las validas)";
        default:
            return "Error desconocido";
    }
}

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

void ranking_mostrar(const Ranking *r)
{
    if (r == NULL) {
        return;
    }
    ranking_mostrar_en(stdout, r);
}
