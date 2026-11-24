#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/ui_cli.h"

#define ANCHO_CONSOLA 100

char texto_por_defecto[] = "Ingrese una opción: ";

char *nombres_barcos[] = {
    "Lancha (1 casilla)",
    "Velero (2 casillas)",
    "Submarino (3 casillas)",
    "Buque (4 casillas)",
    "Portaviones (5 casillas)"
};

char *tipos_de_partidas[] = {
    "JUGADOR VS JUGADOR - MODO NORMAL",
    "JUGADOR VS JUGADOR - MODO TORMENTA",
    "JUGADOR VS IA - MODO NORMAL",
    "JUGADOR VS IA - MODO TORMENTA"
};

void consumir_buffer(void){
   
    int c;
    while ((c = getchar()) != '\n' && c != EOF){
    }
}

void esperar_usuario(void){
    imprimir_linea();

    printf("<Presione 'enter' para continuar>\n");
    
    consumir_buffer();
}

void imprimir_linea(void){
    for (int i = 0; i < ANCHO_CONSOLA; i++){
        printf("-");
    }
    printf("\n");
}

void imprimir_centrado(const char *texto, int ancho){
    int longitud = strlen(texto);
    int espacios = ancho - longitud;

    if (espacios <= 0) {
        printf("%s", texto);
    } else {
        int izquierda = espacios / 2;
        int derecha = espacios - izquierda;

        printf("%*s%s%*s", izquierda, "", texto, derecha, "");
    }
}

void ui_confirmacion_barco_colocado(int caso){
    
    switch (caso){
        case 1:
            imprimir_linea();
            printf("Barco ingresado correctamente\n");
            esperar_usuario();
            return;
        case -1:
            imprimir_linea();
            printf("El barco no entra en los límites del tablero\n");
            esperar_usuario();
            return;
        case -2:
            imprimir_linea();
            printf("El barco debe estar a al menos una casilla de distancia de otro barco\n");
            esperar_usuario();
            return;
    }
}

// pide al usuario que ingrese una opción numérica, y consume el resto de caracteres (como letras y saltos de línea)
// si el usuario no ingresa un entero, retorna -1
int pedir_opcion_al_usuario(char texto[]){
    
    int opcion;

    printf("%s", texto);
      
    if (scanf("%d", &opcion) != 1){ //comprueba que scanf haya leído un entero
    
        consumir_buffer();

        return -1;  // indica que la entrada no era un entero
    }
    
    // Consumir el resto de la línea
    consumir_buffer();

    return opcion;
}

static int leer_nombre_de_usuario(char jugador[]){
    
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

    return 1;
}

// ingresa el nombre del jugador y reemplaza el salto de línea por un \0
void pedir_nombre_jugador(char jugador[], const int numero){

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

// saluda al jugador indicado
void saludar_jugador(char jugador[]){
    printf("Bienvenido %s!\n", jugador);
}

// imprime un archivo de texto por pantalla
void ui_mostrar_reglamento(const char *ruta_al_archivo){
    
    system("clear");

    FILE *archivo = fopen(ruta_al_archivo, "r");

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo \"%s\"\n", ruta_al_archivo);
        return;
    }

    imprimir_linea();

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

    pedir_nombre_jugador(jugador, 1);

    system("clear");

    imprimir_linea();
    saludar_jugador(jugador);

    esperar_usuario();

    imprimir_linea();
    ui_mostrar_reglamento(ruta_al_archivo);
}

void mostrar_menu_principal(void){
    
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
    imprimir_linea();
}

void mostrar_menu_de_dificultad(void){

    imprimir_linea();
    printf("Seleccione el nivel de dificultad:\n");
    printf("    1. Modo Normal.\n");
    printf("    2. Modo Tormenta.\n");
    printf("    0. Volver al menú principal.\n");
    imprimir_linea();
}

// Muestra el menú de selección de dificultad
// retorna 1 si se eligió una opción de dificultad y 0 para volver al menú anterior
int elegir_dificultad(Dificultad *dificultad){
    
    int opcion;

    for (;;){ // ejecuta el menú hasta que se llegue a un return
        system("clear");
        mostrar_menu_de_dificultad();
        
        opcion = pedir_opcion_al_usuario(texto_por_defecto);

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

// muestra el menú principal y permite navegar por él
// asigna el modo de juego y dificultad elegidos por el usuario y retorna 1
// retorna 0 en caso de que el usuario quiera salir del juego

static int validar_entrada_fila_y_columna(const int opcion){
    // devuelve 1 si la fila/columna está dentro del rango del tablero
    if (opcion >= 0 && opcion <= 9){
        return 1;
    }
    return 0;
}

void solicitar_entrada_de_fila(int *fila){

    int opcion;
    
    do {
        
        // pide la columna al usuario
        opcion = pedir_opcion_al_usuario("Fila: "); // toma el primer caracter ingresado
        
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


void solicitar_entrada_de_columna(int *columna){
    
    char entrada[10];
    char *salto;
    char letra;

    for (;;) {
        
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
        // y la convierte al valor correspondiente 
        if (letra >= 'A' && letra <= 'J'){
            letra -= 65;
        }
        else if (letra >= 'a' && letra <= 'j'){
            letra -= 97;
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

void solicitar_orientacion(Orientacion *orientacion){

    // le pregunta al usuario hasta que ingrese una opción válida
    for (;;){
        printf("Seleccione la orientación: \n");
        printf("    1. Horizontal\n");
        printf("    2. Vertical\n");
        
        int opcion = pedir_opcion_al_usuario(texto_por_defecto);

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

void pedir_posicion_del_barco(Jugador *jugador, const int indice){
    
    printf("Barco a colocar: %s\n", nombres_barcos[indice]);
    imprimir_linea();

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

void ui_mostrar_tableros(const Jugador jugador, const int turno, const int tipo_de_partida){

    system("clear");

    imprimir_linea();

    imprimir_centrado(tipos_de_partidas[tipo_de_partida], ANCHO_CONSOLA);
    printf("\n");

    imprimir_linea();

    printf("JUGADOR: %s\n", jugador.nombre);
    
    // no muestra el turno durante la colocación de barcos
    if (turno != 0){
        imprimir_linea();
        printf("Turno N° %d\n", turno);
    }

    // leyenda
    imprimir_linea();
    printf("        '%c': Agua", CASILLA_AGUA);
    printf("   '%c': Barco", CASILLA_BARCO);
    printf("   '%c': Impacto", CASILLA_IMPACTO);
    // SOLO EN EL MODO TORMENTA
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
    printf("\n                ");

    // mostrar nombres de columnas
    for (int i = 0; i < COL; i++){
        printf("%c ", 'A' + i);
    }

    printf("             |                ");

    for (int i = 0; i < COL; i++){
        printf("%c ", 'A' + i);
    }

    printf("\n");
    
    // mostrar tableros
    for (int i = 0; i < FIL; i++){
        
        // TABLERO DE BARCOS

        printf("%15d ", i + 1);

        for (int k = 0; k < COL; k++){
            printf("%c ", jugador.tableros.tablero[i][k]);
        }

        printf("             |");

        // TABLERO DE DISPAROS

        printf("%15d ", i + 1);

        for (int k = 0; k < COL; k++){
            printf("%c ", jugador.tableros.tablero_disparos[i][k]);
        }
    
        printf("\n");
    }

    printf("\n");

    imprimir_linea();
}

void pedir_coordenadas_de_ataque(int *fila, int *columna){
      
    solicitar_entrada_de_fila(fila);
    solicitar_entrada_de_columna(columna);
}

void mostrar_pantalla_cambio_de_turno(const Jugador jugador, const int turno){

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

void mostrar_pantalla_de_victoria(Partida *partida){
    
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

void resultado_del_ataque(const Rdo_Disparo disparo){
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

void mostrar_mensaje_tormenta (Jugador *defensor, int Fil_afectada, int Col_afectada) {
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

void mostrar_menu_ranking(void){

    imprimir_linea();
    imprimir_centrado("RANKING", ANCHO_CONSOLA);
    printf("\n");

    imprimir_linea();
    printf("Seleccione una opción:\n");
    printf("    1. Mostrar todo el ranking.\n");
    printf("    2. Buscar usuario por nombre.\n");
    printf("    0. Volver al menú principal.\n");
    imprimir_linea();
}


// pide al usuario que ingrese un nombre y busca coincidencias en el ranking
// retorna -1 en caso de que no haya coincidencias
int buscar_registro_ranking(const Ranking ranking, char jugador[]){

    for (;;){
        imprimir_linea();
    
        printf("Ingrese el nombre del Jugador: ");

        printf("BB\n");
        // si el nombre no es válido, salta al próximo bucle
        if (leer_nombre_de_usuario(jugador) == 0){
            printf("\nEl nombre no puede estar vacío.\n");
            continue;
        }

        printf("CC\n");
        return ranking_buscar(&ranking, jugador);
    }
}

void mostrar_registro_ranking(const Ranking ranking, const int indice){
    printf("|%-10d|%-25s|%-11d|", 
        indice + 1, 
        ranking.entradas->nombre, 
        ranking.entradas->victorias
    );
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
        
        int opcion = pedir_opcion_al_usuario(texto_por_defecto);
        
        switch (opcion){
            case 1:
                system("clear");
            
                printf("| Posición |%-25s| Victorias |\n", "Nombre");
                
                imprimir_linea();
                
                for (int i = 0; i < ranking.cantidad; i++){
                    mostrar_registro_ranking(ranking, i);
                }
                printf("\n");
            
                esperar_usuario();
            
                return;
            
            case 2:
                char *jugador = NULL;
                int indice = buscar_registro_ranking(ranking, jugador);
                printf("A\n");

                if (indice < 0){
                    printf("No se encontraron coincidencias.\n");
                    esperar_usuario();
                    return;
                }
            
                printf("| Posición |%-25s| Victorias |\n", "Nombre");
                mostrar_registro_ranking(ranking, indice);
            
                esperar_usuario();
                return;

            case 0:
                return;

            default:
                printf("Opción no reconocida.\n");
                break;
        }
    }
}