# Batalla naval — documento general del proyecto

## 1. Descripción

El proyecto consiste en un juego de batalla naval desarrollado en C como trabajo final de una materia de programación de la licenciatura en informática. El equipo está formado por cuatro personas y la fecha objetivo de entrega es mediados de noviembre.

La primera versión tendrá una interfaz de consola. Más adelante se podrá agregar una interfaz gráfica, manteniendo las mismas reglas y el mismo motor del juego.

## 2. Alcance inicial

- Dos modalidades: jugador contra jugador y jugador contra máquina.
- Tablero inicial de 10 × 10 casillas.
- Cinco barcos por jugador, de 5, 4, 3, 2 y 1 casillas.
- Cada jugador coloca manualmente sus barcos antes de comenzar. Cada barco se ubica en sentido horizontal o vertical.
- El programa verifica que cada barco quede dentro de los límites del tablero y que no se superponga con otro barco.
- Gana quien primero hunde todos los barcos del oponente.
- El juego registra las victorias por nombre y muestra un ranking al terminar la partida.
- El ranking se guarda en un archivo `ranking.dat`.
- El programa debe poder compilarse y ejecutarse en Windows y Linux.

El tamaño del tablero y la cantidad y longitud de los barcos deberían definirse como datos de configuración para poder cambiarlos en el futuro sin modificar la lógica de cada módulo.

## 3. Estructura sugerida

```text
.
├── docs/
│   ├── contrato-ranking.md
│   └── proyecto.md
├── include/
│   ├── app.h
│   ├── config.h
│   ├── game.h
│   ├── ranking.h
│   ├── tablero.h
│   └── ui_cli.h
├── src/
│   ├── main.c
│   ├── app.c
│   ├── ui_cli.c
│   ├── game.c
│   ├── tablero.c
│   ├── ranking.c
│   └── demo_ranking.c
├── data/                 # Se crea o contiene ranking.dat
├── files/                # Reglamento.txt
├── tests/
│   ├── test_ranking.c
│   ├── test_tablero.c
│   └── test_game.c
├── Makefile
└── README_gian.md
```

`game.c` (motor de partida) ya esta integrado y cubierto por pruebas. `ai.c`
(maquina) y la conexion de la partida completa desde `app.c` se agregaran en
la proxima etapa; por ahora el programa compila y ejecuta presentacion,
reglamento, menu, dificultad y nombres.

La futura interfaz gráfica puede agregarse como `ui_gui.c`, usando las mismas operaciones públicas que la interfaz de consola. No es necesario implementarla para empezar el motor ni el flujo de consola.

## 4. Responsabilidad de cada módulo

| Archivo o módulo | Responsabilidad |
|---|---|
| `main.c` | Punto de entrada. Inicializa la aplicación y llama a `app_run()`. Debe mantenerse pequeño. |
| `app.c` | Coordina el recorrido de pantallas, el menú, los turnos y el final de la partida. Decide qué paso sigue. |
| `ui_cli.c` | Presenta información en consola y recoge entradas del usuario. Más adelante, `ui_gui.c` ofrecerá la misma interacción mediante una GUI. |
| `game.c` | Mantiene las reglas y el estado de la partida: inicialización, colocación, turnos, disparos, barcos hundidos y condición de victoria. Implementado y probado. |
| `tablero.c` | Maneja las casillas, la colocación de barcos, los disparos recibidos y los barcos hundidos. |
| `ai.c` | Elige los disparos de la máquina a partir de los resultados de sus disparos anteriores. (Pendiente de integración.) |
| `ranking.c` | Actualiza, consulta, muestra y guarda las victorias por jugador en `ranking.dat`. |

La separación principal es: `app.c` coordina el flujo, la interfaz interactúa con los jugadores y el motor (`game.c` y `tablero.c`) aplica las reglas. Las interfaces no deberían implementar reglas del juego.

## 5. Flujo general

1. Presentación.
2. Reglamento.
3. Solicitud del nombre del primer jugador.
4. Menú para elegir jugador contra jugador o jugador contra máquina.
5. En jugador contra jugador, solicitud del nombre del segundo jugador.
6. Cada jugador coloca sus barcos manualmente, en horizontal o vertical.
7. Desarrollo de la partida. En jugador contra jugador, una pantalla de espera separa los turnos para que el siguiente jugador no vea el tablero del adversario.
8. Pantalla de victoria.
9. Actualización y presentación del ranking.
10. Opción de volver al menú o salir.

Estado actual del flujo: los pasos 1 a 5 están implementados, incluida la
elección de dificultad, el ranking y el reglamento desde el menú. El motor de
partida (`game.c`) ya está integrado y probado con sus propias funciones de
colocación y disparo. Queda pendiente conectar la colocación manual de la
flota, la IA (`ai.c`), la partida completa (pasos 6 a 8) y el registro
automático del ganador (paso 9).

## 6. Reglas de colocación y partida

Al colocar cada barco, el programa debe comprobar que:

- La orientación elegida sea horizontal o vertical.
- Todas las casillas ocupadas por el barco estén dentro del tablero.
- El barco no se superponga con otro ya colocado.

Si la ubicación no es válida, se informa el motivo y se permite volver a ingresar la posición. Todavía queda por decidir si dos barcos pueden tocarse por los lados o en diagonal; esa regla debe acordarse y aplicarse consistentemente.

Durante la partida, el motor también debe rechazar coordenadas fuera del tablero y disparos repetidos sobre una misma casilla. La interfaz se ocupa de traducir las coordenadas ingresadas por las personas a las coordenadas que utiliza el programa.

En el modo jugador contra jugador, la interfaz muestra al jugador activo la información que le corresponde y oculta la ubicación de los barcos del adversario. La pantalla de espera sirve para separar los turnos cuando ambos jugadores comparten el mismo equipo.

## 7. Comportamiento de la máquina

La máquina comienza eligiendo al azar una casilla que todavía no haya recibido un disparo. Si logra un impacto, su siguiente disparo se elige entre las casillas contiguas que sean válidas y no hayan sido probadas. Si no quedan casillas contiguas disponibles, vuelve a elegir aleatoriamente entre las casillas restantes.

La máquina nunca debe repetir un disparo ni elegir una coordenada fuera del tablero. `ai.c` propone el disparo y `game.c` valida y aplica la acción.

## 8. Ranking y archivo de datos

El ranking registra el nombre de cada jugador y sus victorias acumuladas. Al terminar una partida, se suma una victoria al ganador y se guarda el ranking. Si `ranking.dat` todavía no existe, el juego puede iniciar con un ranking vacío.

El grupo eligió la **Opción A**: guardar un volcado binario directo del struct `Ranking` con `fwrite`/`fread`. Es simple y evita el parseo de texto, pero no es portable entre arquitecturas y no detecta corrupción fina (un byte alterado que siga siendo un valor válido). El contrato completo, el layout de 3604 bytes, las reglas de validación y las limitaciones están en `docs/contrato-ranking.md`.

## 9. Portabilidad

- Usar C23 (`-std=c23 -pedantic -Wall -Wextra`) como estándar único y mantener las reglas independientes del sistema operativo. Todos los módulos se compilan con el mismo estándar y sin extensiones del compilador.
- Concentrar las diferencias de consola entre Windows y Linux en la interfaz, no en el motor.
- Leer entradas con funciones que permitan validar la línea completa antes de convertirla.
- Definir dónde se guarda `ranking.dat` y documentar cómo se ejecuta el programa desde ese directorio.
- Usar un `Makefile` para compilar con GCC o Clang; documentar o agregar luego la alternativa elegida para Windows.

## 10. Reparto inicial para cuatro integrantes

Una división posible es:

1. Tablero y reglas de colocación de barcos.
2. Flujo de pantallas e interfaz de consola.
3. Inteligencia artificial.
4. Ranking, archivo de datos y configuración de compilación.

El equipo debería acordar primero las interfaces entre módulos —nombres de funciones, datos que reciben y devuelven, y formato de coordenadas— para que las partes puedan integrarse sin duplicar reglas.

## 11. Decisiones tomadas y pendientes

Decisiones ya aplicadas en el código:

- Los barcos deben dejar al menos una casilla de agua alrededor; no pueden
  tocarse ni por los lados ni en diagonal.
- Cada disparo válido consume un turno; los disparos inválidos o repetidos no
  cambian el turno.
- El ranking se ordena por victorias descendentes y, en caso de empate, por
  nombre sin distinguir mayúsculas de minúsculas.
- Los nombres que solo difieren en mayúsculas y minúsculas son el mismo
  jugador.
- La flota se coloca en orden de mayor a menor longitud: 5, 4, 3, 2 y 1.

Pendiente de acordar o implementar:

- Si un impacto debe permitir al jugador volver a disparar en el mismo turno.
- La estrategia completa de la máquina (`ai.c`) y su integración con el motor.
- La secuencia de pantallas de la partida completa y el registro automático
  del ganador en el ranking.

## 12. Hitos sugeridos

- **Inicio:** acordar reglas, estructura de archivos y contratos entre módulos.
- **Primera etapa:** tablero, colocación manual validada y disparos entre dos jugadores.
- **Segunda etapa:** flujo de consola completo y modalidad contra máquina.
- **Tercera etapa:** ranking persistente e integración de todos los módulos.
- **Cierre:** probar partidas completas en Windows y Linux, corregir errores y preparar la demostración.
