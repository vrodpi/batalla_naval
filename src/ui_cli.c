#include "../include/ui_cli.h"
#include "../include/tablero.h"
#include "../include/game.h"

void limpiar_pantalla_cli(void){
    #if defined(_WIN32) && !defined(__CYGWIN__)
        system("cls");
    #else
        system("clear");
    #endif
}

void esperar_usuario(void){
    printf("Presione 'enter' para continuar.\n");
    getchar();
}

void pedir_nombre_jugador(char jugador[]){

    printf("Ingrese el nombre del jugador: ");
    fgets(jugador, sizeof(jugador), stdin);
    jugador[strcspn(jugador, "\n")] = '\0';

}

void saludar_jugador(char jugador[]){
    printf("Bienvenido %s!\n", jugador);
}

void ingresar_y_saludar_jugador(char jugador[]){

    pedir_nombre_jugador(jugador);

    saludar_jugador(jugador);
}

void mostrar_archivo_de_texto(const char *ruta_al_archivo){
    
    FILE *archivo = fopen(ruta_al_archivo, "r");

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo \"%s\"\n", ruta_al_archivo);
        return 1;
    }

    int c;

    while ((c = fgetc(archivo)) != EOF) {
        putchar(c);
    }

    fclose(archivo);
}

void mostrar_presentacion_y_reglamento(char jugador[]){
    limpiar_pantalla_cli();

    ingresar_y_saludar_jugador(jugador);

    limpiar_pantalla_cli();

    char ruta_reglamento[] = "../files/Reglamento.txt";

    mostrar_archivo_de_texto(ruta_reglamento);
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
    printf("3. Mostrar ranking.\n");
    printf("0. Salir del juego.\n");
    imprimir_linea();
}

void mostrar_menu_de_dificultad(void){
    imprimir_linea();
    printf("Seleccione el nivel de dificultad:\n");
    printf("    1. Modo normal\n");
    printf("    2. Modo Tormenta\n");
    imprimir_linea();
}

void imprimir_linea(void){
    printf("-------------------------------------------------------------------\n");
}

void elegir_modo_de_juego(void){

    int opcion_menu;

    do {
        limpiar_pantalla_cli();

        mostrar_menu_principal();
        printf("Ingrese una opción: ");
        scanf("%d", &opcion_menu);
        getchar();

        switch (opcion_menu){
            case 1:
                /*
                mostrar_menu_de_dificultad();
                printf("Ingrese una opción: ");
                scanf("%d", opcion_menu);
                getchar();
                */

                // iniciar partida vs Jugador
                break;
            case 2:
                // seleccionar dificultad
                // iniciar artida vs IA
                break;
            case 3:
                // mostrar ranking
                break;
            case 0:
                printf("Saliendo...\n");
                break;
            default:
                printf("No se reconoce la opción ingresada.\n");
                break;
        }
    } while (opcion_menu != 0);
}
/*
void pedir_posicion_del_barco(Jugador jugador, Tamanio barco_a_colocar){
    int barcos_colocados = 1;

    printf();
}
    */