/**
 * @file ranking.c
 * @brief Ranking de victorias y persistencia binaria.
 *
 * Formato (Opcion A): volcado directo de Ranking, 3604 bytes en x86_64.
 *   entradas[100] (32 bytes de nombre + 4 de victorias) + cantidad (4).
 * Se abre siempre en modo binario. Al guardar se normaliza una copia a cero
 * para que el archivo sea deterministico; al cargar se validan tamano,
 * cantidad, nombres y victorias.
 *
 * Invariantes: solo las primeras `cantidad` entradas son validas y el arreglo
 * queda ordenado despues de cargar o registrar.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../include/ranking.h"
/*
static_assert(sizeof(RankingEntrada) == (RANKING_NOMBRE_MAX + sizeof(int)),
              "RankingEntrada no debe tener relleno");

static_assert(sizeof(Ranking) == (RANKING_MAX_JUGADORES * sizeof(RankingEntrada)
                                  + sizeof(int)),
              "Ranking no debe tener relleno");
*/
/**
 * @brief Copia `origen` sin espacios en los extremos.
 *
 * Si el resultado no entra en `destino`, lo trunca a tam_destino - 1
 * caracteres. `destino` siempre queda terminado en '\0' cuando los argumentos
 * son validos.
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

/**
 * @brief Compara dos nombres ignorando mayusculas.
 *
 * Ordena como strcmp sobre las versiones en minuscula. NULL se considera
 * menor que cualquier cadena y dos NULL son iguales.
 *
 * @return <0, 0 o >0 segun el orden.
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

/**
 * @brief Valida un nombre y lo deja listo para el ranking.
 *
 * Recorta espacios de los extremos y exige entre 1 y 31 caracteres utiles,
 * sin caracteres de control. La longitud se mide sobre el original para
 * rechazar nombres que no entren en RANKING_NOMBRE_MAX; recien despues se
 * copia al buffer de salida.
 *
 * @return 1 si el nombre es valido; 0 si esta vacio, es demasiado largo o
 *         contiene controles.
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

/**
 * @brief Valida una entrada leida del archivo binario.
 *
 * Exige que `nombre` tenga '\0' dentro de los 32 bytes, que pase las reglas
 * de nombre_normalizado() y que `victorias` este en el rango permitido. La
 * version normalizada se devuelve en `nombre`.
 *
 * @return 1 si la entrada puede incorporarse al ranking; 0 si esta corrupta.
 */
static int entrada_valida(const RankingEntrada *e, char *nombre)
{
    int i;
    int tiene_fin;

    if (e == NULL || nombre == NULL) {
        return 0;
    }

    tiene_fin = 0;
    for (i = 0; i < RANKING_NOMBRE_MAX; i++) {
        if (e->nombre[i] == '\0') {
            tiene_fin = 1;
            break;
        }
    }
    if (!tiene_fin) {
        return 0;
    }

    if (!nombre_normalizado(e->nombre, nombre, RANKING_NOMBRE_MAX)) {
        return 0;
    }

    if (e->victorias < 0 || e->victorias > RANKING_VICTORIAS_MAX) {
        return 0;
    }
    return 1;
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

/**
 * @brief Busca un jugador por nombre sin distinguir mayusculas.
 *
 * Normaliza el nombre recibido y recorre las entradas validas comparando con
 * comparar_insensible(), de modo que "ana" y "ANA" son el mismo jugador.
 *
 * @return indice dentro de entradas[]; -1 si no existe o el nombre es
 *         invalido.
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

int ranking_obtener_victorias(const Ranking *r, const char *nombre)
{
    int idx;

    idx = ranking_buscar(r, nombre);
    if (idx < 0) {
        return 0;
    }
    return r->entradas[idx].victorias;
}

/**
 * @brief Ordena el ranking con insercion, en el lugar.
 *
 * Criterio: mas victorias primero; a igualdad, nombre alfabetico sin
 * distinguir mayusculas; si aun empatan, strcmp sensible para que el
 * resultado sea determinista. La insercion es O(n^2), adecuada para un
 * maximo de 100 entradas.
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

/**
 * @brief Suma una victoria a un jugador, creandolo si no existe.
 *
 * Busca por nombre normalizado. Si existe, incrementa hasta
 * RANKING_VICTORIAS_MAX; si es nuevo, lo agrega con una victoria. En ambos
 * casos deja el arreglo ordenado. Un nombre invalido no modifica el ranking.
 *
 * @return RANKING_OK, RANKING_ERROR_ARGUMENTO, RANKING_ERROR_NOMBRE_INVALIDO
 *         o RANKING_ERROR_LLENO.
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

/**
 * @brief Carga el ranking desde un volcado binario de Ranking.
 *
 * Rechaza archivos con bytes sobrantes, cantidad fuera de rango o entradas
 * corruptas. Las entradas validas se incorporan; los duplicados se fusionan
 * sumando victorias con tope RANKING_VICTORIAS_MAX. Ante cualquier descarte
 * devuelve RANKING_ERROR_FORMATO, pero conserva lo que pudo cargar.
 *
 * @return RANKING_OK o uno de los codigos de error del modulo.
 * @note Si el archivo no existe devuelve RANKING_ERROR_ARCHIVO y deja el
 *       ranking vacio; app.c lo trata como ranking nuevo.
 */
int ranking_cargar(Ranking *r, const char *ruta)
{
    FILE *f;
    Ranking disco;
    size_t leidos;
    int sobrante;
    int hubo_formato;
    int i;

    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }

    r->cantidad = 0;

    f = fopen(ruta, "rb");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }

    leidos = fread(&disco, sizeof(disco), 1, f);
    if (leidos != 1) {
        int fallo_io;

        fallo_io = ferror(f);
        fclose(f);
        return fallo_io ? RANKING_ERROR_ARCHIVO : RANKING_ERROR_FORMATO;
    }

    sobrante = fgetc(f);
    fclose(f);
    if (sobrante != EOF) {
        return RANKING_ERROR_FORMATO;
    }

    if (disco.cantidad < 0 || disco.cantidad > RANKING_MAX_JUGADORES) {
        return RANKING_ERROR_FORMATO;
    }

    hubo_formato = 0;

    for (i = 0; i < disco.cantidad; i++) {
        char nombre[RANKING_NOMBRE_MAX];
        int idx;
        int k;

        if (!entrada_valida(&disco.entradas[i], nombre)) {
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
            long suma;

            suma = (long)r->entradas[idx].victorias
                 + (long)disco.entradas[i].victorias;
            if (suma > RANKING_VICTORIAS_MAX) {
                suma = RANKING_VICTORIAS_MAX;
            }
            r->entradas[idx].victorias = (int)suma;
        } else if (r->cantidad < RANKING_MAX_JUGADORES) {
            strcpy(r->entradas[r->cantidad].nombre, nombre);
            r->entradas[r->cantidad].victorias = disco.entradas[i].victorias;
            r->cantidad++;
        } else {
            hubo_formato = 1;
        }
    }

    ranking_ordenar(r);
    if (hubo_formato) {
        return RANKING_ERROR_FORMATO;
    }
    return RANKING_OK;
}

/**
 * @brief Guarda el ranking como un volcado binario de Ranking.
 *
 * Escribe una copia normalizada a cero: solo las `cantidad` entradas usadas
 * tienen datos, de modo que el archivo no filtra bytes de memoria ni varia
 * entre ejecuciones. La cantidad se acota al rango valido antes de escribir.
 *
 * @return RANKING_OK o RANKING_ERROR_ARGUMENTO / RANKING_ERROR_ARCHIVO.
 */
int ranking_guardar(const Ranking *r, const char *ruta)
{
    FILE *f;
    Ranking copia;
    int i;

    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }

    memset(&copia, 0, sizeof(copia));
    copia.cantidad = r->cantidad;

    if (copia.cantidad < 0) {
        copia.cantidad = 0;
    }
    if (copia.cantidad > RANKING_MAX_JUGADORES) {
        copia.cantidad = RANKING_MAX_JUGADORES;
    }

    for (i = 0; i < copia.cantidad; i++) {
        int j;

        for (j = 0; j < RANKING_NOMBRE_MAX - 1
                    && r->entradas[i].nombre[j] != '\0'; j++) {
            copia.entradas[i].nombre[j] = r->entradas[i].nombre[j];
        }
        copia.entradas[i].victorias = r->entradas[i].victorias;
    }

    f = fopen(ruta, "wb");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }

    if (fwrite(&copia, sizeof(copia), 1, f) != 1) {
        fclose(f);
        return RANKING_ERROR_ARCHIVO;
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
            return "Nombre invalido (1 a 31 caracteres, sin controles)";
        case RANKING_ERROR_LLENO:
            return "Ranking lleno";
        case RANKING_ERROR_ARCHIVO:
            return "No se pudo leer o guardar el archivo";
        case RANKING_ERROR_FORMATO:
            return "Archivo binario invalido o corrupto";
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
