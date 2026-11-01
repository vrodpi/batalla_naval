#include "../include/ui_cli.h"
#include "../include/game.h"
#include "../include/tablero.h"

int main(void){

    Partida partida;

    mostrar_presentacion_y_reglamento(partida.jugador1.Nombres);

    elegir_modo_de_juego();
}