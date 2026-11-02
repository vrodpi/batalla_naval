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

enum Columnas {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J
};

char *nombres_barcos[] = {
    "Velero",
    "Lancha",
    "Submarino",
    "Buque",
    "Portaviones"
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
    printf("-------------------------------------------------------------------\n");
}

// pide al usuario que ingrese una opción numérica, y consume el resto de caracteres (como letras y saltos de línea)
// si el usuario no ingresa un entero, retorna -1
int pedir_opcion_al_usuario(void) {
    
    int opcion;

    printf("Ingrese una opción: ");
    
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
    printf("MENÚ PRINCIPAL\n");
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
        
        opcion = pedir_opcion_al_usuario();

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
int elegir_modo_de_juego(Modo_de_juego *modo, Dificultad *dificultad){

    int opcion_menu;

    for (;;){ // ejecuta el menú hasta que el usuario elija modo y dificultad, o salga del juego
        limpiar_pantalla_cli();
        mostrar_menu_principal();
        opcion_menu = pedir_opcion_al_usuario();

        switch (opcion_menu){
            case OPCION_PVP:

                *modo = PVP;    // asigna modo Jugador vs Jugador

                if (elegir_dificultad(dificultad) == 0){ // muestra el menú de dificultad
                    break;  // vuelve al menú principal 
                }

                return 1; // modo y dificultad asignados correctamente

            case OPCION_PVIA:
                
                *modo = PVIA;   // asigna modo Jugador vs IA

                if (elegir_dificultad(dificultad) == 0){ // muestra el menú de dificultad
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
    if (fila >= 0 && fila <= 9){
        return 1;
    }
    return 0;
}

int solicitar_entrada_de_columna(void){
    int opcion;

    printf("Ingrese una opción: ");
    opcion = getchar();
}

void pedir_posicion_del_barco(Jugador *jugador, Tamanio barco){
    
    int opcion;

    printf("Barco: %s\n", nombres_barcos[barco - 1]);
    printf("Fila: ");

    do {
        opcion = pedir_opcion_al_usuario();
        opcion--;
    } while (!validar_entrada_fila_y_columna(opcion));
    
}
