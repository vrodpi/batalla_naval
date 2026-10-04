/*
 * test_ranking.c - Pruebas del modulo ranking (sin frameworks externos).
 *
 * Compilar: gcc -ansi -pedantic -Wall -Wextra -Iinclude \
 *               tests/test_ranking.c src/ranking.c -o build/test_ranking
 * Ejecutar desde la raiz: ./build/test_ranking (o build\test_ranking.exe)
 * Devuelve 0 si todo pasa, 1 si algo falla.
 */

#include <stdio.h>
#include <string.h>

#include "ranking.h"

static int fallos = 0;
static int pruebas = 0;

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

/* Escribe un archivo de texto para pruebas de carga. */
static int escribir_texto(const char *ruta, const char *contenido)
{
    FILE *f;
    f = fopen(ruta, "w");
    if (f == NULL) {
        return 0;
    }
    fputs(contenido, f);
    fclose(f);
    return 1;
}

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
}

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
    rc = ranking_registrar_victoria(&r, "A;B");
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "nombre con ; invalido");
    rc = ranking_registrar_victoria(&r, NULL);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "nombre NULL argumento");
    rc = ranking_registrar_victoria(NULL, "Ana");
    chequear(rc == RANKING_ERROR_ARGUMENTO, "ranking NULL argumento");

    for (i = 0; i < 79; i++) {
        largo[i] = 'x';
    }
    largo[79] = '\0';
    rc = ranking_registrar_victoria(&r, largo);
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "nombre muy largo invalido");
    chequear(ranking_cantidad(&r) == 0, "nada se agrego con invalidos");
}

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
    /* Esperado: Luis 3, Ana 2, Zoe 1 */
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

    /* Empate: alfabetico insensible. */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "beto");
    ranking_registrar_victoria(&r, "Ana");
    e = ranking_obtener(&r, 0);
    chequear(e != NULL && strcmp(e->nombre, "Ana") == 0,
             "empate: Ana antes que beto");
}

static void prueba_guardar_cargar(void)
{
    Ranking r;
    Ranking r2;
    int rc;
    const char *tmp = "data/ranking_test_tmp.dat";

    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Luis");

    rc = ranking_guardar(&r, tmp);
    chequear(rc == RANKING_OK, "guardar roundtrip ok");

    ranking_inicializar(&r2);
    rc = ranking_cargar(&r2, tmp);
    chequear(rc == RANKING_OK, "cargar roundtrip ok");
    chequear(ranking_cantidad(&r2) == 2, "roundtrip cantidad 2");
    chequear(ranking_obtener_victorias(&r2, "ana") == 2, "roundtrip Ana 2");
    chequear(ranking_obtener_victorias(&r2, "LUIS") == 1, "roundtrip Luis 1");

    /* Limpieza del archivo temporal. */
    remove(tmp);
}

static void prueba_archivo_inexistente(void)
{
    Ranking r;
    int rc;
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, "data/no_existe_12345.dat");
    chequear(rc == RANKING_ERROR_ARCHIVO, "archivo inexistente -> ERROR_ARCHIVO");
    chequear(ranking_cantidad(&r) == 0, "archivo inexistente deja vacio");
}

static void prueba_formato(void)
{
    Ranking r;
    int rc;

    ranking_inicializar(&r);
    chequear(escribir_texto("data/ranking_test_malo.dat",
            "Ana;2\n"
            "linea sin separador\n"
            "Luis;xyz\n"
            "\n"
            "Beto;-1\n"
            "Zoe;1\n") == 1, "escribir archivo con errores (setup)");

    rc = ranking_cargar(&r, "data/ranking_test_malo.dat");
    chequear(rc == RANKING_ERROR_FORMATO, "lineas malas -> ERROR_FORMATO");
    chequear(ranking_obtener_victorias(&r, "Ana") == 2, "linea valida Ana se carga");
    chequear(ranking_obtener_victorias(&r, "Zoe") == 1, "linea valida Zoe se carga");
    chequear(ranking_cantidad(&r) == 2, "solo 2 validas");
    remove("data/ranking_test_malo.dat");

    /* Duplicados en archivo se fusionan. */
    chequear(escribir_texto("data/ranking_test_dup.dat",
            "Ana;2\nana;3\n") == 1, "escribir duplicados (setup)");
    ranking_inicializar(&r);
    rc = ranking_cargar(&r, "data/ranking_test_dup.dat");
    chequear(rc == RANKING_OK, "duplicados no son error fatal");
    chequear(ranking_cantidad(&r) == 1, "duplicados fusionan en 1");
    chequear(ranking_obtener_victorias(&r, "ANA") == 5, "duplicados suman 2+3=5");
    remove("data/ranking_test_dup.dat");
}

static void prueba_lleno(void)
{
    Ranking r;
    int i;
    int rc;
    char nombre[16];

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
}

static void prueba_mostrar(void)
{
    Ranking r;
    int rc;
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    rc = ranking_mostrar_en(stdout, &r);
    chequear(rc == RANKING_OK, "mostrar_en ok");
    rc = ranking_mostrar_en(NULL, &r);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "mostrar_en NULL -> ARGUMENTO");
    chequear(ranking_obtener(&r, 99) == NULL, "obtener fuera de rango NULL");
    chequear(ranking_mensaje_error(RANKING_OK) != NULL, "mensaje_error no NULL");
}

int main(void)
{
    printf("== test_ranking ==\n");
    prueba_basica();
    prueba_mayusculas_espacios();
    prueba_nombres_invalidos();
    prueba_orden();
    prueba_guardar_cargar();
    prueba_archivo_inexistente();
    prueba_formato();
    prueba_lleno();
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
