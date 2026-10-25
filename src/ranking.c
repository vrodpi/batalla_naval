/*
 * ranking.c - Ranking de victorias por jugador (logica + persistencia).
 *
 * ---------------------------------------------------------------------------
 * QUE HACE ESTE MODULO
 * ---------------------------------------------------------------------------
 * Mantiene cuantas partidas gano cada jugador y lo guarda en
 * "data/ranking.dat" como BINARIO: una copia cruda del struct Ranking
 * (opcion A, fwrite/fread del struct completo). Es el modulo del
 * integrante 4 (ranking, datos y compilacion).
 *
 * Estandar del proyecto: C99 (-std=c99 -pedantic -Wall -Wextra). Este
 * archivo no necesita caracteristicas de C99, asi que sigue siendo valido
 * tambien en C89; se compila junto al resto en el modo C99 del Makefile.
 *
 * Separacion logica / interfaz (Anexo A):
 *   - ESTE archivo decide reglas: que nombre vale, quien es el mismo
 *     jugador, en que orden se lista y como se lee/escribe el archivo.
 *   - La interfaz (ui_cli.c, futura ui_gui.c) SOLO pide el nombre por
 *     teclado y muestra mensajes. Jamas suma victorias, ordena ni escribe
 *     el archivo por su cuenta.
 *
 * ---------------------------------------------------------------------------
 * QUE CAMBIO RESPECTO DE LA VERSION TEXTO
 * ---------------------------------------------------------------------------
 * Antes: texto "nombre;victorias\n", legible y trivial de parsear.
 * Ahora: 3604 bytes binarios = struct Ranking tal cual esta en memoria.
 * Motivo: indicacion de la catedra de teoria ("la informacion importante
 * o sensible debe ir en archivos binarios"). Decision del grupo.
 *
 * Que se gana: el archivo no se abre/edita por accidente con un editor
 * de texto y no hay que parsear numeros ni lineas.
 * Que NO se gana: seguridad. Con el codigo fuente a mano, reescribir el
 * binario es trivial. Ademas no hay magic number, version ni checksum:
 * el archivo no tiene forma de demostrar que fue generado por este
 * programa. Ver "modelo de amenaza" en docs/contrato-ranking.md.
 *
 * ---------------------------------------------------------------------------
 * LAYOUT EN DISCO (lo que realmente se escribe)
 * ---------------------------------------------------------------------------
 * sizeof(Ranking) = 3604 bytes en x86_64 con int de 4 bytes:
 *
 *   offset    0: entradas[0..99], 100 x 36 bytes        = 3600
 *                 +0   char nombre[32]  ASCII + '\0'
 *                 +32  int  victorias   4 bytes little-endian
 *   offset 3600: cantidad               int, 4 bytes
 *
 * Los dos typedefs de "assert" de abajo verifican EN TIEMPO DE
 * COMPILACION que no exista relleno (padding): si un compilador agrega
 * bytes entre nombre y victorias, o al final del struct, la compilacion
 * falla con "size of array is negative" en lugar de generar un archivo
 * incompatible en silencio. Es la unica proteccion posible sin cambiar
 * el formato (un magic/version/checksum seria la opcion B).
 *
 * ---------------------------------------------------------------------------
 * DETALLES CRITICOS DE LA IMPLEMENTACION BINARIA
 * ---------------------------------------------------------------------------
 * 1. MODO BINARIO: fopen("wb")/fopen("rb"), nunca "w"/"r". En modo
 *    texto, Windows convierte cada 0x0A en 0x0D 0x0A al escribir y al
 *    reves al leer: un int con valor 10 (0x0A 0x00 0x00 0x00) se
 *    corromperia. Con "wb"/"rb" los bytes van y vuelven intactos en
 *    Windows y Linux.
 * 2. ESCRITURA DETERMINISTA: no se hace fwrite(r) directo. Se copia el
 *    struct a una variable local, se pone TODO a cero con memset y se
 *    copian solo los datos validos. Si se escribiera el struct original,
 *    los slots libres (entradas[cantidad..99]) y la cola de cada nombre
 *    contendrian basura de la pila: el archivo cambiaria en cada guardado
 *    (imposible comparar/depurar) y filtraria memoria del proceso. El
 *    layout en disco es identico; solo se garantiza que los bytes no
 *    usados sean cero.
 * 3. LECTURA VALIDADA: un archivo binario corrupto no se puede "parsear
 *    con tolerancia" como el texto, asi que despues de fread se verifica
 *    tamano exacto, cantidad en rango, '\0' dentro de cada nombre y
 *    victorias dentro de 0..RANKING_VICTORIAS_MAX. Las entradas validas
 *    se conservan y las invalidas se descartan (carga parcial).
 * 4. TAMANO EXACTO: si sobran bytes (archivo de otra version) o faltan
 *    (truncado), se devuelve RANKING_ERROR_FORMATO. No se intenta
 *    adivinar el formato.
 *
 * ---------------------------------------------------------------------------
 * INVARIANTES EN MEMORIA
 * ---------------------------------------------------------------------------
 * INVARIANTE 1: solo las primeras `cantidad` entradas son validas.
 * INVARIANTE 2: tras registrar/cargar, el arreglo queda ordenado.
 * INVARIANTE 3: `nombre` de cada entrada usada tiene '\0' dentro de sus
 *               32 bytes (lo garantizan registrar y cargar).
 *
 * ---------------------------------------------------------------------------
 * MAPA DE FUNCIONES (orden de lectura sugerido)
 * ---------------------------------------------------------------------------
 * Ayudas internas (static):
 *   recortar_extremos, comparar_insensible, nombre_normalizado,
 *   entrada_valida
 * API publica (ranking.h):
 *   ranking_inicializar, ranking_cantidad, ranking_obtener,
 *   ranking_buscar, ranking_obtener_victorias, ranking_ordenar,
 *   ranking_registrar_victoria, ranking_cargar, ranking_guardar,
 *   ranking_mensaje_error, ranking_mostrar_en, ranking_mostrar
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ranking.h"

/*
 * Asserts de layout (ver explicacion en el encabezado).
 * - RankingEntrada debe medir exactamente 32 + sizeof(int) (sin relleno).
 * - Ranking debe medir 100 * sizeof(RankingEntrada) + sizeof(int).
 * Si alguna condicion es falsa, el tamano del arreglo es -1 y el
 * compilador se detiene con un error claro.
 */
typedef char ranking_assert_entrada_sin_relleno[
    (sizeof(RankingEntrada) == (RANKING_NOMBRE_MAX + sizeof(int))) ? 1 : -1];

typedef char ranking_assert_ranking_sin_relleno[
    (sizeof(Ranking) == (RANKING_MAX_JUGADORES * sizeof(RankingEntrada)
                         + sizeof(int))) ? 1 : -1];

/*
 * recortar_extremos: copia `origen` en `destino` sin los espacios en
 * blanco de los extremos ("  Ana \t" -> "Ana").
 *
 * Algoritmo (dos punteros sobre el mismo string):
 *  1. `ini` avanza mientras haya espacios (isspace: ' ', '\t', etc.).
 *  2. Si solo habia espacios, el resultado es "" y se termina.
 *  3. `fin` retrocede desde el '\0' mientras haya espacios.
 *  4. Se copian los [ini, fin) caracteres, truncando a tam_destino-1
 *     si hiciera falta, y siempre se agrega el '\0' final.
 *
 * Detalle tecnico: isspace() exige un valor representable como
 * `unsigned char` (o EOF). Pasar un `char` negativo (letras con tilde
 * en Latin-1/UTF-8) seria comportamiento indefinido; por eso cada
 * caracter se convierte con (unsigned char) antes de llamar.
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
 * comparar_insensible: compara dos strings ignorando mayusculas,
 * como strcasecmp() pero portable.
 *
 * Por que no usar strcasecmp/stricmp: ninguna es C estandar (ni C89 ni
 * C99). strcasecmp es POSIX (Linux) y stricmp es de Windows: usarlas
 * romperia la portabilidad que exige la consigna. Esta version solo usa
 * tolower() de <ctype.h>, que si es estandar, y baja cada caracter a
 * minuscula antes de comparar.
 *
 * Devuelve 0 si son iguales, negativo si a<b, positivo si a>b
 * (misma convencion que strcmp). Los NULL se ordenan antes que
 * cualquier string, para no romper nunca por un puntero nulo.
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
 * nombre_normalizado: valida un nombre de jugador y produce su forma
 * canonica (recortada, con mayusculas originales).
 *
 * Reglas que aplica, en orden:
 *  1. Recorta espacios de los extremos.
 *  2. El largo real (sin contar esos espacios) debe ser 1..31.
 *  3. No puede contener '\n', '\r' ni ningun caracter de control
 *     (< 32). El ';' SI se permite: en binario no es un separador (a
 *     diferencia del formato texto anterior).
 *
 * Por que DOS pasadas de medicion: recortar_extremos() trunca en
 * silencio si el texto no entra en el buffer. Si midieramos solo el
 * resultado recortado, un nombre de 200 caracteres pasaria como valido
 * de 31. Por eso la segunda pasada mide el largo REAL sobre el string
 * original (con punteros ini/fin, sin copiar nada) y rechaza todo lo
 * que sea <= 0 o >= RANKING_NOMBRE_MAX antes de aceptar.
 *
 * Devuelve 1 + `salida` con el nombre listo para guardar, o 0 si el
 * nombre es invalido (en ese caso `salida` no debe usarse).
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

/*
 * entrada_valida: verifica que una entrada leida del archivo binario sea
 * usable y devuelve su nombre normalizado en `nombre`.
 *
 * Por que existe: al leer bytes crudos, un archivo corrupto o de otra
 * version puede contener cualquier cosa. Antes de llamar a
 * nombre_normalizado() hay que garantizar que exista un '\0' DENTRO de
 * los 32 bytes; si no, strlen() leería fuera del struct (comportamiento
 * indefinido, posible caida). Se recorre el arreglo con un tope fijo y
 * recien despues se valida el nombre con las reglas normales.
 * Tambien se rechazan victorias negativas o mayores que el tope.
 *
 * Devuelve 1 si la entrada es valida, 0 si debe descartarse.
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

/*
 * ranking_inicializar: deja el ranking vacio (cantidad = 0).
 * Debe llamarse antes de cualquier otro uso: es lo que garantiza el
 * INVARIANTE 1 (solo las primeras `cantidad` entradas son validas).
 */
void ranking_inicializar(Ranking *r)
{
    if (r == NULL) {
        return;
    }
    r->cantidad = 0;
}

/*
 * ranking_cantidad: cuantos jugadores distintos hay (0 si r es NULL).
 * Se usa para recorrer con ranking_obtener(0..cantidad-1) y para que
 * la futura GUI dibuje tantas filas como haga falta.
 */
int ranking_cantidad(const Ranking *r)
{
    if (r == NULL) {
        return 0;
    }
    return r->cantidad;
}

/*
 * ranking_obtener: acceso de SOLO LECTURA a la entrada del puesto
 * `indice` (0 = lider). Devuelve NULL si el indice esta fuera de
 * rango, para que la interfaz no lea basura del arreglo. El puntero
 * apunta a memoria interna del Ranking: no debe liberarse ni
 * modificarse (toda modificacion pasa por registrar_victoria).
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
 * ranking_buscar: busca un nombre con la misma regla de identidad que
 * el resto del modulo (recorte + insensible a mayusculas). Recorre el
 * arreglo linealmente de 0 a cantidad-1: con maximo 100 jugadores el
 * costo O(n) es despreciable y evita estructuras mas complejas.
 * Devuelve el indice o -1 si no existe (o si el nombre es invalido,
 * porque un nombre invalido nunca puede estar guardado).
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

/*
 * ranking_obtener_victorias: atajo de consulta para la interfaz
 * ("¿cuantas lleva Ana?"). Devuelve 0 si el jugador no figura, que es
 * el valor correcto para alguien que aun no gano nada.
 */
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
 * ranking_ordenar: ordena el arreglo in situ de mejor a peor puesto.
 *
 * Criterio compuesto (primero el que desempata, despues el otro):
 *  1. mas victorias primero;
 *  2. a igual victorias, nombre ascendente insensible a mayusculas;
 *  3. a igual victorias E igual nombre insensible, strcmp sensible,
 *     solo para que el resultado sea determinista (mismo input, mismo
 *     output) en cualquier compilador.
 *
 * Algoritmo: ordenamiento por INSERCION. Invariante del bucle: antes
 * de la iteracion i, el prefijo [0..i-1] ya esta ordenado. Se toma la
 * entrada i como `tmp` y se desplazan a la derecha todos los
 * anteriores que "deben ir despues" (peor criterio), hasta encontrar
 * su hueco. Con n <= 100 el costo O(n^2) del peor caso (~10.000
 * comparaciones) es instantaneo, y el codigo es corto y facil de
 * seguir en la defensa. La copia `tmp = r->entradas[i]` es asignacion
 * de structs, valida en C99 (y ya lo era en C89).
 */
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

/*
 * ranking_registrar_victoria: suma UNA victoria al jugador `nombre`.
 * Es la unica via para modificar el ranking (la interfaz no toca el
 * arreglo directamente). Flujo:
 *   1. valida punteros y normaliza el nombre (rechaza invalidos);
 *   2. busca si ya existe:
 *      - existe: incrementa en 1, con tope en RANKING_VICTORIAS_MAX
 *        (en vez de desbordar el int, se queda en el maximo);
 *      - no existe: si hay lugar lo agrega con victorias = 1 y
 *        cantidad++, si no hay lugar devuelve RANKING_ERROR_LLENO;
 *   3. reordena SIEMPRE (mantiene el INVARIANTE 2).
 *
 * Al crear la entrada se copia el nombre caracter por caracter y se
 * agrega el '\0': no se usa strcpy para no depender de que el origen
 * este bien terminado y para que quede claro el limite del buffer.
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

/*
 * ranking_cargar: lee el archivo binario `ruta` y reconstruye el ranking.
 *
 * Protocolo:
 *  1. Vacia el ranking (cantidad = 0): cargar SIEMPRE parte de cero.
 *  2. Abre en "rb". Si no existe -> RANKING_ERROR_ARCHIVO (no fatal:
 *     app.c lo trata como primera partida y arranca vacio).
 *  3. fread de UN struct completo. Si no lee exactamente uno:
 *     - ferror() verdadero -> problema de E/S -> RANKING_ERROR_ARCHIVO;
 *     - si no, el archivo esta vacio/truncado -> RANKING_ERROR_FORMATO.
 *  4. fgetc extra: si hay CUALQUIER byte despues del struct, el archivo
 *     es de otro tamano/version -> RANKING_ERROR_FORMATO. No se intenta
 *     interpretar un formato desconocido.
 *  5. Valida `cantidad` en 0..RANKING_MAX_JUGADORES. Fuera de rango ->
 *     FORMATO y ranking vacio (no se recorre memoria invalida).
 *  6. Por cada entrada declarada:
 *     a. Verifica que haya '\0' dentro de nombre[32] y que el nombre
 *        pase las reglas (entrada_valida). Si no, la descarta y marca
 *        error de formato, pero SIGUE con las demas.
 *     b. Verifica victorias en 0..RANKING_VICTORIAS_MAX; si no, descarta.
 *     c. Fusiona duplicados insensibles a mayusculas sumando con tope
 *        (solo puede pasar con archivos manipulados: guardar nunca
 *        escribe dos veces al mismo jugador).
 *  7. Ordena y devuelve OK, o FORMATO si hubo al menos una entrada
 *     descartada (las validas quedan cargadas: carga parcial).
 *
 * Nota: no se valida la basura de los slots libres. El layout se
 * garantiza por tamano exacto + cantidad valida + asserts de ranking.c.
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
    /* Cualquier byte extra significa otro formato/version: se rechaza. */
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
            /* Fusiona duplicados sumando, con tope para no desbordar. */
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

/*
 * ranking_guardar: escribe TODO el ranking en `ruta` en binario.
 *
 * Por que no es un simple fwrite(r, sizeof *r, 1, f):
 *  - `r` tiene 100 slots y solo `cantidad` son validos; el resto
 *    contiene basura de la pila. Volcarla haria que el archivo cambie
 *    en cada guardado y filtraria memoria del proceso.
 *  - La cola de cada nombre (bytes despues del '\0' dentro de los 32)
 *    tambien puede tener restos.
 * Solucion: se trabaja sobre una copia local `copia`, se pone TODO a
 * cero con memset, se copia `cantidad` y, por cada entrada usada, se
 * copia el nombre byte a byte con tope de 31 mas el '\0' (nunca
 * strcpy: si el origen no tuviera '\0', strcpy se desbordaria). Asi
 * todos los bytes no usados quedan en cero y el archivo es
 * deterministico: mismos datos -> mismos bytes.
 *
 * Se abre en "wb" (ver encabezado: el modo texto corromperia enteros con
 * byte 0x0A). Se controla fwrite y tambien fclose, porque la escritura
 * es con buffer y el error real puede aparecer al volcar al disco.
 * No modifica el ranking en memoria (copia local + const).
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

/*
 * ranking_mensaje_error: traduce un codigo de retorno a texto en
 * espanol para mostrar en pantalla. La logica devuelve numeros y la
 * interfaz los convierte con esta funcion: asi los mensajes viven en
 * un solo lugar y la futura GUI puede reutilizarlos o reemplazarlos.
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
            return "Ranking lleno";
        case RANKING_ERROR_ARCHIVO:
            return "No se pudo leer o guardar el archivo";
        case RANKING_ERROR_FORMATO:
            return "Archivo binario invalido o corrupto";
        default:
            return "Error desconocido";
    }
}

/*
 * ranking_mostrar_en: imprime la tabla del ranking en el FILE dado.
 * Formato: encabezado con cantidad ("1 jugador" / "N jugadores") y
 * una fila por puesto, " 1. Ana                            2".
 * El %-31s alinea los nombres en columna. Si esta vacio avisa en vez
 * de no imprimir nada (asi la pantalla final nunca queda en blanco).
 * Existe esta variante con FILE (y no solo printf) para poder probar
 * la salida redirigiendola y para que la futura GUI use las funciones
 * de consulta en vez de esta impresion.
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

/*
 * ranking_mostrar: atajo que imprime en pantalla (stdout). Es lo que
 * llama app.c para la pantalla final de consola exigida por la
 * consigna. No hace nada mas: delega todo en ranking_mostrar_en.
 */
void ranking_mostrar(const Ranking *r)
{
    if (r == NULL) {
        return;
    }
    ranking_mostrar_en(stdout, r);
}
