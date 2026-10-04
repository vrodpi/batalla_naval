#include "../include/ui_cli.h"

#define MAX_CARACTERES_NICKS 21


char nombres_jugadores[2];
int cantidad_de_jugadores = 0;

#ifdef _WIN32 // funciones escritas para Windows

void limpiar_pantalla_cli(void) {
        system("cls");            
}

#elif defined(__linux__) // funciones escritas para Linux

void limpiar_pantalla_cli(void) {
    system("clear");
}

#endif
    
void esperar_usuario(void){
    printf("Presione 'enter' para continuar.\n");
    getchar();
}

char *pedir_nombre_jugador(int numero_de_jugador){

    char nick_jugador[MAX_CARACTERES_NICKS];
    
    printf("Ingrese el nombre del jugador %d: ", numero_de_jugador + 1);
    fgets(nick_jugador, MAX_CARACTERES_NICKS, stdin);
    nick_jugador[strcspn(nick_jugador, "\n")] = '\0';

    return nick_jugador;
}

void saludar_jugador(char *nombre_jugador){
    printf("Bienvenido %s!\n", nombre_jugador);
}

void ingresar_jugador(void){

    nombres_jugadores[cantidad_de_jugadores] = pedir_nombre_jugador(cantidad_de_jugadores);

    saludar_jugador(nombres_jugadores[cantidad_de_jugadores]);

    cantidad_de_jugadores++;
}

void mostrar_archivo_de_texto(const char *ruta_al_archivo){
    
    FILE *reglamento = fopen(ruta_al_archivo, "r");

    if (reglamento == NULL) {
        printf("No se pudo abrir el archivo \"%s\"\n", ruta_al_archivo);
        return 1;
    }

    int c;

    while ((c = fgetc(reglamento)) != EOF) {
        putchar(c);
    }

    fclose(reglamento);
}

void mostrar_presentacion_y_reglamento(void){
    limpiar_pantalla_cli();

    ingresar_jugador();

    limpiar_pantalla_cli();

    char ruta_reglamento[] = "../files/Reglamento.txt";

    mostrar_archivo_de_texto(ruta_reglamento);
    esperar_usuario();
    
}

void mostrar_menu_principal(void){
    imprimir_linea();
}

void imprimir_linea(void){
    printf("-------------------------------------------------------------------\n");
}