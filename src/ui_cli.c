/**
 * @file ui_cli.c
 * @brief Interfaz de consola de Batalla Naval.
 *
 * Este archivo SOLO interactua con el usuario: limpia la pantalla, pide
 * nombres y opciones del menu, muestra textos, el reglamento y los tableros,
 * y espera confirmaciones. NO aplica reglas del juego, NO modifica el tablero
 * y NO suma victorias. Esas decisiones viven en tablero.c y ranking.c.
 *
 * @note Manejo de entrada: scanf("%d") deja el '\n' pendiente y, si el
 *       usuario escribe letras, falla sin limpiar el buffer. Por eso todo se
 *       lee con fgets() y se procesa como texto. leer_linea() consume siempre
 *       la linea completa y leer_opcion() convierte con strtol() exigiendo
 *       que no queden caracteres extra.
 * @note Si se alcanza EOF (entrada redirigida o cerrada), las funciones
 *       avisan al llamador y el menu devuelve MODO_SALIR para no colgarse.
 * @note Estandar unico: C99 (-std=c99 -pedantic -Wall -Wextra).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_cli.h"
#include "config.h"

/**
 * @brief Ancho de la linea decorativa del menu.
 */
#define UI_ANCHO_LINEA 67

/**
 * @brief Nombres de los jugadores humanos de la partida actual.
 * @note El arreglo es interno (static): ningun otro modulo debe modificarlo.
 */
static char nombres_jugadores[CONFIG_JUGADORES_PARTIDA][CONFIG_NOMBRE_MAX];

/**
 * @brief Cantidad de jugadores humanos ya registrados.
 */
static int cantidad_de_jugadores = 0;

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
 * @note Siempre deja `destino` terminado en '\0' cuando puede escribir.
 */
static int leer_linea(char *destino, size_t tam)
{
    size_t largo; /* Largo actual de la linea leida. */
    int c;        /* Caracter usado para descartar el resto. */

    /* Validacion defensiva de los argumentos. */
    if (destino == NULL || tam == 0) {
        return 0;
    }

    /* EOF o error de lectura: se informa al llamador. */
    if (fgets(destino, (int)tam, stdin) == NULL) {
        destino[0] = '\0';
        return 0;
    }

    /* Si la linea entro completa, se quita el salto de linea. */
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
 * @note Se aplica al nombre antes de saludar; ranking.c volvera a recortar y
 *       validar el nombre cuando se registre la victoria.
 */
static void recortar_espacios(char *texto)
{
    char *ini;    /* Primer caracter no espacio. */
    char *fin;    /* Posicion del '\0' recortado. */
    size_t largo; /* Largo del texto ya recortado por izquierda. */

    /* Sin texto no hay nada que recortar. */
    if (texto == NULL) {
        return;
    }

    /* Avanzar sobre los espacios iniciales. */
    ini = texto;
    while (*ini == ' ' || *ini == '\t') {
        ini++;
    }

    /* Si habia espacios iniciales, mover el contenido al inicio del buffer. */
    if (ini != texto) {
        largo = strlen(ini);
        memmove(texto, ini, largo + 1);
    }

    /* Retroceder desde el final sobre espacios y tabuladores. */
    fin = texto + strlen(texto);
    while (fin > texto && (fin[-1] == ' ' || fin[-1] == '\t')) {
        fin--;
    }
    *fin = '\0';
}

/**
 * @brief Lee una linea y la convierte a una opcion de menu.
 *
 * @return 0, 1 o 2 si es una opcion valida; -1 en cualquier otro caso
 *         (linea vacia, letras, numero fuera de rango o EOF).
 * @note Se usa strtol() y se exige que no queden caracteres extra, de modo
 *       que "1abc" o "  " se rechacen.
 */
static int leer_opcion(void)
{
    char linea[16]; /* Texto crudo ingresado por el usuario. */
    char *fin;      /* Primer caracter no convertido por strtol. */
    long valor;     /* Numero convertido. */

    /* EOF: no hay opcion posible. */
    if (!leer_linea(linea, sizeof(linea))) {
        return -1;
    }

    /* Conversion decimal con deteccion del final del numero. */
    valor = strtol(linea, &fin, 10);

    /* Se toleran espacios o tabuladores despues del numero. */
    while (*fin == ' ' || *fin == '\t') {
        fin++;
    }

    /* Sin digitos o con basura despues del numero: opcion invalida. */
    if (fin == linea || *fin != '\0') {
        return -1;
    }

    /* Solo se aceptan las opciones 0, 1 y 2. */
    if (valor < 0 || valor > 2) {
        return -1;
    }
    return (int)valor;
}

/**
 * @brief Limpia la consola.
 *
 * En Windows usa `cls`; en Linux/Unix usa `clear` si hay terminal. Si no hay
 * terminal disponible, imprime lineas en blanco.
 *
 * @return No devuelve valor.
 * @note El resultado de `system` se ignora con (void) porque no hay una
 *       accion util que tomar si el comando de limpieza falla.
 */
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
    /* Plataforma desconocida: separacion simple, sin comandos externos. */
    printf("\n\n");
#endif
}

/**
 * @brief Muestra una pausa y consume una linea completa.
 *
 * @return No devuelve valor.
 * @note Usa fflush(stdout) para que el mensaje se vea antes de bloquearse.
 */
void esperar_usuario(void)
{
    char buffer[8]; /* Solo se necesita consumir la linea. */

    printf("Presione ENTER para continuar.\n");
    fflush(stdout);
    (void)leer_linea(buffer, sizeof(buffer));
}

/**
 * @brief Pide el nombre de un jugador y lo guarda en `destino`.
 *
 * @param numero_de_jugador Numero base cero usado para numerar el mensaje.
 * @param destino Buffer donde se guarda el nombre terminado en '\0'.
 * @param tam Tamano total del buffer, incluido el '\0'.
 * @return 1 si leyo una linea; 0 si se alcanzo EOF o los argumentos son
 *         invalidos.
 * @note Recorta espacios de los extremos, pero no valida el contenido.
 */
int pedir_nombre_jugador(int numero_de_jugador, char *destino, size_t tam)
{
    /* Validacion defensiva antes de escribir en el buffer. */
    if (destino == NULL || tam == 0) {
        return 0;
    }

    /* El numero se muestra base uno para el usuario. */
    printf("Ingrese el nombre del jugador %d: ", numero_de_jugador + 1);
    fflush(stdout);

    /* EOF: dejar el buffer vacio y avisar al llamador. */
    if (!leer_linea(destino, tam)) {
        destino[0] = '\0';
        return 0;
    }

    /* Limpieza cosmetica; la validacion real vive en ranking.c. */
    recortar_espacios(destino);
    return 1;
}

/**
 * @brief Imprime un saludo para el nombre indicado.
 *
 * @param nombre_jugador Nombre a saludar.
 * @return No devuelve valor.
 * @note Si el puntero es NULL no imprime nada.
 */
void saludar_jugador(const char *nombre_jugador)
{
    /* Un puntero nulo no es un nombre: no hay nada que mostrar. */
    if (nombre_jugador == NULL) {
        return;
    }
    printf("Bienvenido %s!\n", nombre_jugador);
}

/**
 * @brief Pide el nombre del siguiente jugador humano y lo registra.
 *
 * @return No devuelve valor.
 * @note Repite la pregunta si el nombre queda vacio y sale sin registrar si
 *       se alcanza EOF. Como maximo registra CONFIG_JUGADORES_PARTIDA.
 */
void ingresar_jugador(void)
{
    int indice; /* Posicion del jugador que se esta registrando. */

    /* Tope de jugadores humanos permitidos por la configuracion. */
    if (cantidad_de_jugadores >= CONFIG_JUGADORES_PARTIDA) {
        printf("Ya se registraron los %d jugadores.\n", CONFIG_JUGADORES_PARTIDA);
        return;
    }

    indice = cantidad_de_jugadores;

    /* Repetir hasta obtener un nombre no vacio o hasta EOF. */
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

    /* Saludo y actualizacion del contador de jugadores registrados. */
    saludar_jugador(nombres_jugadores[indice]);
    cantidad_de_jugadores++;
}

/**
 * @brief Copia a stdout el contenido de un archivo de texto.
 *
 * @param ruta_al_archivo Ruta relativa o absoluta del archivo.
 * @return No devuelve valor.
 * @note Si la ruta es NULL o el archivo no se puede abrir, informa el
 *       problema en pantalla.
 */
void mostrar_archivo_de_texto(const char *ruta_al_archivo)
{
    FILE *archivo; /* Archivo de texto abierto en modo lectura. */
    int c;         /* Caracter leido. */

    /* Validacion de la ruta antes de llamar a fopen. */
    if (ruta_al_archivo == NULL) {
        printf("Ruta de archivo invalida.\n");
        return;
    }

    /* Apertura en modo texto: el reglamento es un archivo legible. */
    archivo = fopen(ruta_al_archivo, "r");
    if (archivo == NULL) {
        printf("No se pudo abrir \"%s\".\n", ruta_al_archivo);
        return;
    }

    /* Copia caracter por caracter hasta EOF. */
    while ((c = fgetc(archivo)) != EOF) {
        putchar(c);
    }

    /* Cerrar siempre, aunque fgetc haya llegado a EOF. */
    fclose(archivo);
}

/**
 * @brief Muestra la pantalla inicial y el reglamento.
 *
 * @return No devuelve valor.
 * @note Limpia la pantalla, pide el primer jugador, muestra el reglamento y
 *       espera ENTER. Depende de CONFIG_REGLAMENTO_RUTA.
 */
void mostrar_presentacion_y_reglamento(void)
{
    /* Pantalla limpia y captura del primer jugador. */
    limpiar_pantalla_cli();
    ingresar_jugador();

    /* Reglamento y pausa para que el usuario lo lea. */
    limpiar_pantalla_cli();
    mostrar_archivo_de_texto(CONFIG_REGLAMENTO_RUTA);
    esperar_usuario();
}

/**
 * @brief Imprime las opciones del menu principal.
 *
 * @return No devuelve valor.
 * @note Solo muestra; no lee ni valida ninguna opcion.
 */
void mostrar_menu_principal(void)
{
    /* Marco superior del menu. */
    imprimir_linea();
    printf("MENU PRINCIPAL\n");
    imprimir_linea();

    /* Opciones numeradas con los valores del enum ModoJuego. */
    printf("    %d. Jugador vs Jugador\n", (int)MODO_JUGADOR_VS_JUGADOR);
    printf("    %d. Jugador vs Maquina\n", (int)MODO_JUGADOR_VS_MAQUINA);
    printf("    %d. Salir del juego.\n", (int)MODO_SALIR);

    /* Marco inferior del menu. */
    imprimir_linea();
}

/**
 * @brief Imprime una linea horizontal de separacion.
 *
 * @return No devuelve valor.
 * @note Usa UI_ANCHO_LINEA caracteres '-' seguidos de un salto de linea.
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
 * @brief Muestra el menu y devuelve la opcion valida elegida.
 *
 * @return El ModoJuego elegido.
 * @note Si la entrada es invalida, avisa y vuelve a preguntar. Si se alcanza
 *       EOF, devuelve MODO_SALIR para no colgar el programa.
 */
ModoJuego elegir_modo_de_juego(void)
{
    int opcion; /* Opcion cruda devuelta por leer_opcion. */

    /* Bucle hasta obtener una opcion valida o detectar EOF. */
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

        /* Entrada invalida: avisar y volver a mostrar el menu. */
        printf("Opcion invalida. Intente de nuevo.\n");
        esperar_usuario();
    }
}
