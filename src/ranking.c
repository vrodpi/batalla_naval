/*
<<<<<<< HEAD
 * ranking.c - Ranking de victorias por jugador (logica + persistencia).
 *
 * ---------------------------------------------------------------------------
 * QUE ES ESTE MODULO
 * ---------------------------------------------------------------------------
 * Guarda cuantas partidas gano cada jugador y lo persiste en el archivo
 * "data/ranking.dat". Es el modulo del integrante 4 (ranking, datos y
 * compilacion). Sus unicas dependencias son <stdio.h>, <string.h> y
 * <ctype.h>: no usa nada especifico de Windows ni de Linux, por eso el
 * mismo archivo compila en ambos sistemas sin cambios.
 *
 * Separacion logica / interfaz (Anexo A de la consigna):
 *   - ESTE archivo decide reglas: que nombre vale, quien es el mismo
 *     jugador, en que orden se lista y como se lee/escribe el archivo.
 *   - La interfaz (ui_cli.c, futura ui_gui.c) SOLO pide el nombre por
 *     teclado y muestra mensajes. Jamas suma victorias ni ordena por su
 *     cuenta: para eso llama a las funciones publicas de ranking.h.
 *
 * ---------------------------------------------------------------------------
 * DATOS: COMO SE GUARDA EL RANKING EN MEMORIA
 * ---------------------------------------------------------------------------
 *   typedef struct { char nombre[32]; int victorias; } RankingEntrada;
 *   typedef struct { RankingEntrada entradas[100]; int cantidad; } Ranking;
 *
 * - El Ranking vive en la pila de quien lo usa (normalmente una variable
 *   local de app.c). No se usa malloc/free: con maximo 100 jugadores el
 *   arreglo fijo es simple, rapido y evita fugas de memoria.
 * - INVARIANTE 1: solo las primeras `cantidad` posiciones tienen datos
 *   validos. El resto del arreglo es basura y nunca se lee.
 * - INVARIANTE 2: despues de cada operacion que modifica datos
 *   (registrar_victoria, cargar), el arreglo queda ORDENADO de mejor a
 *   peor puesto. Asi la posicion 0 siempre es el lider y mostrar() solo
 *   recorre e imprime, sin ordenar nada.
 * - `nombre` guarda la grafia ORIGINAL recortada ("Ana"), pero las
 *   comparaciones ignoran mayusculas ("Ana" == "ANA" == " ana ").
 *
 * ---------------------------------------------------------------------------
 * ARCHIVO: FORMATO PORTABLE DE ranking.dat
 * ---------------------------------------------------------------------------
 * Texto plano, una linea por jugador:  nombre;victorias
 * Ejemplo de archivo valido:
 *     Ana;3
 *     Luis;1
 * No se usa fwrite() del struct porque el relleno (padding) y el orden de
 * bytes cambian entre compiladores: un .dat binario escrito en Windows
 * podria leerse mal en Linux. El texto "Ana;3\n" se lee igual en todos
 * lados. El archivo se abre en modo texto ("r"/"w") para que Windows
 * traduzca solo los saltos de linea \n <-> \r\n.
 *
 * ---------------------------------------------------------------------------
 * DECISIONES DE REGLAS (cierran los "pendientes" del documento general)
 * ---------------------------------------------------------------------------
 * 1. Orden: victorias descendente; empate -> nombre ascendente insensible
 *    a mayusculas; si aun empatan (ej. "ana" vs "Ana" no puede pasar por
 *    ser el mismo jugador, pero por determinismo) -> strcmp sensible.
 * 2. Mayusculas: "Luis", "LUIS" y "  luis " son EL MISMO jugador. Se
 *    conserva la primera grafia guardada.
 * 3. Nombre valido: 1 a 31 caracteres tras recortar espacios, sin ';'
 *    (es el separador), sin saltos de linea ni caracteres de control.
 * 4. Carga tolerante: las lineas en blanco se ignoran y las malformadas
 *    se saltean avisando con RANKING_ERROR_FORMATO, pero las validas
 *    igual se cargan. El juego nunca pierde todo el ranking por una
 *    linea rota o editada a mano.
 * 5. Duplicados dentro del archivo (ej. "Ana;2" y "ana;3") se FUSIONAN
 *    sumando (5), con tope en RANKING_VICTORIAS_MAX.
 *
 * ---------------------------------------------------------------------------
 * MAPA DE FUNCIONES (orden de lectura sugerido)
 * ---------------------------------------------------------------------------
 * Ayudas internas (static, no se ven desde otros archivos):
 *   recortar_extremos, comparar_insensible, nombre_normalizado,
 *   parsear_victorias, quitar_salto
 * API publica (declarada en ranking.h, la que usa app.c / ui):
 *   ranking_inicializar, ranking_cantidad, ranking_obtener,
 *   ranking_buscar, ranking_obtener_victorias, ranking_ordenar,
 *   ranking_registrar_victoria, ranking_cargar, ranking_guardar,
 *   ranking_mensaje_error, ranking_mostrar_en, ranking_mostrar
=======
 * ranking.c - Implementacion del ranking de victorias.
 *
 * ANSI C (C89): solo <stdio.h>, <string.h>, <ctype.h>.
 * Sin fwrite de structs: el archivo es texto "nombre;victorias".
 * Sin dependencias de sistema operativo.
>>>>>>> main
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ranking.h"

<<<<<<< HEAD
/*
 * RANKING_LINEA_MAX: tamano del buffer con que se lee cada linea del
 * archivo. 256 sobra para "31 de nombre + ';' + 7 de numero + salto".
 * Si una linea supera el buffer se la trata como linea corrupta
 * (ver ranking_cargar) en lugar de truncarla en silencio.
 */
#define RANKING_LINEA_MAX 256

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
=======
#define RANKING_LINEA_MAX 256

/* Recorta espacios en extremos. Siempre termina en '\0'. */
>>>>>>> main
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

<<<<<<< HEAD
/*
 * comparar_insensible: compara dos strings ignorando mayusculas,
 * como strcasecmp() pero portable.
 *
 * Por que no usar strcasecmp/stricmp: ninguna es ANSI C. strcasecmp
 * es POSIX (Linux) y stricmp es de Windows: usarlas romperia la
 * portabilidad que exige la consigna. Esta version solo usa tolower()
 * de <ctype.h>, que si es estandar, y baja cada caracter a minuscula
 * antes de comparar.
 *
 * Devuelve 0 si son iguales, negativo si a<b, positivo si a>b
 * (misma convencion que strcmp). Los NULL se ordenan antes que
 * cualquier string, para no romper nunca por un puntero nulo.
 */
=======
/* Comparacion insensible a mayusculas. Estilo strcasecmp portable. */
>>>>>>> main
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
<<<<<<< HEAD
 * nombre_normalizado: valida un nombre de jugador y produce su forma
 * canonica (recortada, con mayusculas originales).
 *
 * Reglas que aplica, en orden:
 *  1. Recorta espacios de los extremos.
 *  2. El largo real (sin contar esos espacios) debe ser 1..31.
 *  3. No puede contener ';' (es el separador del archivo), '\n',
 *     '\r' ni ningun caracter de control (< 32).
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
=======
 * Normaliza y valida un nombre.
 * Salida: nombre recortado (conserva mayusculas originales).
 * Devuelve 1 si valido, 0 si no.
>>>>>>> main
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

<<<<<<< HEAD
/*
 * parsear_victorias: convierte el texto a la derecha del ';' en un
 * entero 0..RANKING_VICTORIAS_MAX. Gramatica aceptada:
 *     [espacios] ['+'] digitos [espacios]   (y nada mas)
 * Ejemplos: "3" ok, "  12  " ok, "+7" ok; "x", "", "-1", "3;" mal.
 *
 * Por que no usar atoi(): atoi() no detecta errores (devuelve 0 tanto
 * para "0" como para "basura") y su comportamiento con overflow es
 * indefinido. Por que no usar sscanf("%d"): aceptaria "12abc" como 12
 * y tampoco controla overflow de forma portable.
 *
 * Algoritmo: se saltean espacios, se acepta un '+' opcional, se
 * rechaza '-' de entrada, y se acumula digito por digito en un `long`.
 * Despues de cada digito se verifica que no se supere el maximo: asi
 * el overflow se detecta ANTES de que ocurra, sin depender del tamano
 * de int de cada plataforma. Al final solo puede quedar espacio y el
 * '\0'; cualquier otro resto invalida el texto. Se exige al menos un
 * digito para rechazar strings vacios o de solo espacios.
 */
=======
/* Convierte texto a victorias 0..RANKING_VICTORIAS_MAX. 1 si ok. */
>>>>>>> main
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

<<<<<<< HEAD
/*
 * quitar_salto: elimina los '\n' y '\r' del final de una linea leida
 * con fgets(). Se quitan AMBOS porque un archivo escrito en Windows
 * ("Ana;3\r\n") puede leerse en Linux, donde el modo texto no traduce
 * nada y el '\r' quedaria pegado al numero ("3\r" dejaria de parsear).
 * Quitar en bucle cubre lineas con mas de un salto al final.
 */
=======
/* Quita '\n' y '\r' finales de una linea leida con fgets. */
>>>>>>> main
static void quitar_salto(char *linea)
{
    int largo;
    largo = (int)strlen(linea);
    while (largo > 0 && (linea[largo - 1] == '\n' || linea[largo - 1] == '\r')) {
        linea[largo - 1] = '\0';
        largo--;
    }
}

<<<<<<< HEAD
/*
 * ranking_inicializar: deja el ranking vacio (cantidad = 0).
 * Debe llamarse antes de cualquier otro uso: es lo que garantiza el
 * INVARIANTE 1 (solo las primeras `cantidad` entradas son validas).
 */
=======
>>>>>>> main
void ranking_inicializar(Ranking *r)
{
    if (r == NULL) {
        return;
    }
    r->cantidad = 0;
}

<<<<<<< HEAD
/*
 * ranking_cantidad: cuantos jugadores distintos hay (0 si r es NULL).
 * Se usa para recorrer con ranking_obtener(0..cantidad-1) y para que
 * la futura GUI dibuje tantas filas como haga falta.
 */
=======
>>>>>>> main
int ranking_cantidad(const Ranking *r)
{
    if (r == NULL) {
        return 0;
    }
    return r->cantidad;
}

<<<<<<< HEAD
/*
 * ranking_obtener: acceso de SOLO LECTURA a la entrada del puesto
 * `indice` (0 = lider). Devuelve NULL si el indice esta fuera de
 * rango, para que la interfaz no lea basura del arreglo. El puntero
 * apunta a memoria interna del Ranking: no debe liberarse ni
 * modificarse (toda modificacion pasa por registrar_victoria).
 */
=======
>>>>>>> main
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

<<<<<<< HEAD
/*
 * ranking_buscar: busca un nombre con la misma regla de identidad que
 * el resto del modulo (recorte + insensible a mayusculas). Recorre el
 * arreglo linealmente de 0 a cantidad-1: con maximo 100 jugadores el
 * costo O(n) es despreciable y evita estructuras mas complejas.
 * Devuelve el indice o -1 si no existe (o si el nombre es invalido,
 * porque un nombre invalido nunca puede estar guardado).
 */
=======
>>>>>>> main
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

<<<<<<< HEAD
/*
 * ranking_obtener_victorias: atajo de consulta para la interfaz
 * ("¿cuantas lleva Ana?"). Devuelve 0 si el jugador no figura, que es
 * el valor correcto para alguien que aun no gano nada.
 */
=======
>>>>>>> main
int ranking_obtener_victorias(const Ranking *r, const char *nombre)
{
    int idx;
    idx = ranking_buscar(r, nombre);
    if (idx < 0) {
        return 0;
    }
    return r->entradas[idx].victorias;
}

<<<<<<< HEAD
/*
 * ranking_ordenar: ordena el arreglo in situ de mejor a peor puesto.
 *
 * Criterio compuesto (primero el que desempatas, despues el otro):
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
 * de structs, valida en ANSI C.
 */
=======
>>>>>>> main
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

<<<<<<< HEAD
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
 *   3. reordena SIEMPRE (mantiene el INVARIANTE 2: el ranking nunca
 *      queda desordenado, ni siquiera entre llamadas).
 */
=======
>>>>>>> main
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

<<<<<<< HEAD
/*
 * ranking_cargar: lee el archivo `ruta` y reconstruye el ranking.
 *
 * Protocolo de lectura, linea por linea con fgets():
 *  1. Vacia el ranking primero (cantidad = 0): cargar SIEMPRE parte
 *     de cero, nunca agrega sobre datos previos.
 *  2. Si el archivo no existe -> RANKING_ERROR_ARCHIVO. NO es un error
 *     fatal: app.c lo interpreta como "ranking vacio inicial" y sigue.
 *  3. Por cada linea:
 *     a. Deteccion de linea MAS LARGA que el buffer: fgets() trunca
 *        en silencio cuando la linea no entra. Se detecta porque el
 *        buffer quedo lleno Y no se vio el '\n' (el !feof discrimina
 *        la ultima linea corta sin salto final). El resto de la linea
 *        se consume con fgetc() hasta el '\n'/EOF para no confundir el
 *        resto con la linea siguiente; la linea se descarta y marca
 *        error de formato.
 *     b. Se quita el salto y se ignoran lineas vacias o de solo
 *        espacios (no son error: permiten archivos prolijos).
 *     c. Se parte en DOS por el PRIMER ';' (strchr). Sin ';' -> error
 *        de formato. La mitad izquierda es el nombre, la derecha el
 *        numero (puede contener otro ';', que hara fallar el parseo).
 *     d. Se validan ambas mitades con nombre_normalizado() y
 *        parsear_victorias(). Cualquiera que falle -> se saltea la
 *        linea y se marca error de formato, SIN abortar el resto.
 *     e. Si el nombre ya estaba (insensible a mayusculas) se FUSIONA
 *        sumando victorias con tope; si no, se agrega si hay lugar.
 *  4. Al final se ordena y se devuelve RANKING_OK, o
 *     RANKING_ERROR_FORMATO si hubo al menos una linea mala (pero con
 *     las lineas validas ya cargadas: carga tolerante, decision 4).
 */
=======
>>>>>>> main
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

<<<<<<< HEAD
/*
 * ranking_guardar: escribe TODO el ranking en `ruta`, una linea
 * "nombre;victorias\n" por jugador, en el orden actual (que ya esta
 * ordenado por el INVARIANTE 2). Abre con "w": trunca el archivo
 * anterior y lo reescribe completo, asi el archivo nunca tiene
 * entradas viejas duplicadas. No modifica la memoria.
 *
 * Deteccion de errores: se controla el retorno de cada fprintf()
 * (falla si el disco se lleno o se corta la escritura) y tambien el
 * de fclose(), porque la escritura en C es con buffer y el error real
 * puede aparecer recien al volcar el buffer al cerrar. Cualquier falla
 * -> RANKING_ERROR_ARCHIVO para que la interfaz avise al usuario.
 */
=======
>>>>>>> main
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

<<<<<<< HEAD
/*
 * ranking_mensaje_error: traduce un codigo de retorno a texto en
 * espanol para mostrar en pantalla. La logica devuelve numeros y la
 * interfaz los convierte con esta funcion: asi los mensajes viven en
 * un solo lugar y la futura GUI puede reutilizarlos o reemplazarlos.
 */
=======
>>>>>>> main
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

<<<<<<< HEAD
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
=======
>>>>>>> main
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

<<<<<<< HEAD
/*
 * ranking_mostrar: atajo que imprime en pantalla (stdout). Es lo que
 * llama app.c para la pantalla final de consola exigida por la
 * consigna. No hace nada mas: delega todo en ranking_mostrar_en.
 */
=======
>>>>>>> main
void ranking_mostrar(const Ranking *r)
{
    if (r == NULL) {
        return;
    }
    ranking_mostrar_en(stdout, r);
}
