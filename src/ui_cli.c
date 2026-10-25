/*
 * ui_cli.c - Interfaz de consola de Batalla Naval.
 *
 * ---------------------------------------------------------------------------
 * RESPONSABILIDAD
 * ---------------------------------------------------------------------------
 * Este archivo SOLO interactua con el usuario:
 *   - limpia la pantalla;
 *   - pide nombres y opciones del menu;
 *   - muestra textos, el reglamento y los tableros;
 *   - espera confirmaciones.
 *
 * NO aplica reglas del juego, NO modifica el tablero y NO suma victorias.
 * Esas decisiones viven en tablero.c y ranking.c. app.c coordina el orden
 * de las pantallas.
 *
 * ---------------------------------------------------------------------------
 * MANEJO DE ENTRADA (por que no se usa scanf("%d") a secas)
 * ---------------------------------------------------------------------------
 * scanf("%d", ...) deja el '\n' pendiente y, si el usuario escribe letras,
 * falla sin limpiar el buffer. Eso produce bucles infinitos y preguntas
 * que se responden solas. Por eso todo se lee con fgets() y se procesa
 * como texto:
 *   - leer_linea() consume SIEMPRE la linea completa (y descarta el resto
 *     si no entra en el buffer);
 *   - leer_opcion() convierte con strtol() y exige que no queden caracteres
 *     extra, asi "1abc" o "  " se rechazan.
 *
 * Si se alcanza EOF (entrada redirigida o cerrada), las funciones avisan
 * al llamador y el menu devuelve MODO_SALIR para no colgarse.
 *
 * ---------------------------------------------------------------------------
 * ESTANDAR
 * ---------------------------------------------------------------------------
 * C99 (-std=c99 -pedantic -Wall -Wextra). Se usan comentarios // y
 * declaraciones dentro de bloques, permitidos desde C99.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_cli.h"
#include "config.h"

/* Ancho de la linea decorativa del menu. */
#define UI_ANCHO_LINEA 67

/*
 * Nombres de los jugadores humanos de la partida actual.
 * El arreglo es interno (static): ningun otro modulo debe modificarlo.
 */
static char nombres_jugadores[CONFIG_JUGADORES_PARTIDA][CONFIG_NOMBRE_MAX];
static int cantidad_de_jugadores = 0;

/*
 * leer_linea: lee una linea completa de stdin en `destino`.
 *
 * Algoritmo:
 *  1. fgets() lee hasta tam-1 caracteres o hasta el '\n'.
 *  2. Si encontro '\n', lo reemplaza por '\0'.
 *  3. Si NO lo encontro, la linea era demasiado larga: se descarta el resto
 *     con getchar() hasta el fin de linea o EOF. Asi la proxima lectura
 *     empieza limpia.
 *  4. Si quedo un '\r' final (entradas con finales Windows), se quita.
 *
 * Devuelve 1 si leyo algo, 0 en EOF o argumentos invalidos.
 */
static int leer_linea(char *destino, size_t tam)
{
    size_t largo;
    int c;

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
    if (largo > 0 && destino[largo - 1] == '\r') {
        destino[largo - 1] = '\0';
    }
    return 1;
}

/*
 * recortar_espacios: quita espacios y tabuladores de los extremos.
 * Se aplica al nombre antes de saludar; ranking.c volvera a recortar y
 * validar el nombre cuando se registre la victoria.
 */
static void recortar_espacios(char *texto)
{
    char *ini;
    char *fin;
    size_t largo;

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

/*
 * leer_opcion: lee una linea y la convierte a entero.
 * Devuelve 0, 1 o 2 si es una opcion valida; -1 en cualquier otro caso
 * (linea vacia, letras, numero fuera de rango o EOF).
 */
static int leer_opcion(void)
{
    char linea[16];
    char *fin;
    long valor;

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
    if (valor < 0 || valor > 2) {
        return -1;
    }
    return (int)valor;
}

void limpiar_pantalla_cli(void)
{
#if defined(_WIN32)
    /* Windows: el comando cls del interprete de comandos. */
    (void)system("cls");
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    /*
     * Linux/Unix: clear solo si hay terminal. Si TERM no esta definido
     * (por ejemplo, al redirigir la salida a un archivo), se imprimen
     * lineas en blanco para no ensuciar la salida con errores de clear.
     */
    if (getenv("TERM") != NULL) {
        (void)system("clear");
    } else {
        printf("\n\n");
    }
#else
    printf("\n\n");
#endif
}

void esperar_usuario(void)
{
    char buffer[8];

    printf("Presione ENTER para continuar.\n");
    fflush(stdout);
    (void)leer_linea(buffer, sizeof(buffer));
}

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

void saludar_jugador(const char *nombre_jugador)
{
    if (nombre_jugador == NULL) {
        return;
    }
    printf("Bienvenido %s!\n", nombre_jugador);
}

void ingresar_jugador(void)
{
    int indice;

    if (cantidad_de_jugadores >= CONFIG_JUGADORES_PARTIDA) {
        printf("Ya se registraron los %d jugadores.\n", CONFIG_JUGADORES_PARTIDA);
        return;
    }

    indice = cantidad_de_jugadores;
    for (;;) {
        if (!pedir_nombre_jugador(indice, nombres_jugadores[indice],
                                  CONFIG_NOMBRE_MAX)) {
            /* EOF: no hay mas entrada; se sale sin registrar. */
            return;
        }
        if (nombres_jugadores[indice][0] != '\0') {
            break;
        }
        printf("El nombre no puede estar vacio. Intente de nuevo.\n");
    }

    saludar_jugador(nombres_jugadores[indice]);
    cantidad_de_jugadores++;
}

void mostrar_archivo_de_texto(const char *ruta_al_archivo)
{
    FILE *archivo;
    int c;

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

void mostrar_presentacion_y_reglamento(void)
{
    limpiar_pantalla_cli();
    ingresar_jugador();
    limpiar_pantalla_cli();
    mostrar_archivo_de_texto(CONFIG_REGLAMENTO_RUTA);
    esperar_usuario();
}

void mostrar_menu_principal(void)
{
    imprimir_linea();
    printf("MENU PRINCIPAL\n");
    imprimir_linea();
    printf("    %d. Jugador vs Jugador\n", (int)MODO_JUGADOR_VS_JUGADOR);
    printf("    %d. Jugador vs Maquina\n", (int)MODO_JUGADOR_VS_MAQUINA);
    printf("    %d. Salir del juego.\n", (int)MODO_SALIR);
    imprimir_linea();
}

void imprimir_linea(void)
{
    int i;

    for (i = 0; i < UI_ANCHO_LINEA; i++) {
        putchar('-');
    }
    putchar('\n');
}

ModoJuego elegir_modo_de_juego(void)
{
    int opcion;

    for (;;) {
        limpiar_pantalla_cli();
        mostrar_menu_principal();
        printf("Ingrese una opcion: ");
        fflush(stdout);

        opcion = leer_opcion();
        if (opcion >= 0) {
            return (ModoJuego)opcion;
        }
        if (feof(stdin)) {
            /* No queda entrada: salir en lugar de repetir para siempre. */
            return MODO_SALIR;
        }
        printf("Opcion invalida. Intente de nuevo.\n");
        esperar_usuario();
    }
}
