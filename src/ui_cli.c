/**
 * @file ui_cli.c
 * @brief Interfaz de consola de Batalla Naval.
 *
 * Este archivo SOLO interactua con el usuario: limpia la pantalla, pide
 * nombres, opciones y coordenadas, muestra textos, menus y el ranking, y
 * espera confirmaciones. NO aplica reglas del juego, NO modifica el tablero
 * y NO suma victorias. Esas decisiones viven en tablero.c, game.c y ranking.c.
 *
 * @note Manejo de entrada: todo se lee con fgets() y se procesa como texto.
 *       leer_linea() consume siempre la linea completa y
 *       leer_entero_rango() convierte con strtol() exigiendo que no queden
 *       caracteres extra.
 * @note Si se alcanza EOF (entrada redirigida o cerrada), las funciones
 *       avisan al llamador y los menus devuelven 0 o -1 para no colgarse.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ranking.h"
#include "ui_cli.h"

/**
 * @brief Ancho de la linea decorativa del menu.
 */
#define UI_ANCHO_LINEA 67

/**
 * @brief Lee una linea completa de stdin en `destino`.
 *
 * Algoritmo:
 *  1. fgets() lee hasta tam-1 caracteres o hasta el '\n'.
 *  2. Si encontro '\n', lo reemplaza por '\0'.
 *  3. Si NO lo encontro, la linea era demasiado larga: descarta el resto con
 *     getchar() hasta el fin de linea o EOF, para que la proxima lectura
 *     empiece limpia.
 *  4. Si quedo un '\r' final (entradas con finales Windows), lo quita.
 *
 * @param destino Buffer donde se guarda la linea.
 * @param tam Tamano total del buffer.
 * @return 1 si leyo algo; 0 en EOF o si los argumentos son invalidos.
 */
static int leer_linea(char *destino, size_t tam)
{
    size_t largo; /* Largo actual de la linea leida. */
    int c;        /* Caracter usado para descartar el resto. */

    if (destino == NULL || tam == 0) {
        return 0;
    }

    if (fgets(destino, (int)tam, stdin) == NULL) {
        destino[0] = '\0';
        return 0;
    }

    largo = strlen(destino);
    if (largo > 0 && destino[largo - 1] == '\n') {
        destino[largo - 1] = '\0';
        largo--;
    } else {
        /* Linea mas larga que el buffer: descartar el resto. */
        c = getchar();
        while (c != '\n' && c != EOF) {
            c = getchar();
        }
    }

    /* Compatibilidad con entradas que traen CRLF. */
    if (largo > 0 && destino[largo - 1] == '\r') {
        destino[largo - 1] = '\0';
    }
    return 1;
}

/**
 * @brief Quita espacios y tabuladores de los extremos de un texto.
 *
 * @param texto Cadena modificada in situ.
 * @return No devuelve valor.
 */
static void recortar_espacios(char *texto)
{
    char *ini;    /* Primer caracter no espacio. */
    char *fin;    /* Posicion del '\0' recortado. */
    size_t largo; /* Largo del texto ya recortado por izquierda. */

    if (texto == NULL) {
        return;
    }

    ini = texto;
    while (*ini == ' ' || *ini == '\t') {
        ini++;
    }

    if (ini != texto) {
        largo = strlen(ini);
        memmove(texto, ini, largo + 1);
    }

    fin = texto + strlen(texto);
    while (fin > texto && (fin[-1] == ' ' || fin[-1] == '\t')) {
        fin--;
    }
    *fin = '\0';
}

/**
 * @brief Lee un entero completo dentro de un rango.
 *
 * @param minimo Valor minimo aceptado.
 * @param maximo Valor maximo aceptado.
 * @return El numero convertido o -1 si la entrada es invalida o hay EOF.
 */
static int leer_entero_rango(int minimo, int maximo)
{
    char linea[32]; /* Texto crudo ingresado. */
    char *fin;      /* Primer caracter no convertido. */
    long valor;     /* Numero convertido. */

    if (!leer_linea(linea, sizeof(linea))) {
        return -1;
    }

    valor = strtol(linea, &fin, 10);

    while (*fin == ' ' || *fin == '\t') {
        fin++;
    }

    if (fin == linea || *fin != '\0') {
        return -1;
    }

    if (valor < minimo || valor > maximo) {
        return -1;
    }

    return (int)valor;
}

/**
 * @brief Limpia la consola.
 *
 * @return No devuelve valor.
 */
void limpiar_pantalla_cli(void)
{
#if defined(_WIN32)
    (void)system("cls");
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    if (getenv("TERM") != NULL) {
        (void)system("clear");
    } else {
        printf("\n\n");
    }
#else
    printf("\n\n");
#endif
}

/**
 * @brief Muestra una pausa y consume una linea completa.
 *
 * @return No devuelve valor.
 */
void esperar_usuario(void)
{
    char buffer[8]; /* Solo se necesita consumir la linea. */

    printf("Presione ENTER para continuar.\n");
    fflush(stdout);
    (void)leer_linea(buffer, sizeof(buffer));
}

/**
 * @brief Imprime una linea horizontal de separacion.
 *
 * @return No devuelve valor.
 */
void imprimir_linea(void)
{
    int i; /* Contador de caracteres. */

    for (i = 0; i < UI_ANCHO_LINEA; i++) {
        putchar('-');
    }
    putchar('\n');
}

/**
 * @brief Pide una opcion de menu por consola.
 *
 * @return Opcion 0..4 o -1 si la entrada es invalida o hay EOF.
 */
int pedir_opcion_al_usuario(void)
{
    printf("Ingrese una opcion: ");
    fflush(stdout);
    return leer_entero_rango(0, 4);
}

/**
 * @brief Pide el nombre de un jugador y lo guarda en `destino`.
 *
 * @param numero_de_jugador Numero base cero usado solo para numerar el mensaje.
 * @param destino Buffer donde se guarda el nombre terminado en '\0'.
 * @param tam Tamano total del buffer, incluido el '\0'.
 * @return 1 si leyo una linea; 0 si se alcanzo EOF o los argumentos son
 *         invalidos.
 */
int pedir_nombre_jugador(int numero_de_jugador, char *destino, size_t tam)
{
    if (destino == NULL || tam == 0) {
        return 0;
    }

    printf("Ingrese el nombre del jugador %d: ", numero_de_jugador + 1);
    fflush(stdout);

    if (!leer_linea(destino, tam)) {
        destino[0] = '\0';
        return 0;
    }

    recortar_espacios(destino);
    return 1;
}

/**
 * @brief Imprime un saludo para el nombre indicado.
 *
 * @param nombre_jugador Nombre a saludar.
 * @return No devuelve valor.
 */
void saludar_jugador(const char *nombre_jugador)
{
    if (nombre_jugador == NULL) {
        return;
    }
    printf("Bienvenido %s!\n", nombre_jugador);
}

/**
 * @brief Copia a stdout el contenido de un archivo de texto.
 *
 * @param ruta_al_archivo Ruta relativa o absoluta del archivo.
 * @return No devuelve valor.
 */
void mostrar_archivo_de_texto(const char *ruta_al_archivo)
{
    FILE *archivo; /* Archivo de texto abierto en modo lectura. */
    int c;         /* Caracter leido. */

    if (ruta_al_archivo == NULL) {
        printf("Ruta de archivo invalida.\n");
        return;
    }

    archivo = fopen(ruta_al_archivo, "r");
    if (archivo == NULL) {
        printf("No se pudo abrir \"%s\".\n", ruta_al_archivo);
        return;
    }

    while ((c = fgetc(archivo)) != EOF) {
        putchar(c);
    }

    fclose(archivo);
}

/**
 * @brief Muestra la pantalla inicial y el reglamento.
 *
 * @param nombre Buffer donde se guarda el nombre del jugador 1.
 * @param tam Tamano total del buffer.
 * @return No devuelve valor.
 */
void mostrar_presentacion_y_reglamento(char *nombre, size_t tam)
{
    if (nombre == NULL || tam == 0) {
        return;
    }

    limpiar_pantalla_cli();

    /* Repetir hasta obtener un nombre no vacio o hasta EOF. */
    for (;;) {
        if (!pedir_nombre_jugador(0, nombre, tam)) {
            (void)snprintf(nombre, tam, "Jugador 1");
            break;
        }
        if (nombre[0] != '\0') {
            break;
        }
        printf("El nombre no puede estar vacio. Intente de nuevo.\n");
    }

    limpiar_pantalla_cli();
    imprimir_linea();
    saludar_jugador(nombre);
    imprimir_linea();
    mostrar_archivo_de_texto(CONFIG_REGLAMENTO_RUTA);
    imprimir_linea();
    esperar_usuario();
}

/**
 * @brief Imprime las opciones del menu principal.
 *
 * @return No devuelve valor.
 */
void mostrar_menu_principal(void)
{
    imprimir_linea();
    printf("MENU PRINCIPAL\n");
    imprimir_linea();
    printf("    1. Jugador vs Jugador\n");
    printf("    2. Jugador vs Maquina\n");
    printf("    3. Mostrar ranking.\n");
    printf("    4. Mostrar reglamento.\n");
    printf("    0. Salir del juego.\n");
    imprimir_linea();
}

/**
 * @brief Imprime las opciones del menu de dificultad.
 *
 * @return No devuelve valor.
 */
void mostrar_menu_de_dificultad(void)
{
    imprimir_linea();
    printf("NIVEL DE DIFICULTAD\n");
    imprimir_linea();
    printf("    1. Modo Normal.\n");
    printf("    2. Modo Tormenta.\n");
    printf("    0. Volver al menu principal.\n");
    imprimir_linea();
}

/**
 * @brief Muestra el ranking actual sin modificarlo.
 *
 * @return No devuelve valor.
 * @note La lectura y el formateo se delegan en ranking.c; la interfaz solo
 *       muestra el resultado.
 */
static void mostrar_ranking_actual(void)
{
    Ranking ranking; /* Copia local del ranking. */
    int codigo;      /* Resultado de la carga. */

    ranking_inicializar(&ranking);
    codigo = ranking_cargar(&ranking, CONFIG_RANKING_RUTA);

    if (codigo == RANKING_ERROR_ARCHIVO) {
        printf("Todavia no hay victorias registradas.\n");
    } else if (codigo != RANKING_OK) {
        printf("%s\n", ranking_mensaje_error(codigo));
    } else {
        ranking_mostrar(&ranking);
    }

    esperar_usuario();
}

/**
 * @brief Muestra el menu de dificultad y guarda la opcion elegida.
 *
 * @param dificultad Destino de la dificultad elegida.
 * @return 1 si se eligio una dificultad; 0 si se volvio atras o hay EOF.
 */
int elegir_dificultad(Dificultad *dificultad)
{
    int opcion; /* Opcion cruda del menu. */

    if (dificultad == NULL) {
        return 0;
    }

    for (;;) {
        limpiar_pantalla_cli();
        mostrar_menu_de_dificultad();
        opcion = pedir_opcion_al_usuario();

        if (opcion < 0) {
            if (feof(stdin)) {
                return 0;
            }
            printf("Opcion invalida.\n");
            esperar_usuario();
            continue;
        }

        if (opcion == 0) {
            return 0;
        }
        if (opcion == 1) {
            *dificultad = NORMAL;
            return 1;
        }
        if (opcion == 2) {
            *dificultad = TORMENTA;
            return 1;
        }

        printf("Opcion invalida.\n");
        esperar_usuario();
    }
}

/**
 * @brief Muestra el menu principal y guarda modo y dificultad.
 *
 * @param modo Destino del modo elegido.
 * @param dificultad Destino de la dificultad elegida.
 * @return 1 si se eligio un modo; 0 si el usuario salio o hay EOF.
 */
int elegir_modo_de_juego(Modo_de_juego *modo, Dificultad *dificultad)
{
    int opcion; /* Opcion cruda del menu. */

    if (modo == NULL || dificultad == NULL) {
        return 0;
    }

    for (;;) {
        limpiar_pantalla_cli();
        mostrar_menu_principal();
        opcion = pedir_opcion_al_usuario();

        if (opcion < 0) {
            if (feof(stdin)) {
                return 0;
            }
            printf("Opcion invalida.\n");
            esperar_usuario();
            continue;
        }

        switch (opcion) {
        case 0:
            printf("Saliendo...\n");
            return 0;

        case 1:
            if (elegir_dificultad(dificultad)) {
                *modo = PVP;
                return 1;
            }
            break;

        case 2:
            if (elegir_dificultad(dificultad)) {
                *modo = PVIA;
                return 1;
            }
            break;

        case 3:
            limpiar_pantalla_cli();
            mostrar_ranking_actual();
            break;

        case 4:
            limpiar_pantalla_cli();
            mostrar_archivo_de_texto(CONFIG_REGLAMENTO_RUTA);
            esperar_usuario();
            break;

        default:
            printf("Opcion invalida.\n");
            esperar_usuario();
            break;
        }
    }
}

/**
 * @brief Pide una fila del tablero.
 *
 * @return Fila base cero o -1 si la entrada es invalida.
 */
int solicitar_entrada_de_fila(void)
{
    int fila; /* Fila base uno ingresada. */

    printf("Fila (1 a %d): ", CONFIG_FILAS);
    fflush(stdout);
    fila = leer_entero_rango(1, CONFIG_FILAS);

    if (fila < 0) {
        printf("Fila invalida.\n");
        return -1;
    }

    return fila - 1;
}

/**
 * @brief Pide una columna del tablero.
 *
 * @return Columna base cero o -1 si la entrada es invalida.
 */
int solicitar_entrada_de_columna(void)
{
    char linea[16]; /* Linea cruda ingresada. */
    int columna;    /* Columna convertida. */

    printf("Columna (A a %c): ", (char)('A' + CONFIG_COLUMNAS - 1));
    fflush(stdout);

    if (!leer_linea(linea, sizeof(linea))) {
        return -1;
    }

    if (linea[0] >= 'A' && linea[0] <= (char)('A' + CONFIG_COLUMNAS - 1)
        && linea[1] == '\0') {
        columna = linea[0] - 'A';
    } else if (linea[0] >= 'a'
               && linea[0] <= (char)('a' + CONFIG_COLUMNAS - 1)
               && linea[1] == '\0') {
        columna = linea[0] - 'a';
    } else {
        printf("Columna invalida.\n");
        return -1;
    }

    return columna;
}
