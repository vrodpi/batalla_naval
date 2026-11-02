#include "../include/game.h"

void inicializar_partida(Partida *partida){

    partida->victoria = 0;
    partida->turno = PJ1;
    partida->cantidad_turnos = 0;

    // ubicar barcos
}

int comprobar_victoria(Partida *partda){
    
    if (partda->victoria == 1){
        return 1;
    }

    return 0;
}

void pasar_turno(Partida *partida){
    if (partida->turno == PJ1){
        partida->turno = PJ2;
    }
    else {
        partida->turno = PJ1;
    }
}