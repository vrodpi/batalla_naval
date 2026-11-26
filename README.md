# BATALLA NAVAL

## INTRODUCCIÓN: 
Versión Clásica del Juego **BATALLA NAVAL**, ahora con la implementación del Modo **Tormenta**.  
Desarrollado en el Lenguaje C, para su uso por Consola y por Interfaz gráfica. 
Proyecto realizado por Estudiantes de la Carrera Analista Programador Universitario, de la Universidad Nacional de la Patagonia San Juan Busco, para la Cátedra Algorítmica y Programación I, dictada por los Profesores Juan Manuel Cortez, Martín Ebene y Gustavo Pazos. 

<br> 

## INDICE
1) Integrantes del Proyecto <br> 
2) Reglamento y Desarrollo. <br> 
3) Modos (Jugadores - IA) y Dificultad (Normal - Tormenta) <br> 
4) Juguemos una partida <br> 
5) Archivos <br> 
6) Compilación y Ejecución <br> 

<br> 

## 1) INTEGRANTES DEL PROYECTO:

| NOMBRES Y APELLIDOS                       | HS TRABAJADAS  |
|-----------------------------------------  |----------------|
|Vicente Rodriguez Pi                       |-------X--------|
|                                           |                |
|Teresa de los Angeles Carabajal Anderson   |-------X--------|
|                                           |                |
|Gianfranco Di Pisa Echagüe                 |-------X--------|
|                                           |                |
|Huenú Rosalba Beatriz Margalef Quijada     |-------X--------|
|                                           |                |

<br> 

## 2) REGLAMENTO y DESARROLLO DEL JUEGO. 
#### a) Tableros: 
* Tamaño 10x10. 100 Casillas*. <br> 
* Tablero de Barcos (En Adelante solo "Tablero"): Se registran los barcos del Jugador de Turno, y se actualiza según los Disparos DEL enemigo. <br> 
* Tablero de Disparos: Para llevar un registro de los Disparos AL enemigo. <br> 
* Al inicio de cada Partida, ambos Tableros solo contienen Agua. <br> 
* Para el ingreso de la Flota y para dar las Coordenadas de ataque, solo se Permite:
    - Ingreso Numeral de la Fila (1 al 10). <br> 
    - Ingreso Alfabético de la Columna (A a J). <br> 
* La *FLOTA* de cada Jugador consiste en 5 barcos, que se diferenciarán de acuerdo a su nombre/tamaño respectivamente. <br> 

**Tipo y tamaño del barco**
|Cantidad|**FLOTA**         | **TAMAÑO**  | 
|:------:|:----------------:|:-----------:|
| 1      |    porta_aviones | 5 casilleros|
| 1      |    buque         | 4 casilleros|
| 1      |    submarino     | 3 casilleros|
| 1      |    velero        | 2 casilleros|
| 1      |    lancha        | 1 casilleros|

<br> 

**Menú Principal**

![Menú Principal](<img/image Menu.png>)

**Selección de Dificultad**

![Dificultad](<img/image-1 Dificultad.png>)

<br> 

  
#### b) Flota e Ingreso de Barcos:      
* No se permite que los barcos se superpongan.  <br> 
* No se permite que los barcos se encuentren ubicados en posiciones contiguas, ni en diagonal. (Al menos una casilla de distancia entre ellos).<br> 
* No se permite el ingreso de ubicación fuera de rango. <br>
* No se permite el ingreso de ubicación repetida. <br> 
* No se permite el ingreso del barco, si por su tamaño (cantidad de casillas que ocupe), no entra completo en el Tablero. <br> 
<br>

**Ingreso de Flota**
![Flota](<img/image-2 Flota.png>)<br> 
<br>

#### c) Disparos. Ataque a Flota Oponente.
* El Atacante es el jugador de Turno. Solo tiene permitido hacer un (1) único disparo por turno. <br> 
* El Disparo consiste en elegir (y escribir) una Fila y Una Columna.
* Si el Disparo alcanzó un Barco, se confirma por Pantalla y se actualizan los tableros. En el Tablero Impacto a Barco es X . <br> 
* Si el Disparo fue a agua, entonces en pantalla "AGUA". En el tablero Impacto a Agua se representa con un 0. <br> 
* No se permite el ingreso de ubicación repetida. De ser el caso, se solicitan nuevas coordenadas.<br> 

**Disparos**
![Disparos](<img/image-3 Disparos.png>) <br> 


#### d) Partidas-Condiciones de Victoria:<br> 
Al iniciar la Partida, se solicita el nombre del jugador, y a traves de un Menú el usuario accederá a opciones sobre Modo de Juego y Dificultad que debera elegir para continuar la partida. <br> 
Entre turno de jugadores, se ha implementado una pantalla cortina para separar los turnos, para evitar ver el tablero del adversario. <br>
El Ganador de la Partida es el Jugador que lográ Hundir los 5 barcos de la flota Enemiga. Y con ello, se le suma 1 (una) Victoria en *ranking* <br> 

**Modo Tormenta**
![Mensaje Tormenta](<img/image-5 Tormenta.png>)<br> 
![Tormenta "/"](img/image-6.Tormenta.png)<br> 

<br>

## 3) MODOS DE JUEGO <br> 
a) JUGADOR1 VS JUGADOR2: CLASICO Y TORMENTA<br> 
b) JUGADOR VS IA (MÁQUINA): CLASICO Y TORMENTA<br> 


#### a) JUGADOR1 VS JUGADOR2:**<br> 
Enfrentamiento entre Personas.<br> 
La interfaz muestra al jugador de turno sus tableros. En **Tablero** se muestra la situación actual de su Flota, y en **Tablero de Disparos** lleva un registro de sus operaciones tácticas. <br>
Como se mencionó anteriormente, mediante una cortina, se concede el turno al siguiente jugador y así se oculta la ubicación de la flota del oponente. <br> 

#### b) JUGADOR VS IA**<br> 
Enfrentamiento entre un Jugador y la Máquina.<br> 

## DIFICULTAD  <br> 

#### a) DIFICULTAD NORMAL
Juego Clásico, sea el modo elegido (Entre Jugadores o Contra IA). No difiere en uso, ni métodos adicionales, ni estratégicas ya conocidas y/o antes mencionadas. <br>

#### b) DIFICULTAD TORMENTA APTO PARA JUGAR EN AMBOS MODOS.

1) El jugador de turno dispara a flota enemiga.  <br>
2) Sin importar si acertó o no, con Tormenta el adversario tendrá su flota en peligro debido a que por coordenada aleatoria elegido por el sistema, un rayo impactará sobre un barco activo (una de sus casillas).  <br> 
* No permite el hundimiento total del barco, (excepto que se trate de una lancha, que por su tamaño solo ocupa una casilla).
* El impacto por Tormenta se verá en el tablero con el simbolo "/". <br> 

#### i) TORMENTA POR TURNOS:<br>
Se activa cada 3 turnos. <br>

#### ii) TORMENTA POR IMPACTOS CONSECUTIVOS A AGUA.<br>
Se Activa Tormenta Cuando el jugador de Turno, haya hecho Impacto a Agua, en sus tres turnos consecutivos.<br>

**¿Cómo funciona Tormenta por Impactos Consecutivos a Agua?**<br> 

Caso 1:<br> 
impacto_agua_consecutivo=0 (contador inicializado en 0) <br> 
Turno 1: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 2: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 3: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
impacto_agua_consecutivo=3 Se Activa Tormenta. <br> 
<br>
Caso 2: <br> 
impacto_agua_consecutivo=0 (contador inicializado en 0). <br> 
Turno 1: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 2: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 3: Atacante dispara a Barco (impacto_agua_consecutivo= 0) -> se reinicia el contador. <br> 

<br>

## 4) JUGUEMOS UNA PARTIDA / FLUJO GENERAL                           
1. Solicitud del nombre del primer jugador.<br>
2. Saludo
3. Reglamento.<br>
4. Menú:
    * Para elegir Modo "jugador contra Jugador" o "Jugador contra Máquina". <br>
    * Para mostrar el Ranking y el Reglamento.
    * Opción "Salir del Juego".
5. Menú Dificultad: "Partida Normal" o "Tormenta".<br>
6. En jugador contra jugador, solicitud del nombre del segundo jugador.<br>
7. En Cualquier Modo/Dificultad elegida: Cada jugador deberá ingresar sus barcos por teclado, fila, columna, y especificar su orientación (horizontal o vertical).<br>
8. Desarrollo de la partida. <br>
    * Cada Jugador tendrá solo un disparo por Turno. Deberá ingresar coordenada de ataque (Fila y Columna), el tipo de impacto se indicará mediante Mensaje de Aviso y se verá reflejado en el Tablero de Disparos del Atacante y en el Tablero de Barcos del Adversario. 
    * En jugador contra jugador, una pantalla de espera separa los turnos, para que el siguiente jugador no vea el tablero del adversario.<br>
    En jugador contra la IA: Durante el Turno de la Máquina, en pantalla muestra los tableros del Jugador Humano y un Mensaje del lugar en donde disparó y su resultado. Habrá una cortina para indicar cambio de Turno.<br>
9. Mensaje de victoria, cuando la flota enemiga haya sido destruida.<br>
10. Menú (Para indicar si volver a Jugar o salir).<br>

<br>

## 5) ARCHIVOS: 
#### De que trata cada Archivo y cual es su vínculo<br>

#### DIAGRAMA DE DEPENDENCIAS
```mermaid
   


   flowchart TD
    Main:::rojo --> Game[game.c]:::turquesa
    Main --> UI[ui_cli.c]:::amarillo
    Main --> Tablero[tablero.c]:::verde
    Main --> IA[ai.c]:::violeta
    Main --> Ranking[ranking.c]:::naranja
    Tablero --> IA 
    Game --> Tablero
    UI --> Game
    UI --> Ranking
   

    classDef verde fill: #94e366,stroke: #28a745, color: #000000
    classDef naranja fill: #ed8123,stroke: #fd7106, color: #000000
    classDef rojo fill: #f35e6a,stroke: #dc3545, color: #000000
    classDef violeta fill: #d66cf1,stroke: #6f42c1, color: #000000
    classDef turquesa fill: #a9dae4,stroke: #3acef0,color: #000000
    classDef amarillo fill: #eeec65,stroke: #f2f531,color: #000000

```

**Arbol de Directorios**

```
BATALLA 
|
├── bin
│   └── batalla_naval.exe
|
├── data
│   |__ ranking.dat
|
├── files
│   └── Reglamento.txt
|
|__img
|   |__image Menu.png
|   |__image-1 Dificultad.png
|   |__image-2 Flota
|   |__image-3 Disparos
|   |__image-5 Tormenta
|   |__image-6 Tormenta
| 
├── include
│   ├── config.h
│   ├── game.h
│   ├── ranking.h
│   ├── tablero.h
│   |__ ui_cli.h
|   |__ai.h
|
├── obj
│   ├── game.o
│   ├── main.o
│   ├── ranking.o
│   ├── tablero.o
│   |__ ui_cli.o
|   |__ai.o
|
├── src
│   ├── demo_ranking.c
│   ├── game.c
│   ├── main.c
│   ├── ranking.c
│   ├── tablero.c
│   |__ ui_cli.c
|   |__ai.c
|
|__tests
|  |__test_ai.c
|
|__MAkefile
|
├── README.md
|
|__ git.txt
```
  
Los arhivos cuya terminación sea ".h" solo contienen las funciones y declaraciones de variables necesarias que se implementarán en sus respectivos archivos.c"<br>

#### main.c <br>
Se encarga de la Presentación y Reglamento. Permite el flujo general del programa, cuya ejecución difiere según la decisión del/los jugadores respecto a Modo y Dificultad. También se encarga de la coordinación de la colocación de Barcos y Cambios de turno. 

#### ui_cli.c 
Se encarga de la interacción por consola con el jugador. Contiene funciones para la solicitud del nombre, coordenadas y orientación de la flota (Requiere validación, para traducir letras de columnas a índices numéricos). Coordenadas de ataque. Mensaje del lugar de Impacto y Victoria. Como así también se encarga de limpiar la consola, de acuerdo al Sistema Operativo que se esté usando. <br>

#### game.c
Administra el inicio de cada partida, los cambios de turno, comprueba si la partida finalizó (por inexistencia de barcos activos). Procesa los disparos y administra los cambios pertinentes en Tablero de Disparos (Jugador de Turno) y Tablero (Defensor). Y la lógica del juego en Modo tormenta. <br>

#### tablero.c
Inicializa los tableros con Agua. Valida que los barcos se ubiquen dentro de los límites del tablero y que los mismos no se superpongan.<br>
El tamaño del barco, en terminos de ubicación, refiere a la cantidad de cuadrantes que ocupa, por lo que también "tablero.c" se encarga de validar que el barco entre completamente. <br> 
Cuando cumpla con las condiciones, solo entonces, actualiza Agua por Barco. <br>

#### config.h 
En este archivo nombramos Macros para que el preprocesador, las reemplace por valores que previamente definimos: Dimensiones del tablero, cantidad de barcos y sus tamaños,etc.<br>
Al realizar algun cambio en los valores de las macros, también se aplica en todos los archivos que las incluyan. <br>
Con #ifndef BATALLA_NAVAL_CONFIG_H y #endif evitamos duplicados del archivo.<br>
<br>

#### ranking.c
Registra, y administra y almacena las victorias en un archivo binario, "ranking.dat". Ordena de mayor a menor cantidad de victorias por jugador.
<br>


## 6) COMPILACIÓN Y EJECUCIÓN <br>
En Batalla Naval utilizamos Makefile, para vincular los archivos y automatizar la compilación. <br>
Cada archivo.c se compila individualmente a traves del -c, generado su archivo.o (archivo objeto). La variable OBJS contiene la lista de estos archivos. Luego GCC (CC) los enlaza para obtener un único ejecutable denominado "batalla_naval.exe", que se va a almacenar en el directorio "bin". <br>

En la terminal, nos ubicamos en la carpeta del Juego y utilizamos los siguientes comandos:<br>

1) make clean: para eliminar los archivos y directorios que se generaron al compilar. Para hacer la compilación desde cero.<br>
2) make: para compilar.<br>
3) ./bin/batalla_naval.exe: para ejecutar el Juego.<br>

**Comandos**
```
make clean
make
./bin/batalla_naval.exe
```

