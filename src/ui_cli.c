#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/ui_cli.h"

#define MAX_NICKNAME 50

enum Menu_principal {
    OPCION_PVP = 1,
    OPCION_PVIA = 2,
    MOSTRAR_RANKING = 3,
    MOSTRAR_REGLAMENTO = 4,
    SALIR = 0
};

char texto_por_defecto[] = "Ingrese una opción: ";

char *nombres_barcos[] = {
    "Velero (1 casilla)",
    "Lancha (2 casillas)",
    "Submarino (3 casillas)",
    "Buque (4 casillas)",
    "Portaviones (5 casillas)"
};

// limpia la pantalla de la consola detectando el sistema operativo
void limpiar_pantalla_cli(void){
    #if defined(_WIN32) && !defined(__CYGWIN__)
        system("cls");
    #else
        system("clear");
    #endif
}

void esperar_usuario(void){
    printf("<Presione 'enter' para continuar>\n");
    getchar();
}

void imprimir_linea(void){
    printf("--------------------------------------------------------------------\n");
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

void imprimir_posicion_invalida(void){
    imprimir_linea();
    printf("Posición inválida.\n");
    printf("Vuelva a intentarlo.\n");
    esperar_usuario();
}

// pide al usuario que ingrese una opción numérica, y consume el resto de caracteres (como letras y saltos de línea)
// si el usuario no ingresa un entero, retorna -1
int pedir_opcion_al_usuario(char texto[]) {
    
    int opcion;

    printf("%s", texto);
    
    if (scanf("%d", &opcion) != 1){ //comprueba que scanf haya leído un entero
        return -1;  // indica que la entrada no era un entero
    }

    // Consumir el resto de la línea
    int c;
    while ((c = getchar()) != '\n' && c != EOF){
    }

    return opcion;
}

// ingresa el nombre del jugador y reemplaza el salto de línea por un \0
void pedir_nombre_jugador(char jugador[]){

    printf("Ingrese el nombre del jugador: ");
    fgets(jugador, MAX_NICKNAME, stdin);
    jugador[strcspn(jugador, "\n")] = '\0';

}

// saluda al jugador indicado
void saludar_jugador(char jugador[]){
    printf("Bienvenido %s!\n", jugador);
}

// imprime un archivo de texto por pantalla
void mostrar_archivo_de_texto(const char *ruta_al_archivo){
    
    FILE *archivo = fopen(ruta_al_archivo, "r");

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo \"%s\"\n", ruta_al_archivo);
        return;
    }

    int c;

    while ((c = fgetc(archivo)) != EOF) {
        putchar(c);
    }

    fclose(archivo);
}

// pantalla inicial del juego, pide el nombre del Jugador y muestra el reglamento del juego
void mostrar_presentacion_y_reglamento(char jugador[], char ruta_al_archivo[]){
    limpiar_pantalla_cli();

    pedir_nombre_jugador(jugador);

    limpiar_pantalla_cli();

    imprimir_linea();
    saludar_jugador(jugador);

    imprimir_linea();
    mostrar_archivo_de_texto(ruta_al_archivo);
    imprimir_linea();

    esperar_usuario();    
}

void mostrar_menu_principal(void){
    
    imprimir_linea();
    imprimir_centrado("MENÚ PRINCIPAL", 68);
    printf("\n");
    imprimir_linea();
    printf("Seleccionar modo de juego:\n");
    printf("    1. Jugador vs Jugador\n");
    printf("    2. Jugador vs IA\n");
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
        limpiar_pantalla_cli();
        mostrar_menu_de_dificultad();
        
        opcion = pedir_opcion_al_usuario(texto_por_defecto);

        switch (opcion){
        
            case 1:
                *dificultad = NORMAL; // asigna la dificultad normal
                return 1;

            case 2:
                *dificultad = TORMENTA; // asigna el modo tormenta
                return 1;

            case 0: 
                return 0; // volver al menú anterior

            default:
                printf("Opción inválida\n");
                esperar_usuario();
                break;
        }
    }
}

// muestra el menú principal y permite navegar por él
// asigna el modo de juego y dificultad elegidos por el usuario y retorna 1
// retorna 0 en caso de que el usuario quiera salir del juego
int elegir_modo_de_juego(Partida *partida){

    int opcion_menu;

    for (;;){ // ejecuta el menú hasta que el usuario elija modo y dificultad, o salga del juego
        limpiar_pantalla_cli();
        mostrar_menu_principal();
        opcion_menu = pedir_opcion_al_usuario(texto_por_defecto);

        switch (opcion_menu){
            case OPCION_PVP:

                partida->modo_de_juego = PVP;    // asigna modo Jugador vs Jugador

                if (elegir_dificultad(&partida->dificultad) == 0){ // muestra el menú de dificultad
                    break;  // vuelve al menú principal 
                }

                return 1; // modo y dificultad asignados correctamente

            case OPCION_PVIA:
                
                partida->modo_de_juego = PVIA;   // asigna modo Jugador vs IA

                if (elegir_dificultad(&partida->dificultad) == 0){ // muestra el menú de dificultad
                    break;  // vuelve al menú principal
                }

                return 1; // modo y dificultad asignados correctamente

            case MOSTRAR_RANKING:
                limpiar_pantalla_cli();
                // placeholder
                printf("Jugador | Puntos\n");
                imprimir_linea();
                printf("Vicen   | 1.500.123\n");

                esperar_usuario();
                break;

            case MOSTRAR_REGLAMENTO:
                limpiar_pantalla_cli();
                
                mostrar_archivo_de_texto("files/Reglamento.txt");
                
                esperar_usuario();
                break;

            case SALIR:
                printf("Saliendo...\n");
                return 0;
            
            default:
                printf("Opción inválida.\n");
                esperar_usuario();
                break;
        }
    } 
}


int validar_entrada_fila_y_columna(int fila){
    // devuelve 1 si la fila/columna está dentro del rango del tablero
    if (fila >= 0 && fila <= 9){
        return 1;
    }
    return 0;
}

void solicitar_entrada_de_fila(int *fila){

    int opcion;
    
    do {
        // pide la fila al usuario
        opcion = pedir_opcion_al_usuario("Fila: ");
    
        opcion--;
    
        // si no es válida le avisa al usuario y vuelve a preguntar
        if (validar_entrada_fila_y_columna(opcion) == 0){
            printf("Fila fuera de rango.\n");
        }
        
    } while (validar_entrada_fila_y_columna(opcion) == 0);
    
    // si es válida la guarda en el puntero
    *fila = opcion;
}

void solicitar_entrada_de_columna(int *columna){
    
    int opcion;

    do {
        // pide la columna al usuario
        printf("Columna: ");
        opcion = getchar(); // toma el primer caracter ingresado
    
        // consume el resto de la línea
        int c;
        while ((c = getchar()) != '\n' && c != EOF){
        }
    
        // lee si se ingresó una columna dentro del rango del tablero 
        // y convierte la columna al valor numérico correspondiente 
        if (opcion >= 'A' && opcion <= 'J'){
            opcion -= 65;
        }
        else if (opcion >= 'a' && opcion <= 'j'){
            opcion -= 97;
        }

        // si no es válida le avisa al usuario y vuelve a preguntar
        if (validar_entrada_fila_y_columna(opcion) == 0){
            printf("Columna fuera de rango.\n");
        }

    } while (validar_entrada_fila_y_columna(opcion) == 0);
    
    // si es válida la guarda en el puntero
    *columna = opcion;
}

void solicitar_orientacion(Orientacion *orientacion){

    // le pregunta al usuario hasta que ingrese una opción válida
    for (;;){
        printf("Ingrese la orientación: \n");
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
                printf("Opción no válida.\n");
                break;
        }
    }
}

void pedir_posicion_del_barco(Jugador *jugador, int tamanio){
    
    printf("Barco a colocar: %s\n", nombres_barcos[tamanio-1]);
    imprimir_linea();

    solicitar_entrada_de_fila(&jugador->barco[tamanio].fila);
    solicitar_entrada_de_columna(&jugador->barco[tamanio].columna);
    solicitar_orientacion(&jugador->barco[tamanio].orientacion);
}

void mostrar_tableros(Jugador jugador){

    // leyenda
    imprimir_linea();
    printf("  '~': Agua");
    printf("    '#': Barco");
    printf("    'X': Impacto");
    printf("    'O': Disparo al agua\n");
    
    imprimir_linea();
    printf("\n");

    // mostrar encabezados
    imprimir_centrado("-------MI FLOTA-------", 31);
    printf("|");
    imprimir_centrado("---DISPAROS REALIZADOS---", 31);
    
    printf("\n");
    imprimir_centrado("|", 63);
    printf("\n       ");

    // mostrar nombres de columnas
    for (int i = 0; i < COL; i++){
        printf("%c ", 'A' + i);
    }

    printf("    |       ");

    for (int i = 0; i < COL; i++){
        printf("%c ", 'A' + i);
    }

    printf("\n");
    
    // mostrar tableros
    for (int i = 0; i < FIL; i++){
        
        // TABLERO DE BARCOS

        printf("    %2d ", i + 1);

        for (int k = 0; k < COL; k++){
            printf("%c ", jugador.tableros.tablero[i][k]);
        }

        printf("    |    ");

        // TABLERO DE DISPAROS

        printf("%2d ", i + 1);

        for (int k = 0; k < COL; k++){
            printf("%c ", jugador.tableros.tablero_disparos[i][k]);
        }
    
        printf("\n");
    }
}

void mostrar_pantalla_de_colocacion_de_barcos(Jugador *jugador, int tamanio){
    
    limpiar_pantalla_cli();
    imprimir_linea();

    printf("Jugador: %s\n", jugador->nombre);

    mostrar_tableros(*jugador);

    printf("\n");
    imprimir_linea();

    pedir_posicion_del_barco(jugador, tamanio);  
}

// debería retornar fila y columna en dos variables distintas (punteros)
void mostrar_pantalla_de_ataque(Jugador *jugador, const int turno, int *fila, int *columna){
    
    limpiar_pantalla_cli();
    imprimir_linea();

    printf("Jugador: %s\n", jugador->nombre);
    imprimir_linea();
    printf("\n");
    
    printf("Turno N° %d\n", turno);
    imprimir_linea();
    printf("\n");

    mostrar_tableros(*jugador);

    printf("\n");
    imprimir_linea();

    solicitar_entrada_de_fila(fila);
    solicitar_entrada_de_columna(columna);
}

void mostrar_pantalla_cambio_de_turno(const Jugador jugador, const int turno){

    limpiar_pantalla_cli();
    imprimir_linea();

    printf("TURNO DEL JUGADOR %s\n", jugador.nombre);
    imprimir_linea();
    printf("TURNO N° %d\n", turno);

    imprimir_linea();
    esperar_usuario();
}

void mostrar_pantalla_de_victoria(Partida *partida){
    
    limpiar_pantalla_cli();
    imprimir_linea();

    printf("ENOURABUENA %s!\n", partida->ganador == 1           // si gano el JUG 1
                                    ? partida->jugador1.nombre  // felicita al JUG 1
                                    : partida->jugador2.nombre  // si no felicita al JUG 2
    );
    
    printf("HAS DERROTADO A %s\n", partida->ganador == 1        // si gano el JUG 1
                                    ? partida->jugador2.nombre  // perdedor = JUG 2
                                    : partida->jugador1.nombre  // si no perdedor = JUG 1
    );

    imprimir_linea();
    printf("La partida ha durado %d turnos.\n", partida->cantidad_turnos);
    imprimir_linea();
    
    esperar_usuario();
}