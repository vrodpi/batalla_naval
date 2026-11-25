#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "../include/ui_cli.h"

#define ANCHO_CONSOLA 100

static char texto_por_defecto[] = "Ingrese una opción: ";

static char *nombres_barcos[] = {
    "Lancha (1 casilla)",
    "Velero (2 casillas)",
    "Submarino (3 casillas)",
    "Buque (4 casillas)",
    "Portaviones (5 casillas)"
};

static char *tipos_de_partidas[] = {
    "JUGADOR VS JUGADOR - MODO NORMAL",
    "JUGADOR VS JUGADOR - MODO TORMENTA",
    "JUGADOR VS IA - MODO NORMAL",
    "JUGADOR VS IA - MODO TORMENTA"
};

static void consumir_buffer(void){
   
    int c;
    while ((c = getchar()) != '\n' && c != EOF){
    }
}

static void imprimir_linea(void){
    for (int i = 0; i < ANCHO_CONSOLA; i++){
        printf("-");
    }
    printf("\n");
}

static void esperar_usuario(void){
    imprimir_linea();

    printf("<Presione 'enter' para continuar>\n");
    
    consumir_buffer();
}

static void imprimir_centrado(const char *texto, int ancho){
    // cuenta la cantidad de caracteres del string
    int longitud = strlen(texto);
    // cuenta los espacios restantes
    int espacios = ancho - longitud;

    if (espacios <= 0){
        printf("%s", texto);
    } 
    // si quedan espacios los reparte equitativamente a la izq y der
    else {
        int izquierda = espacios / 2;
        int derecha = espacios - izquierda;

        printf("%*s%s%*s", izquierda, "", texto, derecha, "");
    }
}

void ui_confirmacion_barco_colocado(int caso){
    
    switch (caso){
        case 1:
            printf("Barco ingresado correctamente\n");
            break;;
        case -1:
            printf("El barco no entra en los límites del tablero\n");
            break;;
        case -2:
            printf("El barco debe estar a al menos una casilla de distancia de otro barco\n");
            break;;
        }
    esperar_usuario();
}

void ui_mensajes_menu(UI_Mensaje_menu opcion){
    switch (opcion){
    case UI_MENSAJE_SALIR:
        printf("Saliendo...\n");
        break;
    case UI_MENSAJE_DEFAULT:
        printf("<Opción inválida>\n");
        esperar_usuario();
    }
}

// pide al usuario que ingrese una opción numérica, 
// si el usuario no ingresa una entrada válida, retorna 0
int ui_pedir_opcion_al_usuario(const char *texto, int *opcion){
    char linea[128];
    char *fin;
    long valor;

    for (;;){
        imprimir_linea();
        printf("%s", texto);

        if (fgets(linea, sizeof linea, stdin) == NULL){
            return 0;  // EOF o error de lectura
        }

        /* Si la línea excede el búfer, descartar lo que quedó y reintentar. */
        if (strchr(linea, '\n') == NULL){
            consumir_buffer();
            printf("<La entrada es demasiado larga>\n");
            continue;
        }

        errno = 0;
        // transforma los números de linea en un long int y guarda el resto en fin
        valor = strtol(linea, &fin, 10); 

        // si no hay números en linea le avisa al usuario
        if (fin == linea){ 
            printf("<Ingrese un número entero>\n");
            continue;
        }

        // quita todos los espacios de fin
        while (isspace((unsigned char)*fin)){
            fin++;
        }

        // si quedan caracteres en fin
        if (*fin != '\0' ||     
            // o el número no puede representarse como int
            errno == ERANGE || 
            valor < INT_MIN || 
            valor > INT_MAX 
        ){
            printf("<Ingrese un número entero válido>\n");
            continue;
        }

        // transforma valor en int 
        *opcion = (int)valor;
        return 1;  // entrada válida
    }
}

/* Lee un string ingresado por el usuario y comprueba que cumpla las condiciones
 * de un nombre de usuario válido.
 * Retorna 1 si el nombre es válido y 0 si no lo es
*/
static int leer_nombre_de_usuario(char *jugador){
    
    char *salto;
    
    // comprueba que fgets tenga una salida válida
    if (fgets(jugador, MAX_NOMBRE_DE_USUARIO, stdin) == NULL){
        jugador[0] = '\0'; // si no, vacía el nombre del jugador
        return 0;  // fin de entrada o error
    }

    // busca si hay un salto de linea en el nombre
    salto = strchr(jugador, '\n');

    // si lo encunetra, lo reemplaza por un fin de string
    if (salto != NULL){
        *salto = '\0';
    } 
    // si no lo encuentra, el nombre sobrepasa el tamaño del arreglo
    else {
        /* La línea puede exceder el búfer: descartar lo que sobra. */
        consumir_buffer();
    }

    // si el jugador ingresa un nombre vacío, retorna 0
    if (jugador[0] == '\0'){
        return 0;
    }

    // si el nombre es válido, retorna 1
    return 1;
}

// ingresa el nombre del jugador y reemplaza el salto de línea por un \0
void ui_pedir_nombre_jugador(char jugador[], const int numero){

    for (;;){
        imprimir_linea();
    
        printf("Ingrese el nombre del Jugador %d: ", numero);

        // si lee un nombre válido, retorna
        if (leer_nombre_de_usuario(jugador) == 1){
            return;
        }
    
        printf("\nEl nombre no puede estar vacío.\n");
    }
}

// imprime un archivo de texto por pantalla
void ui_mostrar_reglamento(const char *ruta_al_archivo){
    
    system("clear");

    imprimir_linea();

    FILE *archivo = fopen(ruta_al_archivo, "r");

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo \"%s\"\n", ruta_al_archivo);
        return;
    }

    int c;

    while ((c = fgetc(archivo)) != EOF){
        putchar(c);
    }

    fclose(archivo);

    esperar_usuario();
}

// pantalla inicial del juego, pide el nombre del Jugador y muestra el reglamento del juego
void ui_mostrar_presentacion_y_reglamento(char jugador[], char ruta_al_archivo[]){
    system("clear");

    ui_pedir_nombre_jugador(jugador, 1);

    system("clear");

    imprimir_linea();
    printf("Bienvenido %s!\n", jugador);
    esperar_usuario();

    imprimir_linea();
    ui_mostrar_reglamento(ruta_al_archivo);
}

void ui_mostrar_menu_principal(void){
    
    system("clear");

    imprimir_linea();
    imprimir_centrado("MENÚ PRINCIPAL", ANCHO_CONSOLA);
    printf("\n");
    imprimir_linea();
    printf("Seleccionar modo de juego:\n");
    printf("    1. Jugador vs Jugador.\n");
    printf("    2. Jugador vs IA.\n");
    imprimir_linea();
    printf("    3. Mostrar ranking.\n");
    printf("    4. Mostrar reglamento.\n");
    printf("    0. Salir del juego.\n");
}

static void mostrar_menu_de_dificultad(void){

    imprimir_linea();
    printf("Seleccione el nivel de dificultad:\n");
    printf("    1. Modo Normal.\n");
    printf("    2. Modo Tormenta.\n");
    printf("    0. Volver al menú principal.\n");
}

// Muestra el menú de selección de dificultad
// retorna 1 si se eligió una opción de dificultad y 0 para volver al menú anterior
int ui_elegir_dificultad(Dificultad *dificultad){
    
    int opcion;

    for (;;){ // ejecuta el menú hasta que se llegue a un return
        system("clear");
        mostrar_menu_de_dificultad();
        
        ui_pedir_opcion_al_usuario(texto_por_defecto, &opcion);

        switch (opcion){
        
            case 1:
                *dificultad = NORMAL; // asigna la dificultad normal
                system("clear");
                return 1;

            case 2:
                *dificultad = TORMENTA; // asigna el modo tormenta
                system("clear");
                return 1;

            case 0: 
                return 0; // volver al menú anterior
                
            default:
                imprimir_linea();
                printf("Opción inválida\n");
                esperar_usuario();
                break;
        }
    }
}


// devuelve 1 si la fila/columna está dentro del rango del tablero
static int validar_entrada_fila_y_columna(const int opcion){
    if (opcion >= 0 && opcion <= 9){
        return 1;
    }
    return 0;
}

static void solicitar_entrada_de_fila(int *fila){

    int opcion;
    
    do {
        
        // pide la columna al usuario
        ui_pedir_opcion_al_usuario("Fila: ", &opcion); // toma el primer caracter ingresado
        
        // si la opcion no era un int
        if (opcion == -1){
            printf("<Ingrese un número entero>\n");
            imprimir_linea();

            // salta hasta el principio del próximo loop
            continue;
        }

        opcion--;
    
        // si no es válida le avisa al usuario y vuelve a preguntar
        if (validar_entrada_fila_y_columna(opcion) == 0){
            printf("<Fila fuera de rango>\n");
            imprimir_linea();
        }

    } while (validar_entrada_fila_y_columna(opcion) == 0);

    *fila = opcion;
}


static void solicitar_entrada_de_columna(int *columna){
    
    char entrada[10];
    char *salto;
    char letra;

    for (;;){
        
        // pide la columna al usuario
        printf("Columna: ");

        if (fgets(entrada, sizeof(entrada), stdin) == NULL){
            entrada[0] = '\0';
            return;
        }

        // busca si hay un salto de línea en la entrada
        salto = strchr(entrada, '\n');

        if (salto != NULL){
            // si lo encuentra lo reemplaza por \0
            *salto = '\0'; 
        }
        else {
            // si no lo encuentra, consume el resto de la línea
            consumir_buffer();
        }

        // si el usuario ingresó más de un caracter
        if (strlen(entrada) != 1){
            printf("<Ingrese una sola letra entre A-J>\n");
            imprimir_linea();

            // salta al inicio del próximo bucle
            continue;
        }

        // guarda la primera letra 
        letra = entrada[0];

        // verifica que la columna ingresada este dentro del rango 
        // y la convierte al valor numérico correspondiente 
        if (letra >= 'A' && letra <= 'J'){
            letra -= 'A';
        }
        else if (letra >= 'a' && letra <= 'j'){
            letra -= 'a';
        }
            
        // si no es válida le avisa al usuario y vuelve a preguntar
        if (validar_entrada_fila_y_columna(letra) == 0){
            printf("<Columna fuera de rango>\n");
            imprimir_linea();
        }
        else {
            // si es válida la guarda en el puntero y retorna
            *columna = letra;
            return;
        }
    }     
}

static void solicitar_orientacion(Orientacion *orientacion){

    // le pregunta al usuario hasta que ingrese una opción válida
    for (;;){
        printf("Seleccione la orientación: \n");
        printf("    1. Horizontal\n");
        printf("    2. Vertical\n");
        
        int opcion; 
        ui_pedir_opcion_al_usuario(texto_por_defecto, &opcion);

        switch (opcion){
            case 1:
                *orientacion = horizontal;
                return;
            
            case 2:
                *orientacion = vertical;
                return;
            
            default:
                printf("<Opción no válida>\n");
                imprimir_linea();
                break;
        }
    }
}

void ui_pedir_posicion_del_barco(Jugador *jugador, const int indice){
    
    printf("Barco a colocar: %s\n", nombres_barcos[indice]);

    solicitar_entrada_de_fila(&jugador->barco[indice].fila);
    solicitar_entrada_de_columna(&jugador->barco[indice].columna);

    // si es una lancha, no pide la orientación
    if (jugador->barco[indice].tamanio != 1){
        solicitar_orientacion(&jugador->barco[indice].orientacion);
    }
    else {
        jugador->barco[indice].orientacion = horizontal;
    }
}

void ui_pedir_coordenadas_de_ataque(int *fila, int *columna){
      
    solicitar_entrada_de_fila(fila);
    solicitar_entrada_de_columna(columna);
}

/* Muestra un encabezado con el Modo de Juego y la Dificultad de la partida,
 * el nombre del Jugador activo
 * el Turno actual
 * y los Tableros del Jugador activo 
*/
void ui_mostrar_tableros(const Jugador jugador, const int turno, const int tipo_de_partida){

    system("clear");

    imprimir_linea();

    imprimir_centrado(tipos_de_partidas[tipo_de_partida], ANCHO_CONSOLA);
    printf("\n");

    imprimir_linea();

    if ((tipo_de_partida == 2 || tipo_de_partida == 3) && // Si es partida vs IA
        turno % 2 == 0 && turno != 0){                    // y el turno es par
        
            printf("TURNO DE SKYNET\n");
    }
    else {
        printf("JUGADOR: %s\n", jugador.nombre);
    }
    
    // no muestra el turno durante la colocación de barcos
    if (turno != 0){
        imprimir_linea();
        printf("Turno N° %d\n", turno);
    }
    imprimir_linea();

    // LEYENDA
    // Sangría en modo tormenta
    if (tipo_de_partida == 1 || tipo_de_partida == 3){
        printf("%5c", ' ');
    }
    // Sangría en modo normal
    else {
        printf("%16c", ' ');
    }

    printf("   '%c': Agua", CASILLA_AGUA);
    printf("   '%c': Barco", CASILLA_BARCO);
    printf("   '%c': Impacto", CASILLA_IMPACTO);
    // Solo se muestra en el modo tormenta
    if (tipo_de_partida == 1 || tipo_de_partida == 3){
        printf("   '%c': Impacto tormenta", CASILLA_TORMENTA); 
    }
    printf("   '%c': Disparo al agua\n", CASILLA_DISP_AGUA);
    
    imprimir_linea();
    printf("\n");

    // mostrar encabezados
    imprimir_centrado("-------MI FLOTA-------", (ANCHO_CONSOLA - 1) / 2);
    printf("|");
    imprimir_centrado("---DISPAROS REALIZADOS---", ANCHO_CONSOLA / 2);
    
    printf("\n");
    imprimir_centrado("|", ANCHO_CONSOLA);
    printf("\n%16c", ' '); 

    // mostrar nombres de columnas
    for (int i = 0; i < COL; i++){
        printf("%c ", 'A' + i);
    }

    printf("%14c %15c", '|', ' ');

    for (int i = 0; i < COL; i++){
        printf("%c ", 'A' + i);
    }

    printf("\n");
    
    // mostrar tableros
    for (int i = 0; i < FIL; i++){
        
        // TABLERO DE BARCOS

        printf("%15d ", i + 1); // número de fila

        for (int k = 0; k < COL; k++){
            printf("%c ", jugador.tableros.tablero[i][k]);
        }

        printf("%14c", '|');

        // TABLERO DE DISPAROS

        printf("%15d ", i + 1); // número de fila

        for (int k = 0; k < COL; k++){
            printf("%c ", jugador.tableros.tablero_disparos[i][k]);
        }
    
        printf("\n");
    }

    printf("\n");
}

void ui_mostrar_pantalla_cambio_de_turno(const Jugador jugador, const int turno){

    system("clear");
    imprimir_linea();

    if (turno == 0){
        imprimir_centrado("COLOCACIÓN DE BARCOS", ANCHO_CONSOLA);
        printf("\n");
        imprimir_linea();
        
        printf("TURNO DEL JUGADOR %s\n", jugador.nombre);
    }
    else {
        printf("TURNO DEL JUGADOR %s\n", jugador.nombre);
        imprimir_linea();

        printf("TURNO N° %d\n", turno);
    }

    esperar_usuario();
}

void ui_mostrar_pantalla_de_victoria(Partida *partida){
    
    system("clear");
    imprimir_linea();

    if (partida->modo_de_juego == PVIA && partida->ganador == 2) {

        printf("HAS SIDO DERROTADO POR SKYNET\n");

    } else {

        printf("ENHORABUENA %s!\n", partida->ganador == 1            // si gano el JUG 1
                                        ? partida->jugador1.nombre  // felicita al JUG 1
                                        : partida->jugador2.nombre  // si no felicita al JUG 2
        );

        printf("HAS DERROTADO A %s\n", partida->ganador == 1         // si gano el JUG 1
                                        ? partida->jugador2.nombre  // perdedor = JUG 2
                                        : partida->jugador1.nombre  // si no perdedor = JUG 1
        );
    }

    imprimir_linea();
    printf("La partida ha durado %d turnos.\n", partida->cantidad_turnos);
    
    esperar_usuario();
}

void ui_resultado_del_ataque(const Rdo_Disparo disparo){
    switch (disparo){

    case disp_agua:
        printf("AGUA\n");
        break;
    case disp_impacto:
        printf("IMPACTO\n");
        break;
    case disp_repetido:
        printf("DISPARO REPETIDO: VUELVA A INTENTARLO\n");
        break;
    
    default:
        break;
    }

    esperar_usuario();
}

void ui_mostrar_mensaje_tormenta(Jugador *defensor, int Fil_afectada, int Col_afectada){
    imprimir_linea();
    printf("\n");
    imprimir_centrado("-------TORMENTA ACTIVADA-------", ANCHO_CONSOLA);
    printf("\n");
    
    imprimir_linea();
    
    printf("Un Rayo Impactó sobre la flota del Jugador %s\n", defensor->nombre);
    imprimir_linea();

    printf("Coordenadas de Impacto: \n");
    printf("    Fila %d \n", Fil_afectada + 1);
    printf("    Columna %c \n", Col_afectada + 65);

    esperar_usuario();
}

// Mensajes IA
void ui_mensaje_disparo_ia(const char *nombre, const int fila, const int columna){
    printf("%s Dispara a las Coordenadas:\n", nombre);
    printf("    Fila %d\n", fila + 1);
    printf("    Columna %c\n", columna + 'A');
    imprimir_linea();
}

void ui_mensaje_error_ia(UI_Mensajes_Error_IA mensaje){
    if (mensaje == UI_MENSAJE_FLOTA){
        printf("No se pudo colocar la flota de la IA.\n");
    }
    else {
        printf("La IA no pudo elegir un disparo.\n");
    }
    esperar_usuario();
}

// Ranking
static void mostrar_menu_ranking(void){

    imprimir_linea();
    imprimir_centrado("RANKING", ANCHO_CONSOLA);
    printf("\n");

    imprimir_linea();
    printf("Seleccione una opción:\n");
    printf("    1. Mostrar todo el ranking.\n");
    printf("    2. Buscar usuario por nombre.\n");
    printf("    0. Volver al menú principal.\n");
}


// pide al usuario que ingrese un nombre y busca coincidencias en el ranking
// retorna -1 en caso de que no haya coincidencias
static int buscar_registro_ranking(const Ranking ranking, char jugador[]){

    for (;;){
        imprimir_linea();
    
        printf("Ingrese el nombre del Jugador: ");

        // si el nombre no es válido, salta al próximo bucle
        if (leer_nombre_de_usuario(jugador) == 0){
            printf("\nEl nombre no puede estar vacío.\n");
            continue;
        }

        return ranking_buscar(&ranking, jugador);
    }
}

static void mostrar_registro_ranking(const Ranking ranking, const int indice, const UI_Menu_ranking opcion){
    
    // Muestra el encabezado
    printf("| Posición | %-24s| Victorias |\n", "Nombre");
    imprimir_linea();

    // Muestra todos los registros
    if (opcion == UI_RANKING_COMPLETO){
        for (int i = 0; i < indice; i++){
            printf("|%5d     | %-24s|%6d     |\n", 
                i + 1, 
                ranking.entradas[i].nombre, 
                ranking.entradas[i].victorias
            );
        }
    }
    // Muestra un solo registro
    else if (opcion == UI_RANKING_REGISTRO){
        printf("|%5d     | %-24s|%6d     |\n", 
            indice + 1, 
            ranking.entradas[indice].nombre, 
            ranking.entradas[indice].victorias
        );
    }

    esperar_usuario();
}

void ui_mostrar_ranking(const Ranking ranking){

    
    if (ranking.cantidad < 1){
        system("clear");

        imprimir_linea();
        printf("El ránking está vacío.\n");
        esperar_usuario();
        return;
    }

    
    for (;;){
        system("clear");
        mostrar_menu_ranking();
        
        int opcion;
        ui_pedir_opcion_al_usuario(texto_por_defecto, &opcion);
        
        switch (opcion){
            case UI_RANKING_COMPLETO:
                system("clear");
            
                mostrar_registro_ranking(ranking, ranking.cantidad, UI_RANKING_COMPLETO);            
            
                return;
            
            case UI_RANKING_REGISTRO:
                char jugador[MAX_NOMBRE_DE_USUARIO];
                int indice = buscar_registro_ranking(ranking, jugador);

                if (indice < 0){
                    printf("No se encontraron coincidencias.\n");
                    esperar_usuario();
                    return;
                }
            
                mostrar_registro_ranking(ranking, indice, UI_RANKING_REGISTRO);
            
                return;

            case UI_RANKING_SALIR:
                return;

            default:
                printf("Opción no reconocida.\n");
                break;
        }
    }
}
