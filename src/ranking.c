/**
 * @file ranking.c
 * @brief Ranking de victorias por jugador (logica + persistencia).
 *
 * Mantiene cuantas partidas gano cada jugador y lo guarda en
 * "data/ranking.dat" como BINARIO: una copia cruda del struct Ranking
 * (Opcion A, fwrite/fread del struct completo). Es el modulo del integrante 4
 * (ranking, datos y compilacion).
 *
 * @note Estandar unico del proyecto: C99 (-std=c99 -pedantic -Wall -Wextra).
 * @note Separacion logica / interfaz: ESTE archivo decide que nombre vale,
 *       quien es el mismo jugador, en que orden se lista y como se lee/escribe
 *       el archivo. La interfaz solo pide el nombre por teclado y muestra
 *       mensajes; jamas suma victorias, ordena ni escribe el archivo.
 *
 * @section layout Layout en disco
 * sizeof(Ranking) = 3604 bytes en x86_64 con int de 4 bytes:
 *
 *   offset    0: entradas[0..99], 100 x 36 bytes        = 3600
 *                 +0   char nombre[32]  ASCII + '\0'
 *                 +32  int  victorias   4 bytes little-endian
 *   offset 3600: cantidad               int, 4 bytes
 *
 * @note Los typedefs de "assert" de abajo verifican EN TIEMPO DE COMPILACION
 *       que no exista relleno (padding). Si un compilador agrega bytes entre
 *       nombre y victorias, o al final del struct, la compilacion falla con
 *       "size of array is negative" en lugar de generar un archivo
 *       incompatible en silencio.
 *
 * @section detalles Detalles criticos de la implementacion binaria
 * 1. MODO BINARIO: fopen("wb")/fopen("rb"), nunca "w"/"r". En modo texto,
 *    Windows convierte cada 0x0A en 0x0D 0x0A al escribir y al reves al leer:
 *    un int con valor 10 (0x0A 0x00 0x00 0x00) se corromperia.
 * 2. ESCRITURA DETERMINISTA: no se hace fwrite(r) directo. Se copia el struct
 *    a una variable local, se pone TODO a cero con memset y se copian solo los
 *    datos validos. Asi los slots libres y la cola de cada nombre no filtran
 *    memoria ni cambian en cada guardado.
 * 3. LECTURA VALIDADA: despues de fread se verifica tamano exacto, cantidad en
 *    rango, '\0' dentro de cada nombre y victorias dentro del tope.
 * 4. TAMANO EXACTO: si sobran o faltan bytes se devuelve
 *    RANKING_ERROR_FORMATO. No se intenta adivinar el formato.
 *
 * @section invariantes Invariantes en memoria
 * - INVARIANTE 1: solo las primeras `cantidad` entradas son validas.
 * - INVARIANTE 2: tras registrar/cargar, el arreglo queda ordenado.
 * - INVARIANTE 3: `nombre` de cada entrada usada tiene '\0' dentro de sus
 *   32 bytes (lo garantizan registrar y cargar).
 *
 * @section mapa Mapa de funciones
 * Ayudas internas (static): recortar_extremos, comparar_insensible,
 * nombre_normalizado y entrada_valida. API publica (ranking.h):
 * ranking_inicializar, ranking_cantidad, ranking_obtener, ranking_buscar,
 * ranking_obtener_victorias, ranking_ordenar, ranking_registrar_victoria,
 * ranking_cargar, ranking_guardar, ranking_mensaje_error,
 * ranking_mostrar_en y ranking_mostrar.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ranking.h"

/**
 * @brief Assert de compilacion: RankingEntrada no debe tener relleno.
 * @note Si sizeof(RankingEntrada) no es exactamente 32 + sizeof(int), el
 *       tamano del arreglo es -1 y la compilacion falla.
 */
typedef char ranking_assert_entrada_sin_relleno[
    (sizeof(RankingEntrada) == (RANKING_NOMBRE_MAX + sizeof(int))) ? 1 : -1];

/**
 * @brief Assert de compilacion: Ranking no debe tener relleno.
 * @note Si sizeof(Ranking) no es exactamente 100 * sizeof(RankingEntrada) +
 *       sizeof(int), el tamano del arreglo es -1 y la compilacion falla.
 */
typedef char ranking_assert_ranking_sin_relleno[
    (sizeof(Ranking) == (RANKING_MAX_JUGADORES * sizeof(RankingEntrada)
                         + sizeof(int))) ? 1 : -1];

/**
 * @brief Copia `origen` en `destino` sin espacios en blanco en los extremos.
 *
 * Algoritmo (dos punteros sobre el mismo string):
 *  1. `ini` avanza mientras haya espacios (isspace: ' ', '\t', etc.).
 *  2. Si solo habia espacios, el resultado es "" y se termina.
 *  3. `fin` retrocede desde el '\0' mientras haya espacios.
 *  4. Se copian los caracteres [ini, fin), truncando a tam_destino-1 si hiciera
 *     falta, y siempre se agrega el '\0' final.
 *
 * @param origen Cadena de entrada.
 * @param destino Buffer de salida.
 * @param tam_destino Tamano total del buffer de salida.
 * @return No devuelve valor.
 * @note isspace() exige un valor representable como unsigned char (o EOF).
 *       Pasar un char negativo (letras con tilde en Latin-1/UTF-8) seria
 *       comportamiento indefinido; por eso cada caracter se convierte con
 *       (unsigned char) antes de llamar.
 */
static void recortar_extremos(const char *origen, char *destino, int tam_destino)
{
    const char *ini; /* Primer caracter no espacio. */
    const char *fin; /* Posicion del '\0' recortado. */
    int n;           /* Cantidad de caracteres a copiar. */
    int i;           /* Indice de copia. */

    /* Validacion defensiva de punteros y tamano. */
    if (origen == NULL || destino == NULL || tam_destino <= 0) {
        return;
    }

    /* Avanzar sobre los espacios iniciales. */
    ini = origen;
    while (*ini != '\0' && isspace((unsigned char)*ini)) {
        ini++;
    }

    /* Cadena vacia o compuesta solo por espacios. */
    if (*ini == '\0') {
        destino[0] = '\0';
        return;
    }

    /* Retroceder desde el final sobre los espacios. */
    fin = ini + strlen(ini);
    while (fin > ini && isspace((unsigned char)*(fin - 1))) {
        fin--;
    }

    /* Calcular el largo y truncar si no entra en el destino. */
    n = (int)(fin - ini);
    if (n >= tam_destino) {
        n = tam_destino - 1;
    }

    /* Copia manual y terminacion garantizada. */
    for (i = 0; i < n; i++) {
        destino[i] = ini[i];
    }
    destino[n] = '\0';
}

/**
 * @brief Compara dos strings ignorando mayusculas, como strcasecmp portable.
 *
 * @param a Primer string (puede ser NULL).
 * @param b Segundo string (puede ser NULL).
 * @return 0 si son iguales; negativo si a < b; positivo si a > b. Los NULL se
 *         ordenan antes que cualquier string.
 * @note No se usa strcasecmp/stricmp porque no forman parte de C99:
 *       strcasecmp es POSIX y stricmp es de Windows. Esta version solo usa
 *       tolower() de <ctype.h>.
 */
static int comparar_insensible(const char *a, const char *b)
{
    unsigned char ca; /* Caracter de `a` en minuscula. */
    unsigned char cb; /* Caracter de `b` en minuscula. */

    /* Dos NULL son iguales. */
    if (a == NULL && b == NULL) {
        return 0;
    }
    /* Un NULL se considera menor que cualquier string. */
    if (a == NULL) {
        return -1;
    }
    if (b == NULL) {
        return 1;
    }

    /* Comparar caracter por caracter hasta que alguna cadena termine. */
    while (*a != '\0' && *b != '\0') {
        ca = (unsigned char)tolower((unsigned char)*a);
        cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) {
            return (ca < cb) ? -1 : 1;
        }
        a++;
        b++;
    }

    /* Si ambas terminaron al mismo tiempo, son iguales. */
    if (*a == '\0' && *b == '\0') {
        return 0;
    }
    /* La cadena mas corta es menor. */
    if (*a == '\0') {
        return -1;
    }
    return 1;
}

/**
 * @brief Valida un nombre y produce su forma canonica recortada.
 *
 * Reglas que aplica, en orden:
 *  1. Recorta espacios de los extremos.
 *  2. El largo real (sin contar esos espacios) debe ser 1..31.
 *  3. No puede contener '\n', '\r' ni ningun caracter de control (< 32).
 *     El ';' SI se permite porque en binario no es un separador.
 *
 * @param nombre Nombre original ingresado.
 * @param salida Buffer donde se guarda el nombre normalizado.
 * @param tam_salida Tamano total del buffer de salida.
 * @return 1 si el nombre es valido; 0 si es invalido.
 * @note Se hacen DOS pasadas de medicion. recortar_extremos() trunca en
 *       silencio si el texto no entra en el buffer; si se midiera solo el
 *       resultado recortado, un nombre de 200 caracteres pasaria como valido
 *       de 31. La segunda pasada mide el largo REAL sobre el string original.
 */
static int nombre_normalizado(const char *nombre, char *salida, int tam_salida)
{
    char tmp[RANKING_NOMBRE_MAX]; /* Nombre recortado y truncado. */
    int i;                        /* Indice de recorrido. */
    int largo;                    /* Largo del nombre recortado. */

    /* Validacion defensiva de argumentos. */
    if (nombre == NULL || salida == NULL || tam_salida <= 0) {
        return 0;
    }

    /* Primera pasada: recorte con truncado defensivo. */
    recortar_extremos(nombre, tmp, (int)sizeof(tmp));

    /*
     * Segunda pasada: medir el largo REAL sobre el original, sin copiar y sin
     * truncar, para rechazar nombres que no entran en RANKING_NOMBRE_MAX.
     */
    {
        const char *ini; /* Inicio recortado. */
        const char *fin; /* Final recortado. */
        int real;        /* Largo real del nombre. */

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

    /* El resultado recortado tambien debe tener un largo util. */
    largo = (int)strlen(tmp);
    if (largo <= 0 || largo >= RANKING_NOMBRE_MAX) {
        return 0;
    }

    /* Rechazar caracteres de control dentro del nombre. */
    for (i = 0; i < largo; i++) {
        if ((unsigned char)tmp[i] < 32) {
            return 0;
        }
    }

    /* Copia segura a la salida, respetando su tamano. */
    for (i = 0; i <= largo; i++) {
        if (i < tam_salida) {
            salida[i] = tmp[i];
        }
    }

    /* Terminacion garantizada dentro del buffer de salida. */
    if (tam_salida > 0) {
        salida[tam_salida - 1] = '\0';
    }
    return 1;
}

/**
 * @brief Verifica que una entrada leida del archivo binario sea usable.
 *
 * Antes de llamar a nombre_normalizado() hay que garantizar que exista un
 * '\0' DENTRO de los 32 bytes; si no, strlen() leeria fuera del struct
 * (comportamiento indefinido). Se recorre el arreglo con un tope fijo y recien
 * despues se valida el nombre con las reglas normales. Tambien se rechazan
 * victorias negativas o mayores que el tope.
 *
 * @param e Entrada leida del disco.
 * @param nombre Buffer donde se guarda el nombre normalizado.
 * @return 1 si la entrada es valida; 0 si debe descartarse.
 * @note Es una validacion defensiva: nunca llama a strlen() sobre un nombre
 *       que no tenga '\0' dentro de sus 32 bytes.
 */
static int entrada_valida(const RankingEntrada *e, char *nombre)
{
    int i;         /* Indice dentro de nombre[]. */
    int tiene_fin; /* Bandera: se encontro '\0'. */

    /* Validacion defensiva de punteros. */
    if (e == NULL || nombre == NULL) {
        return 0;
    }

    /* Buscar un '\0' dentro de los 32 bytes sin salir del arreglo. */
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

    /* Validar y normalizar el nombre con las reglas del modulo. */
    if (!nombre_normalizado(e->nombre, nombre, RANKING_NOMBRE_MAX)) {
        return 0;
    }

    /* Rango valido de victorias. */
    if (e->victorias < 0 || e->victorias > RANKING_VICTORIAS_MAX) {
        return 0;
    }
    return 1;
}

/**
 * @brief Deja el ranking vacio (cantidad = 0).
 *
 * @param r Ranking que se reinicia.
 * @return No devuelve valor.
 * @note Debe llamarse antes de cualquier otro uso: es lo que garantiza el
 *       INVARIANTE 1 (solo las primeras `cantidad` entradas son validas).
 */
void ranking_inicializar(Ranking *r)
{
    /* Sin ranking no hay nada que inicializar. */
    if (r == NULL) {
        return;
    }

    /* Un ranking vacio no tiene entradas validas. */
    r->cantidad = 0;
}

/**
 * @brief Devuelve cuantos jugadores distintos hay.
 *
 * @param r Ranking consultado.
 * @return Cantidad de entradas validas; 0 si r es NULL.
 * @note Se usa para recorrer con ranking_obtener(0..cantidad-1) y para que la
 *       futura GUI dibuje tantas filas como haga falta.
 */
int ranking_cantidad(const Ranking *r)
{
    /* Un puntero nulo se trata como ranking vacio. */
    if (r == NULL) {
        return 0;
    }
    return r->cantidad;
}

/**
 * @brief Acceso de solo lectura al puesto `indice` (0 = lider).
 *
 * @param r Ranking consultado.
 * @param indice Posicion solicitada, desde 0.
 * @return Puntero a la entrada interna; NULL si el indice esta fuera de rango
 *         o si r es NULL.
 * @note El puntero apunta a memoria interna del Ranking: no debe liberarse ni
 *       modificarse. Toda modificacion pasa por registrar_victoria.
 */
const RankingEntrada *ranking_obtener(const Ranking *r, int indice)
{
    /* Sin ranking no hay entrada que devolver. */
    if (r == NULL) {
        return NULL;
    }

    /* Solo se permite acceder a las entradas validas. */
    if (indice < 0 || indice >= r->cantidad) {
        return NULL;
    }
    return &r->entradas[indice];
}

/**
 * @brief Busca un jugador con la misma regla de identidad del modulo.
 *
 * La comparacion ignora mayusculas y espacios de los extremos. Recorre el
 * arreglo linealmente de 0 a cantidad-1: con maximo 100 jugadores el costo
 * O(n) es despreciable y evita estructuras mas complejas.
 *
 * @param r Ranking donde buscar.
 * @param nombre Nombre a buscar.
 * @return Indice de la entrada o -1 si no existe (o si el nombre es invalido,
 *         porque un nombre invalido nunca puede estar guardado).
 * @note La comparacion usa la misma normalizacion que registrar_victoria:
 *       recorta extremos y baja mayusculas antes de comparar.
 */
int ranking_buscar(const Ranking *r, const char *nombre)
{
    char norm[RANKING_NOMBRE_MAX]; /* Nombre normalizado para comparar. */
    int i;                         /* Indice de recorrido. */

    /* Validacion defensiva de argumentos. */
    if (r == NULL || nombre == NULL) {
        return -1;
    }

    /* Un nombre invalido no puede coincidir con ninguna entrada. */
    if (!nombre_normalizado(nombre, norm, (int)sizeof(norm))) {
        return -1;
    }

    /* Busqueda lineal con comparacion insensible a mayusculas. */
    for (i = 0; i < r->cantidad; i++) {
        if (comparar_insensible(r->entradas[i].nombre, norm) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Devuelve las victorias de un jugador.
 *
 * @param r Ranking consultado.
 * @param nombre Nombre del jugador.
 * @return Victorias registradas; 0 si no figura, si el nombre es invalido o
 *         si r es NULL.
 * @note Es un atajo de consulta para la interfaz ("¿cuantas lleva Ana?").
 */
int ranking_obtener_victorias(const Ranking *r, const char *nombre)
{
    int idx; /* Indice encontrado por ranking_buscar. */

    /* Si no existe, el valor correcto es cero victorias. */
    idx = ranking_buscar(r, nombre);
    if (idx < 0) {
        return 0;
    }
    return r->entradas[idx].victorias;
}

/**
 * @brief Ordena el ranking in situ de mejor a peor puesto.
 *
 * Criterio compuesto:
 *  1. mas victorias primero;
 *  2. a igual victorias, nombre ascendente insensible a mayusculas;
 *  3. a igual victorias e igual nombre insensible, strcmp sensible, solo para
 *     que el resultado sea determinista.
 *
 * @param r Ranking que se ordena.
 * @return No devuelve valor.
 * @note Algoritmo de ordenamiento por insercion. Invariante: antes de la
 *       iteracion i, el prefijo [0..i-1] ya esta ordenado. Con n <= 100 el
 *       costo O(n^2) del peor caso es instantaneo y el codigo es corto.
 */
void ranking_ordenar(Ranking *r)
{
    int i;                  /* Posicion de la entrada que se inserta. */
    int j;                  /* Posicion del prefijo ya ordenado. */
    RankingEntrada tmp;     /* Copia temporal de la entrada i. */

    /* Sin ranking no hay nada que ordenar. */
    if (r == NULL) {
        return;
    }

    /* Insercion: victorias desc, nombre asc insensible, desempate sensible. */
    for (i = 1; i < r->cantidad; i++) {
        tmp = r->entradas[i];
        j = i - 1;

        /* Desplazar a la derecha las entradas que deben ir despues de tmp. */
        while (j >= 0) {
            int debe_mover; /* Bandera de decision. */
            int cmp;        /* Comparacion insensible de nombres. */

            debe_mover = 0;

            /* Menos victorias significa peor puesto: se mueve. */
            if (r->entradas[j].victorias < tmp.victorias) {
                debe_mover = 1;
            } else if (r->entradas[j].victorias == tmp.victorias) {
                /* Empate: orden alfabetico insensible. */
                cmp = comparar_insensible(r->entradas[j].nombre, tmp.nombre);
                if (cmp > 0) {
                    debe_mover = 1;
                } else if (cmp == 0) {
                    /* Desempate final sensible para determinismo. */
                    if (strcmp(r->entradas[j].nombre, tmp.nombre) > 0) {
                        debe_mover = 1;
                    }
                }
            }

            /* Si ya esta en el lugar correcto, terminar el desplazamiento. */
            if (!debe_mover) {
                break;
            }

            /* Mover la entrada una posicion a la derecha. */
            r->entradas[j + 1] = r->entradas[j];
            j--;
        }

        /* Insertar tmp en el hueco encontrado. */
        r->entradas[j + 1] = tmp;
    }
}

/**
 * @brief Suma una victoria al jugador indicado; si no existe, lo crea.
 *
 * Flujo:
 *  1. valida punteros y normaliza el nombre (rechaza invalidos);
 *  2. busca si ya existe: si existe, incrementa en 1 con tope en
 *     RANKING_VICTORIAS_MAX; si no existe y hay lugar, lo agrega con
 *     victorias = 1 y cantidad++;
 *  3. reordena SIEMPRE (mantiene el INVARIANTE 2).
 *
 * @param r Ranking que se modifica.
 * @param nombre Nombre del ganador.
 * @return RANKING_OK, RANKING_ERROR_ARGUMENTO,
 *         RANKING_ERROR_NOMBRE_INVALIDO o RANKING_ERROR_LLENO.
 * @note Es la unica via publica para modificar el ranking. Al crear la entrada
 *       se copia el nombre caracter por caracter y se agrega el '\0': no se
 *       usa strcpy para no depender de que el origen este bien terminado.
 */
int ranking_registrar_victoria(Ranking *r, const char *nombre)
{
    char norm[RANKING_NOMBRE_MAX]; /* Nombre normalizado. */
    int idx;                       /* Indice del jugador encontrado. */
    int i;                         /* Indice de copia del nombre. */

    /* Validacion de punteros. */
    if (r == NULL || nombre == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }

    /* Validacion y normalizacion del nombre. */
    if (!nombre_normalizado(nombre, norm, (int)sizeof(norm))) {
        return RANKING_ERROR_NOMBRE_INVALIDO;
    }

    /* Buscar si el jugador ya existe. */
    idx = ranking_buscar(r, norm);
    if (idx >= 0) {
        /* Incrementar sin pasar el tope maximo. */
        if (r->entradas[idx].victorias < RANKING_VICTORIAS_MAX) {
            r->entradas[idx].victorias++;
        }
        ranking_ordenar(r);
        return RANKING_OK;
    }

    /* Jugador nuevo: verificar que haya lugar. */
    if (r->cantidad >= RANKING_MAX_JUGADORES) {
        return RANKING_ERROR_LLENO;
    }

    /* Copiar el nombre caracter por caracter, con '\0' final. */
    for (i = 0; norm[i] != '\0'; i++) {
        r->entradas[r->cantidad].nombre[i] = norm[i];
    }
    r->entradas[r->cantidad].nombre[i] = '\0';

    /* La primera victoria del jugador nuevo es 1. */
    r->entradas[r->cantidad].victorias = 1;
    r->cantidad++;

    /* Mantener el invariante de orden. */
    ranking_ordenar(r);
    return RANKING_OK;
}

/**
 * @brief Lee el archivo binario `ruta` y reconstruye el ranking.
 *
 * Protocolo:
 *  1. Vacia el ranking (cantidad = 0): cargar SIEMPRE parte de cero.
 *  2. Abre en "rb". Si no existe -> RANKING_ERROR_ARCHIVO (no fatal: app.c lo
 *     trata como primera partida y arranca vacio).
 *  3. fread de UN struct completo. Si no lee exactamente uno: ferror()
 *     verdadero -> RANKING_ERROR_ARCHIVO; si no, archivo vacio/truncado ->
 *     RANKING_ERROR_FORMATO.
 *  4. fgetc extra: cualquier byte despues del struct indica otro tamano o
 *     version -> RANKING_ERROR_FORMATO.
 *  5. Valida `cantidad` en 0..RANKING_MAX_JUGADORES.
 *  6. Por cada entrada declarada: valida nombre y victorias, descarta las
 *     invalidas, y fusiona duplicados insensibles a mayusculas.
 *  7. Ordena y devuelve OK, o FORMATO si hubo al menos una entrada descartada
 *     (las validas quedan cargadas).
 *
 * @param r Ranking que se reconstruye; se vacia antes de cargar.
 * @param ruta Ruta del archivo binario.
 * @return RANKING_OK, RANKING_ERROR_ARCHIVO, RANKING_ERROR_FORMATO o
 *         RANKING_ERROR_ARGUMENTO.
 * @note No se valida la basura de los slots libres. El layout se garantiza por
 *       tamano exacto + cantidad valida + asserts de ranking.c.
 */
int ranking_cargar(Ranking *r, const char *ruta)
{
    FILE *f;           /* Archivo binario de entrada. */
    Ranking disco;     /* Copia cruda leida del disco. */
    size_t leidos;     /* Cantidad de structs leidos. */
    int sobrante;      /* Byte extra despues del struct. */
    int hubo_formato;  /* Bandera: al menos una entrada invalida. */
    int i;             /* Indice de entrada leida. */

    /* Validacion defensiva de argumentos. */
    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }

    /* Cargar siempre parte de un ranking vacio. */
    r->cantidad = 0;

    /* Apertura binaria: imprescindible en Windows. */
    f = fopen(ruta, "rb");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }

    /* Leer exactamente un struct Ranking. */
    leidos = fread(&disco, sizeof(disco), 1, f);
    if (leidos != 1) {
        int fallo_io; /* Distingue error de E/S de archivo invalido. */

        fallo_io = ferror(f);
        fclose(f);
        return fallo_io ? RANKING_ERROR_ARCHIVO : RANKING_ERROR_FORMATO;
    }

    /* Cualquier byte extra significa otro formato/version: se rechaza. */
    sobrante = fgetc(f);
    fclose(f);
    if (sobrante != EOF) {
        return RANKING_ERROR_FORMATO;
    }

    /* La cantidad declarada debe estar dentro del arreglo. */
    if (disco.cantidad < 0 || disco.cantidad > RANKING_MAX_JUGADORES) {
        return RANKING_ERROR_FORMATO;
    }

    hubo_formato = 0;

    /* Procesar solo las entradas declaradas como validas. */
    for (i = 0; i < disco.cantidad; i++) {
        char nombre[RANKING_NOMBRE_MAX]; /* Nombre normalizado. */
        int idx;                         /* Indice de duplicado. */
        int k;                           /* Indice de busqueda. */

        /* Validar la entrada; si falla, descartarla y seguir. */
        if (!entrada_valida(&disco.entradas[i], nombre)) {
            hubo_formato = 1;
            continue;
        }

        /* Buscar si ya se cargo el mismo jugador (caso archivo manipulado). */
        idx = -1;
        for (k = 0; k < r->cantidad; k++) {
            if (comparar_insensible(r->entradas[k].nombre, nombre) == 0) {
                idx = k;
                break;
            }
        }

        if (idx >= 0) {
            /* Fusionar duplicados sumando, con tope para no desbordar. */
            long suma; /* Suma intermedia en long para evitar overflow. */

            suma = (long)r->entradas[idx].victorias
                 + (long)disco.entradas[i].victorias;
            if (suma > RANKING_VICTORIAS_MAX) {
                suma = RANKING_VICTORIAS_MAX;
            }
            r->entradas[idx].victorias = (int)suma;
        } else if (r->cantidad < RANKING_MAX_JUGADORES) {
            /* Copia segura: `nombre` ya esta normalizado y terminado. */
            strcpy(r->entradas[r->cantidad].nombre, nombre);
            r->entradas[r->cantidad].victorias = disco.entradas[i].victorias;
            r->cantidad++;
        } else {
            /* Mas entradas validas que el maximo: se descarta. */
            hubo_formato = 1;
        }
    }

    /* Ordenar el resultado y reportar si hubo descartes. */
    ranking_ordenar(r);
    if (hubo_formato) {
        return RANKING_ERROR_FORMATO;
    }
    return RANKING_OK;
}

/**
 * @brief Escribe TODO el ranking en `ruta` en binario.
 *
 * Se trabaja sobre una copia local `copia`, se pone TODO a cero con memset, se
 * copia `cantidad` y, por cada entrada usada, se copia el nombre byte a byte
 * con tope de 31 mas el '\0'. Asi todos los bytes no usados quedan en cero y
 * el archivo es deterministico: mismos datos -> mismos bytes.
 *
 * @param r Ranking que se escribe (no se modifica).
 * @param ruta Ruta del archivo binario; se sobrescribe.
 * @return RANKING_OK, RANKING_ERROR_ARGUMENTO o RANKING_ERROR_ARCHIVO.
 * @note Se abre en "wb" porque el modo texto corromperia enteros con byte
 *       0x0A. Se controla fwrite y tambien fclose, porque el error real puede
 *       aparecer al volcar el buffer al disco.
 */
int ranking_guardar(const Ranking *r, const char *ruta)
{
    FILE *f;       /* Archivo binario de salida. */
    Ranking copia; /* Copia normalizada que se escribe. */
    int i;         /* Indice de entrada. */

    /* Validacion defensiva de argumentos. */
    if (r == NULL || ruta == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }

    /* Normalizar toda la copia a cero antes de volcar datos validos. */
    memset(&copia, 0, sizeof(copia));
    copia.cantidad = r->cantidad;

    /* Acotar la cantidad por si el struct en memoria estuviera corrupto. */
    if (copia.cantidad < 0) {
        copia.cantidad = 0;
    }
    if (copia.cantidad > RANKING_MAX_JUGADORES) {
        copia.cantidad = RANKING_MAX_JUGADORES;
    }

    /* Copiar solo las entradas usadas, con nombres acotados. */
    for (i = 0; i < copia.cantidad; i++) {
        int j; /* Indice dentro del nombre. */

        for (j = 0; j < RANKING_NOMBRE_MAX - 1
                    && r->entradas[i].nombre[j] != '\0'; j++) {
            copia.entradas[i].nombre[j] = r->entradas[i].nombre[j];
        }
        copia.entradas[i].victorias = r->entradas[i].victorias;
    }

    /* Apertura binaria: imprescindible en Windows. */
    f = fopen(ruta, "wb");
    if (f == NULL) {
        return RANKING_ERROR_ARCHIVO;
    }

    /* Escribir el struct completo y controlar el resultado. */
    if (fwrite(&copia, sizeof(copia), 1, f) != 1) {
        fclose(f);
        return RANKING_ERROR_ARCHIVO;
    }

    /* fclose tambien puede fallar al vaciar el buffer. */
    if (fclose(f) != 0) {
        return RANKING_ERROR_ARCHIVO;
    }
    return RANKING_OK;
}

/**
 * @brief Traduce un codigo de retorno a texto en espanol.
 *
 * @param codigo Codigo devuelto por otra funcion del modulo.
 * @return Puntero a un texto constante; nunca NULL.
 * @note La logica devuelve numeros y la interfaz los convierte con esta
 *       funcion: asi los mensajes viven en un solo lugar.
 */
const char *ranking_mensaje_error(int codigo)
{
    /* Un case por cada codigo publico; default cubre valores desconocidos. */
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

/**
 * @brief Imprime la tabla del ranking en el FILE indicado.
 *
 * Formato: encabezado con cantidad ("1 jugador" / "N jugadores") y una fila
 * por puesto, por ejemplo " 1. Ana                            2". El %-31s alinea
 * los nombres en columna. Si esta vacio avisa en vez de no imprimir nada.
 *
 * @param salida Flujo de salida ya abierto.
 * @param r Ranking que se imprime.
 * @return RANKING_OK, o RANKING_ERROR_ARGUMENTO si salida o r son NULL.
 * @note Existe esta variante con FILE (y no solo printf) para poder probar la
 *       salida redirigiendola y para que la futura GUI use las funciones de
 *       consulta en vez de esta impresion.
 */
int ranking_mostrar_en(FILE *salida, const Ranking *r)
{
    int i; /* Indice de puesto. */

    /* Validacion de punteros. */
    if (salida == NULL || r == NULL) {
        return RANKING_ERROR_ARGUMENTO;
    }

    /* Ranking vacio: mensaje explicito para que la pantalla no quede muda. */
    if (r->cantidad <= 0) {
        fprintf(salida, "Ranking vacio. Todavia no hay victorias registradas.\n");
        return RANKING_OK;
    }

    /* Encabezado con singular/plural segun la cantidad. */
    fprintf(salida, "=== RANKING (%d jugador%s) ===\n", r->cantidad,
            r->cantidad == 1 ? "" : "es");

    /* Una fila por puesto, con nombre alineado a 31 columnas. */
    for (i = 0; i < r->cantidad; i++) {
        fprintf(salida, "%2d. %-31s %d\n",
                i + 1, r->entradas[i].nombre, r->entradas[i].victorias);
    }
    return RANKING_OK;
}

/**
 * @brief Imprime el ranking en stdout.
 *
 * @param r Ranking que se imprime.
 * @return No devuelve valor.
 * @note Es lo que llama app.c para la pantalla final de consola exigida por la
 *       consigna. No hace nada mas: delega todo en ranking_mostrar_en.
 */
void ranking_mostrar(const Ranking *r)
{
    /* Un puntero nulo no tiene nada que mostrar. */
    if (r == NULL) {
        return;
    }
    ranking_mostrar_en(stdout, r);
}
