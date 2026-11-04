/**
 * @file test_ranking.c
 * @brief Pruebas automaticas del modulo ranking (formato binario).
 *
 * Estrategia de prueba (sin frameworks externos)
 * El proyecto trabaja con C99 y sin dependencias externas: por eso no se
 * usa CUnit, Unity ni nada parecido. El "framework" son dos piezas minimas:
 *   - `pruebas` cuenta cuantas aserciones se ejecutaron;
 *   - `fallos` cuenta cuantas fallaron;
 *   - chequear(condicion, "nombre del caso") imprime "ok" o "FALLO" y
 *     actualiza ambos contadores.
 * El main corre todas las baterias, imprime el resumen y devuelve 0 si
 * todo paso o 1 si hubo al menos un fallo: asi `make test` y cualquier
 * script detectan el resultado por el codigo de salida.
 *
 * IMPORTANTE: como el archivo ahora es binario, las pruebas de lectura
 * NO pueden fabricar casos corruptos con un editor de texto. Por eso
 * este archivo usa fwrite() a proposito (escribir_crudo/escribir_bytes):
 * el test debe poder generar exactamente los bytes que el modulo leera,
 * incluidos los invalidos que ranking_guardar nunca produce (cantidad
 * fuera de rango, nombres sin '\0', victorias negativas, archivos
 * truncados o con bytes de mas). Es el equivalente a "editar el .dat a
 * mano" en la version texto.
 *
 * Cada funcion prueba_* verifica UN aspecto del contrato de ranking.h:
 *   prueba_basica ............. registrar y acumular victorias;
 *   prueba_mayusculas_espacios  identidad insensible (decision de reglas);
 *   prueba_nombres_invalidos .. validacion de nombres;
 *   prueba_orden .............. criterio de orden y desempate;
 *   prueba_guardar_cargar ..... ida y vuelta binaria (layout y tamano);
 *   prueba_archivo_inexistente  primera ejecucion sin ranking.dat;
 *   prueba_binario_corrupto ... archivo vacio/truncado/sobrante/campos
 *                               invalidos y carga parcial;
 *   prueba_duplicados ......... fusion de duplicados manipulados;
 *   prueba_guardado_determinista bytes no usados siempre en cero;
 *   prueba_lleno .............. tope de 100 jugadores;
 *   prueba_mostrar ............ salida por pantalla y casos borde.
 * Las pruebas de archivo usan nombres data/ranking_test_*.dat (ignorados
 * por .gitignore) y borran con remove() al terminar, ganado o perdido,
 * para no dejar basura ni contaminar el ranking.dat real.
 *
 * Compilar: gcc -std=c99 -pedantic -Wall -Wextra -Iinclude \
 *               tests/test_ranking.c src/ranking.c -o build/test_ranking
 * Ejecutar desde la raiz: ./build/test_ranking (o build\test_ranking.exe)
 * Devuelve 0 si todo pasa, 1 si algo falla.
 */

#include <stdio.h>
#include <string.h>

#include "ranking.h"

/**
 * @brief Cantidad de aserciones que fallaron.
 */
static int fallos = 0;

/**
 * @brief Cantidad total de aserciones ejecutadas por la suite.
 */
static int pruebas = 0;

/**
 * @brief Registra una asercion y actualiza los contadores globales.
 *
 * @param condicion Resultado de la condicion que se quiere verificar.
 * @param nombre Texto descriptivo de la invariante comprobada.
 * @return No devuelve valor.
 * @note Si la condicion falla, imprime "FALLO" y suma uno al contador de
 *       fallos. El nombre debe describir el caso porque es lo que se lee
 *       cuando algo se rompe.
 */
static void chequear(int condicion, const char *nombre)
{
    /* Toda asercion ejecutada cuenta, haya pasado o no. */
    pruebas++;
    if (condicion) {
        printf("ok   - %s\n", nombre);
    } else {
        /* Un fallo se informa y se acumula para el codigo de salida. */
        printf("FALLO- %s\n", nombre);
        fallos++;
    }
}

/**
 * @brief Devuelve el tamano en bytes de un archivo.
 *
 * @param ruta Ruta del archivo que se quiere medir.
 * @return Tamano en bytes, o -1 si no se puede abrir o posicionar.
 * @note Se usa para verificar que el archivo binario mida EXACTAMENTE
 *       sizeof(Ranking). Implementado con fseek(SEEK_END) + ftell, portable
 *       en C99.
 */
static long tamano_archivo(const char *ruta)
{
    FILE *f; /* Archivo abierto en binario. */
    long n;  /* Tamano medido. */

    /* Abrir en modo binario para no alterar la medicion. */
    f = fopen(ruta, "rb");
    if (f == NULL) {
        return -1;
    }

    /* Posicionar al final para poder consultar la posicion con ftell. */
    if (fseek(f, 0L, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }

    /* ftell devuelve la cantidad de bytes; -1 indica error. */
    n = ftell(f);
    fclose(f);
    return n;
}

/**
 * @brief Escribe un Ranking completo tal cual esta en memoria.
 *
 * @param ruta Ruta del archivo que se crea o sobrescribe.
 * @param r Ranking que se vuelca sin normalizar.
 * @return 1 si pudo escribir; 0 si fallo la apertura o la escritura.
 * @note Es el mismo layout que usa ranking_guardar, pero sin la normalizacion
 *       de bytes libres: permite construir a mano archivos con cantidad
 *       invalida o nombres rotos para probar la validacion de lectura.
 */
static int escribir_crudo(const char *ruta, const Ranking *r)
{
    FILE *f; /* Archivo de salida binario. */

    /* Se abre en "wb" para volcar los bytes sin traduccion de Windows. */
    f = fopen(ruta, "wb");
    if (f == NULL) {
        return 0;
    }

    /* Un unico fwrite del struct completo, tal como esta en memoria. */
    if (fwrite(r, sizeof(*r), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

/**
 * @brief Escribe `n` bytes arbitrarios en un archivo.
 *
 * @param ruta Ruta del archivo que se crea o sobrescribe.
 * @param datos Puntero a los bytes de origen.
 * @param n Cantidad de bytes a escribir.
 * @return 1 si pudo escribir; 0 si fallo la apertura o la escritura.
 * @note Sirve para simular archivos vacios, truncados o el antiguo formato
 *       texto.
 */
static int escribir_bytes(const char *ruta, const void *datos, size_t n)
{
    FILE *f; /* Archivo de salida binario. */

    /* "wb" evita cualquier traduccion de fin de linea. */
    f = fopen(ruta, "wb");
    if (f == NULL) {
        return 0;
    }

    /* n puede ser 0: en ese caso se crea un archivo vacio valido. */
    if (n > 0 && fwrite(datos, 1, n, f) != n) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

/**
 * @brief Compara dos archivos byte a byte.
 *
 * @param a Ruta del primer archivo.
 * @param b Ruta del segundo archivo.
 * @return 1 si son identicos; 0 si difieren o si alguno no se puede abrir.
 * @note Se usa para demostrar que guardar es deterministico: mismos datos en
 *       memoria producen exactamente los mismos bytes en disco, aunque los
 *       slots libres de una de las copias esten sucios.
 */
static int archivos_iguales(const char *a, const char *b)
{
    FILE *fa;   /* Primer archivo. */
    FILE *fb;   /* Segundo archivo. */
    int ca;     /* Byte leido de a. */
    int cb;     /* Byte leido de b. */
    int iguales;/* Resultado de la comparacion. */

    /* Ambos archivos se abren en binario para comparar bytes crudos. */
    fa = fopen(a, "rb");
    fb = fopen(b, "rb");
    if (fa == NULL || fb == NULL) {
        /* Cerrar solo el que se haya abierto. */
        if (fa != NULL) {
            fclose(fa);
        }
        if (fb != NULL) {
            fclose(fb);
        }
        return 0;
    }

    iguales = 1;

    /* Comparar hasta que ambos lleguen a EOF. */
    do {
        ca = fgetc(fa);
        cb = fgetc(fb);
        if (ca != cb) {
            iguales = 0;
            break;
        }
    } while (ca != EOF);

    fclose(fa);
    fclose(fb);
    return iguales;
}

/**
 * @brief Prueba el circuito minimo de registro de victorias.
 *
 * @return No devuelve valor.
 * @note Inicializar deja cantidad en 0; registrar un nombre nuevo lo crea con
 *       1 victoria; registrarlo de nuevo NO crea un segundo jugador sino que
 *       incrementa a 2.
 */
static void prueba_basica(void)
{
    Ranking r; /* Ranking de trabajo. */
    int rc;    /* Codigo devuelto por el modulo. */

    /* Un ranking recien inicializado no tiene jugadores. */
    ranking_inicializar(&r);
    chequear(ranking_cantidad(&r) == 0, "inicializar deja cantidad 0");

    /* Primer registro: crea la entrada con una victoria. */
    rc = ranking_registrar_victoria(&r, "Ana");
    chequear(rc == RANKING_OK, "registrar Ana ok");
    chequear(ranking_cantidad(&r) == 1, "cantidad 1 tras Ana");
    chequear(ranking_obtener_victorias(&r, "Ana") == 1, "Ana tiene 1");

    /* Segundo registro del mismo nombre: acumula, no duplica. */
    rc = ranking_registrar_victoria(&r, "Ana");
    chequear(rc == RANKING_OK, "segunda victoria Ana ok");
    chequear(ranking_obtener_victorias(&r, "Ana") == 2, "Ana tiene 2");
}

/**
 * @brief Prueba la identidad insensible a mayusculas y espacios.
 *
 * @return No devuelve valor.
 * @note "Luis", "luis" y "  LUIS  " deben colapsar en UNA sola entrada con 3
 *       victorias, y buscar() debe encontrarla en cualquier combinacion de
 *       mayusculas.
 */
static void prueba_mayusculas_espacios(void)
{
    Ranking r; /* Ranking de trabajo. */

    ranking_inicializar(&r);

    /* Tres grafias del mismo nombre: deben acumular en una sola entrada. */
    ranking_registrar_victoria(&r, "Luis");
    ranking_registrar_victoria(&r, "luis");
    ranking_registrar_victoria(&r, "  LUIS  ");

    chequear(ranking_cantidad(&r) == 1, "mayusculas/espacios son mismo jugador");
    chequear(ranking_obtener_victorias(&r, "lUiS") == 3, "3 victorias acumuladas");
    chequear(ranking_buscar(&r, "LUIS") == 0, "buscar insensible encuentra");
}

/**
 * @brief Prueba el rechazo de nombres invalidos.
 *
 * @return No devuelve valor.
 * @note Se rechazan vacio, solo espacios, NULL por ambos lados y un nombre de
 *       79 caracteres (supera el maximo de 31). Tambien se confirma que
 *       ningun intento invalido dejo jugadores fantasma y que el ';' SI se
 *       acepta porque en binario ya no es separador.
 */
static void prueba_nombres_invalidos(void)
{
    Ranking r;      /* Ranking de trabajo. */
    int rc;         /* Codigo devuelto. */
    char largo[80]; /* Nombre deliberadamente demasiado largo. */
    int i;          /* Indice de llenado. */

    ranking_inicializar(&r);

    /* Casos invalidos basicos: vacio, solo espacios y punteros nulos. */
    rc = ranking_registrar_victoria(&r, "");
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "nombre vacio invalido");
    rc = ranking_registrar_victoria(&r, "   ");
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "solo espacios invalido");
    rc = ranking_registrar_victoria(&r, NULL);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "nombre NULL argumento");
    rc = ranking_registrar_victoria(NULL, "Ana");
    chequear(rc == RANKING_ERROR_ARGUMENTO, "ranking NULL argumento");

    /* Nombre de 79 caracteres: supera el maximo de 31. */
    for (i = 0; i < 79; i++) {
        largo[i] = 'x';
    }
    largo[79] = '\0';
    rc = ranking_registrar_victoria(&r, largo);
    chequear(rc == RANKING_ERROR_NOMBRE_INVALIDO, "nombre muy largo invalido");
    chequear(ranking_cantidad(&r) == 0, "nada se agrego con invalidos");

    /* El ';' es valido porque el formato binario no usa separadores. */
    ranking_inicializar(&r);
    rc = ranking_registrar_victoria(&r, "A;B");
    chequear(rc == RANKING_OK, "nombre con ; valido en binario");
    chequear(ranking_obtener_victorias(&r, "a;b") == 1,
             "nombre con ; se recupera");
}

/**
 * @brief Prueba el criterio de orden y el desempate alfabetico.
 *
 * @return No devuelve valor.
 * @note Caso de tres niveles (Luis 3 > Ana 2 > Zoe 1) accedido por puesto con
 *       ranking_obtener(0..2), y empate 1-1 donde "Ana" debe quedar antes que
 *       "beto" aunque se haya registrado despues.
 */
static void prueba_orden(void)
{
    Ranking r;                  /* Ranking de trabajo. */
    const RankingEntrada *e;    /* Entrada consultada por puesto. */

    /* Construir tres jugadores con 1, 2 y 3 victorias. */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Zoe");
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Luis");
    ranking_registrar_victoria(&r, "Luis");
    ranking_registrar_victoria(&r, "Luis");

    /* Esperado: Luis 3, Ana 2, Zoe 1. */
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

    /* Empate a una victoria: decide el nombre alfabetico insensible. */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "beto");
    ranking_registrar_victoria(&r, "Ana");
    e = ranking_obtener(&r, 0);
    chequear(e != NULL && strcmp(e->nombre, "Ana") == 0,
             "empate: Ana antes que beto");
}

/**
 * @brief Prueba el roundtrip completo memoria -> disco -> memoria.
 *
 * @return No devuelve valor.
 * @note Verifica que el archivo mida exactamente sizeof(Ranking), que se
 *       recuperen cantidad y victorias, que la busqueda siga siendo insensible
 *       a mayusculas y que la carga no dependa de la memoria original.
 */
static void prueba_guardar_cargar(void)
{
    Ranking r;                      /* Ranking de origen. */
    Ranking r2;                     /* Ranking reconstruido desde disco. */
    int rc;                         /* Codigo devuelto. */
    const char *tmp = "data/ranking_test_tmp.dat"; /* Archivo temporal. */

    /* Datos de origen: Ana con 2 victorias y Luis con 1. */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Ana");
    ranking_registrar_victoria(&r, "Luis");

    /* Guardar y comprobar el tamano exacto del volcado. */
    rc = ranking_guardar(&r, tmp);
    chequear(rc == RANKING_OK, "guardar roundtrip ok");
    chequear(tamano_archivo(tmp) == (long)sizeof(Ranking),
             "archivo binario mide sizeof(Ranking)");

    /* Cargar en un ranking distinto para no depender de la memoria previa. */
    ranking_inicializar(&r2);
    rc = ranking_cargar(&r2, tmp);
    chequear(rc == RANKING_OK, "cargar roundtrip ok");
    chequear(ranking_cantidad(&r2) == 2, "roundtrip cantidad 2");
    chequear(ranking_obtener_victorias(&r2, "ana") == 2, "roundtrip Ana 2");
    chequear(ranking_obtener_victorias(&r2, "LUIS") == 1, "roundtrip Luis 1");

    /* La prueba limpia su propio archivo temporal. */
    remove(tmp);
}

/**
 * @brief Simula la primera ejecucion, cuando ranking.dat no existe.
 *
 * @return No devuelve valor.
 * @note Cargar debe devolver RANKING_ERROR_ARCHIVO (para que app.c muestre el
 *       aviso) pero dejar el ranking vacio y utilizable, no con basura.
 */
static void prueba_archivo_inexistente(void)
{
    Ranking r; /* Ranking que debe quedar vacio. */
    int rc;    /* Codigo devuelto por la carga. */

    ranking_inicializar(&r);

    /* Ruta que no existe: se espera el codigo especifico de archivo. */
    rc = ranking_cargar(&r, "data/no_existe_12345.dat");
    chequear(rc == RANKING_ERROR_ARCHIVO, "archivo inexistente -> ERROR_ARCHIVO");
    chequear(ranking_cantidad(&r) == 0, "archivo inexistente deja vacio");
}

/**
 * @brief Bateria principal de validacion del formato binario.
 *
 * @return No devuelve valor.
 * @note Cada caso fabrica un archivo que ranking_guardar jamas generaria y
 *       verifica que cargar lo detecte sin caerse y sin cargar basura:
 *       archivo vacio, truncado, con byte extra, cantidad fuera de rango,
 *       victorias negativas, nombre sin '\0', carga parcial y formato texto
 *       viejo.
 */
static void prueba_binario_corrupto(void)
{
    Ranking r;
    Ranking leido;
    const char *ruta = "data/ranking_test_bin.dat";
    int rc;

    /* --- archivo vacio (0 bytes) --- */
    chequear(escribir_bytes(ruta, "", 0) == 1, "crear archivo vacio (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "archivo vacio -> FORMATO");
    chequear(ranking_cantidad(&leido) == 0, "archivo vacio deja ranking vacio");

    /* --- archivo truncado (falta 1 byte) --- */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    chequear(escribir_bytes(ruta, &r, sizeof(Ranking) - 1) == 1,
             "crear archivo truncado (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "archivo truncado -> FORMATO");

    /* --- archivo con un byte de mas --- */
    {
        FILE *f;
        f = fopen(ruta, "wb");
        chequear(f != NULL, "abrir archivo con sobrante (setup)");
        if (f != NULL) {
            fwrite(&r, sizeof(r), 1, f);
            fputc('X', f);
            fclose(f);
        }
        ranking_inicializar(&leido);
        rc = ranking_cargar(&leido, ruta);
        chequear(rc == RANKING_ERROR_FORMATO, "archivo con byte extra -> FORMATO");
    }

    /* --- cantidad fuera de rango: -1 y 101 --- */
    r.cantidad = -1;
    chequear(escribir_crudo(ruta, &r) == 1, "crear cantidad -1 (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "cantidad -1 -> FORMATO");

    r.cantidad = RANKING_MAX_JUGADORES + 1;
    chequear(escribir_crudo(ruta, &r) == 1, "crear cantidad 101 (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "cantidad 101 -> FORMATO");

    /* --- victorias negativas --- */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    r.entradas[0].victorias = -5;
    chequear(escribir_crudo(ruta, &r) == 1, "crear victorias -5 (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "victorias negativas -> FORMATO");
    chequear(ranking_cantidad(&leido) == 0, "victorias negativas se descarta");

    /* --- nombre sin '\0' dentro de los 32 bytes --- */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    memset(r.entradas[0].nombre, 'A', RANKING_NOMBRE_MAX);
    chequear(escribir_crudo(ruta, &r) == 1, "crear nombre sin fin (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "nombre sin '\\0' -> FORMATO");
    chequear(ranking_cantidad(&leido) == 0, "nombre sin '\\0' se descarta");

    /* --- una entrada mala + una buena: carga parcial --- */
    memset(&r, 0, sizeof(r));
    r.cantidad = 2;
    strcpy(r.entradas[0].nombre, "Ana");
    r.entradas[0].victorias = -1;
    strcpy(r.entradas[1].nombre, "Zoe");
    r.entradas[1].victorias = 2;
    chequear(escribir_crudo(ruta, &r) == 1, "crear mixta (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "entrada mala + buena -> FORMATO");
    chequear(ranking_cantidad(&leido) == 1, "carga parcial: queda 1 valida");
    chequear(ranking_obtener_victorias(&leido, "Zoe") == 2, "carga parcial: Zoe 2");

    /* --- formato texto viejo (migracion) --- */
    chequear(escribir_bytes(ruta, "Ana;3\n", 6) == 1, "crear texto viejo (setup)");
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_ERROR_FORMATO, "ranking.dat texto viejo -> FORMATO");
    chequear(ranking_cantidad(&leido) == 0, "texto viejo no carga basura");

    remove(ruta);
}

/**
 * @brief Prueba la fusion de duplicados insensibles a mayusculas.
 *
 * @return No devuelve valor.
 * @note Un archivo manipulado puede tener dos entradas que son el mismo
 *       jugador ("Ana" y "ana"). Cargar no debe fallar: las fusiona sumando
 *       (2 + 3 = 5), igual que hacia la version texto.
 */
static void prueba_duplicados(void)
{
    Ranking r;                      /* Ranking fabricado a mano. */
    Ranking leido;                  /* Ranking reconstruido. */
    const char *ruta = "data/ranking_test_dup.dat"; /* Archivo temporal. */
    int rc;                         /* Codigo devuelto. */

    /* Fabricar dos entradas que solo difieren en mayusculas. */
    memset(&r, 0, sizeof(r));
    r.cantidad = 2;
    strcpy(r.entradas[0].nombre, "Ana");
    r.entradas[0].victorias = 2;
    strcpy(r.entradas[1].nombre, "ana");
    r.entradas[1].victorias = 3;
    chequear(escribir_crudo(ruta, &r) == 1, "crear duplicados (setup)");

    /* La carga debe fusionarlas y sumar las victorias. */
    ranking_inicializar(&leido);
    rc = ranking_cargar(&leido, ruta);
    chequear(rc == RANKING_OK, "duplicados no son error fatal");
    chequear(ranking_cantidad(&leido) == 1, "duplicados fusionan en 1");
    chequear(ranking_obtener_victorias(&leido, "ANA") == 5, "duplicados suman 2+3=5");

    remove(ruta);
}

/**
 * @brief Comprueba que guardar no vuelque basura de memoria.
 *
 * @return No devuelve valor.
 * @note Se construyen dos Rankings con los mismos datos; a uno se le ensucian
 *       los slots libres y la cola del primer nombre con bytes 0xAB/0xCD. Al
 *       guardar ambos, los archivos deben ser identicos byte a byte, porque
 *       ranking_guardar normaliza toda la copia a cero antes de escribir.
 */
static void prueba_guardado_determinista(void)
{
    Ranking a; /* Copia que se ensucia a proposito. */
    Ranking b; /* Copia limpia de referencia. */
    const char *r1 = "data/ranking_test_det1.dat"; /* Salida sucia. */
    const char *r2 = "data/ranking_test_det2.dat"; /* Salida limpia. */
    int i;     /* Indice de llenado. */

    /* Mismos datos en ambas copias. */
    ranking_inicializar(&a);
    ranking_registrar_victoria(&a, "Ana");
    ranking_registrar_victoria(&a, "Luis");
    b = a;

    /* Ensuciar slots libres y la cola del nombre del primero. */
    for (i = a.cantidad; i < RANKING_MAX_JUGADORES; i++) {
        memset(&a.entradas[i], 0xAB, sizeof(RankingEntrada));
    }
    for (i = (int)strlen(a.entradas[0].nombre) + 1; i < RANKING_NOMBRE_MAX; i++) {
        a.entradas[0].nombre[i] = (char)0xCD;
    }

    /* A pesar de la basura, los dos archivos deben ser identicos. */
    chequear(ranking_guardar(&a, r1) == RANKING_OK, "guardar copia sucia ok");
    chequear(ranking_guardar(&b, r2) == RANKING_OK, "guardar copia limpia ok");
    chequear(archivos_iguales(r1, r2) == 1,
             "guardado determinista: bytes libres en cero");

    remove(r1);
    remove(r2);
}

/**
 * @brief Prueba el tope de RANKING_MAX_JUGADORES.
 *
 * @return No devuelve valor.
 * @note Llena el ranking con los nombres "J000".."J099" generados con sprintf
 *       y verifica que el jugador 101 sea rechazado con RANKING_ERROR_LLENO
 *       en vez de desbordar el arreglo.
 */
static void prueba_lleno(void)
{
    Ranking r;      /* Ranking que se llena. */
    int i;          /* Contador de jugadores. */
    int rc;         /* Codigo devuelto. */
    char nombre[16];/* Buffer para "J000".."J099". */

    ranking_inicializar(&r);
    rc = RANKING_OK;

    /* Registrar exactamente el maximo de jugadores distintos. */
    for (i = 0; i < RANKING_MAX_JUGADORES; i++) {
        sprintf(nombre, "J%03d", i);
        rc = ranking_registrar_victoria(&r, nombre);
        if (rc != RANKING_OK) {
            break;
        }
    }
    chequear(rc == RANKING_OK, "llenar hasta MAX ok");
    chequear(ranking_cantidad(&r) == RANKING_MAX_JUGADORES, "cantidad == MAX");

    /* El jugador siguiente debe ser rechazado, no desbordar el arreglo. */
    rc = ranking_registrar_victoria(&r, "UnoMas");
    chequear(rc == RANKING_ERROR_LLENO, "uno mas alla de MAX -> LLENO");
}

/**
 * @brief Prueba la impresion, los accesos fuera de rango y los mensajes.
 *
 * @return No devuelve valor.
 * @note La impresion no debe fallar con datos validos (ademas deja la tabla a
 *       la vista), debe rechazar salida NULL con ARGUMENTO, el acceso fuera de
 *       rango debe dar NULL y el traductor de errores nunca debe devolver NULL
 *       (la interfaz lo imprime directo con %s).
 */
static void prueba_mostrar(void)
{
    Ranking r; /* Ranking de trabajo. */
    int rc;    /* Codigo devuelto. */

    /* Impresion valida: no debe fallar. */
    ranking_inicializar(&r);
    ranking_registrar_victoria(&r, "Ana");
    rc = ranking_mostrar_en(stdout, &r);
    chequear(rc == RANKING_OK, "mostrar_en ok");

    /* Casos borde: salida nula, indice fuera de rango y mensaje seguro. */
    rc = ranking_mostrar_en(NULL, &r);
    chequear(rc == RANKING_ERROR_ARGUMENTO, "mostrar_en NULL -> ARGUMENTO");
    chequear(ranking_obtener(&r, 99) == NULL, "obtener fuera de rango NULL");
    chequear(ranking_mensaje_error(RANKING_OK) != NULL, "mensaje_error no NULL");
}

/**
 * @brief Ejecuta todas las baterias y resume el resultado.
 *
 * @return 0 si todas las aserciones pasaron; 1 si hubo al menos un fallo.
 * @note El codigo de salida es lo que permite que `make test` detecte el
 *       resultado sin depender de analizar el texto impreso.
 */
int main(void)
{
    /* Encabezado de la suite. */
    printf("== test_ranking (binario) ==\n");

    /* Cada bateria cubre un aspecto independiente del contrato. */
    prueba_basica();
    prueba_mayusculas_espacios();
    prueba_nombres_invalidos();
    prueba_orden();
    prueba_guardar_cargar();
    prueba_archivo_inexistente();
    prueba_binario_corrupto();
    prueba_duplicados();
    prueba_guardado_determinista();
    prueba_lleno();
    prueba_mostrar();

    /* Resumen final y codigo de salida para make/CI. */
    printf("------------------\n");
    printf("Pruebas: %d, Fallos: %d\n", pruebas, fallos);
    if (fallos == 0) {
        printf("TODO OK\n");
        return 0;
    }
    printf("HAY FALLOS\n");
    return 1;
}
