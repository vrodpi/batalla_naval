/**
 * @file demo_ranking.c
 * @brief Demo ejecutable del ranking binario (Opcion A).
 *
 * Es la maqueta del final de partida que luego vivira en app.c + ui_cli.c.
 * Permite probar el modulo ranking de punta a punta sin el juego completo:
 * pedir nombre -> saludar -> cargar ranking binario -> sumar 1 victoria ->
 * guardar -> mostrar tabla -> esperar ENTER.
 *
 * Trazabilidad de una victoria (cada flecha es una llamada real):
 *   jugador escribe el nombre
 *     -> leer_linea()                 [captura la linea completa]
 *     -> ranking_cargar()             [volcado binario o vacio si no existe]
 *     -> ranking_registrar_victoria() [valida, suma, ordena]
 *     -> ranking_guardar()            [volcado binario completo]
 *     -> ranking_mostrar()            [tabla ordenada]
 *     -> esperar_tecla()              [congela la pantalla final]
 *
 * @note El formato es binario (Opcion A): una copia cruda del struct Ranking,
 *       SIN magic number, version ni checksum. Si esta truncado o corrupto,
 *       cargar devuelve RANKING_ERROR_FORMATO y se arranca con ranking vacio.
 * @note Un ranking.dat del formato texto viejo se rechaza por tamano (no hay
 *       migracion automatica): borrarlo a mano si existe.
 *
 * Uso:
 *   demo_ranking            pide el nombre por teclado
 *   demo_ranking "Ana"      toma el ganador de argv[1] (pruebas)
 */

#include <stdio.h>
#include <string.h>

#include "ranking.h"
#include "config.h"

/**
 * @brief Lee una linea completa del teclado en `dest`.
 *
 * Se usa fgets() y no scanf("%s") porque scanf corta en el primer espacio
 * ("Ana Maria" quedaria "Ana") y no controla el desborde. gets() esta
 * prohibido. Si hay EOF o error, se deja un string vacio para que el llamador
 * lo trate como nombre invalido. Luego se quitan '\n' y '\r' finales (el '\r'
 * aparece si la entrada viene con finales Windows).
 *
 * @param dest Buffer donde se guarda la linea.
 * @param tam Tamano total del buffer.
 * @return No devuelve valor.
 * @note Si hay EOF o error, deja un string vacio para que el llamador lo
 *       trate como nombre invalido.
 */
static void leer_linea(char *dest, int tam)
{
    /* EOF o error: dejar la cadena vacia para que el nombre sea invalido. */
    if (fgets(dest, tam, stdin) == NULL) {
        dest[0] = '\0';
        return;
    }

    {
        int largo; /* Largo actual de la cadena leida. */

        largo = (int)strlen(dest);

        /* Quitar todos los saltos de linea y retornos de carro finales. */
        while (largo > 0 && (dest[largo - 1] == '\n' || dest[largo - 1] == '\r')) {
            dest[largo - 1] = '\0';
            largo--;
        }
    }
}

/**
 * @brief Espera ENTER para congelar la pantalla final.
 *
 * Cumple el requisito "permanecera en esa pantalla hasta que el usuario
 * presione una tecla". Se usa ENTER (una linea) y no getch()/conio.h porque
 * conio.h no existe en Linux.
 *
 * @return No devuelve valor.
 * @note fflush(stdout) es necesario para que el mensaje aparezca antes de
 *       bloquearse esperando la entrada.
 */
static void esperar_tecla(void)
{
    char buf[16]; /* Buffer temporal para consumir la linea. */

    printf("\nPresione ENTER para finalizar...");
    fflush(stdout);
    leer_linea(buf, (int)sizeof(buf));
}

/**
 * @brief Ejecuta la demo completa de fin de partida.
 *
 * @param argc Cantidad de argumentos de linea de comandos.
 * @param argv Argumentos; si argc > 1, argv[1] es el nombre del ganador.
 * @return 0 si la demo termino normalmente; 1 si el nombre es invalido y no
 *         se pudo registrar la victoria.
 * @note Sin argumento, el nombre se pide por teclado. El programa debe
 *       ejecutarse desde la raiz para que exista data/ranking.dat.
 */
int main(int argc, char *argv[])
{
    Ranking ranking;                    /* Ranking cargado en memoria. */
    char ganador[RANKING_NOMBRE_MAX];   /* Nombre del ganador. */
    int rc;                             /* Codigo de retorno de ranking.c. */

    /* Paso 1: presentacion y reglas (en el juego final lo hace ui_cli). */
    printf("=== BATALLA NAVAL ===\n");
    printf("Reglas resumidas: tablero de %dx%d, %d barcos por jugador\n",
           CONFIG_FILAS, CONFIG_COLUMNAS, CONFIG_NUM_BARCOS);
    printf("de longitudes 5, 4, 3, 2 y 1. Gana quien hunde toda la flota rival.\n\n");

    /* Paso 2: nombre del ganador (argv[1] en pruebas, teclado en uso real). */
    if (argc > 1) {
        /* Copia acotada: nunca se escribe mas alla del buffer. */
        strncpy(ganador, argv[1], (int)sizeof(ganador) - 1);
        ganador[sizeof(ganador) - 1] = '\0';
    } else {
        printf("Ingrese el nombre del ganador de la partida: ");
        fflush(stdout);
        leer_linea(ganador, (int)sizeof(ganador));
    }
    printf("\nHola, %s. Gracias por jugar.\n", ganador);

    /* Paso 3: cargar el ranking binario previo. Un archivo inexistente
       no es fatal: es la primera partida y se arranca vacio. Un archivo
       ilegible o corrupto se informa y tambien se arranca vacio. */
    ranking_inicializar(&ranking);
    rc = ranking_cargar(&ranking, CONFIG_RANKING_RUTA);
    if (rc == RANKING_ERROR_ARCHIVO) {
        printf("(Aviso: no existia %s, se inicia un ranking vacio.)\n",
               CONFIG_RANKING_RUTA);
    } else if (rc == RANKING_ERROR_FORMATO) {
        printf("(Aviso: %s esta corrupto o es de otro formato; se inicia vacio.)\n",
               CONFIG_RANKING_RUTA);
        printf("(Si es el ranking.dat de texto viejo, borrelo: el nuevo es binario.)\n");
    } else if (rc != RANKING_OK) {
        printf("No se pudo leer el ranking: %s\n", ranking_mensaje_error(rc));
    }

    /* Paso 4: registrar la victoria. Si el nombre es invalido se informa
       el motivo y se sale con codigo 1 (no hay partida que registrar). */
    rc = ranking_registrar_victoria(&ranking, ganador);
    if (rc != RANKING_OK) {
        printf("No se pudo registrar la victoria: %s\n",
               ranking_mensaje_error(rc));
        esperar_tecla();
        return 1;
    }

    /* Paso 5: guardar el volcado binario. Si falla (permisos, disco,
       carpeta data/ inexistente) se avisa, pero se muestra el ranking en
       memoria para no perder la pantalla final. */
    rc = ranking_guardar(&ranking, CONFIG_RANKING_RUTA);
    if (rc != RANKING_OK) {
        printf("No se pudo guardar el ranking en %s: %s\n",
               CONFIG_RANKING_RUTA, ranking_mensaje_error(rc));
    } else {
        printf("Ranking binario guardado en %s.\n\n", CONFIG_RANKING_RUTA);
    }

    /* Paso 6: pantalla final y espera de ENTER. */
    ranking_mostrar(&ranking);
    esperar_tecla();
    return 0;
}
