# BATALLA NAVAL

## Introducción: 
Reversión Clásica del Juego **BATALLA NAVAL**, ahora con la implementación del Modo **Tormenta**.  
Desarrollado en el Lenguaje C, para su uso por Consola y por Interfáz gráfica. 
Proyecto realizado por Estudiantes de la Carrera Analista Programador Universitario, de la Universidad Nacional de la Patagonia San Juan Busco, para la Cátedra Algoritmica y Programación I, dictada por los Profesores Juan Manuel Cortez, Martín Ebene y Gustavo Pazos. 


## Indice
A) Integrantes del Proyecto <br> 
B) Reglamento y Desarrallo. <br> 
C) Modos (Jugadores - IA) y Dificultad (Normal - Tormenta) <br> 
D) Juguemos una partida
E) Archivos <br> 
F) Compilación y Ejecución <br> 
G) Comentarios <br> 



### INTEGRANTES DEL PROYECTO

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



### REGLAMENTO y DESARROLLO DEL JUEGO. 

#### Tableros: 
*Tamaño 10x10. 100 Casillas*. <br> 
Tablero de Barcos (En Adelante solo "Tablero"): Se registran los barcos del Jugador de Turno, y se actualiza según los Disparos DEL enemigo. <br> 
Tablero de Disparos: Para llevar un registro de los Disparos AL enemigo. <br> 
Al inicio de cada Partida, ambos Tableros solo contienen Agua. <br> 
*Para el ingreso de la Flota y para dar las Coordenadas de ataque, solo se Permite:
Ingreso Numeral de la Fila (1 al 10). <br> 
Ingreso Alfabético de la Columna (A a J). <br> 
*La *FLOTA* de cada Jugador consiste en 5 barcos, que se diferenciarán de acuerdo a su nombre/tamaño respectivamente. <br> 

**Tipo y tamaño del barco**
|Cantidad|**FLOTA**         | **TAMAÑO**  | 
|:------:|:----------------:|:-----------:|
| 1      |    porta_aviones | 5 casilleros|
| 1      |    buque         | 4 casilleros|
| 1      |    submarino     | 3 casilleros|
| 1      |    velero        | 2 casilleros|
| 1      |    lancha        | 1 casilleros|

<br> 

**Captura de Pantalla Menú Principal**

![Menú Principal](<img/image Menu.png>)

<br> 

![Dificultad](<img/image-1 Dificultad.png>)

<br> 

  
**Ingreso de Barcos:**       
*No se permite que los barcos se superpongan.  <br> 
*No se permite que los barcos se encuentren ubicados en posiciones contiguas, ni en diagonal. (Al menos una casilla de distancia entre ellos).<br> 
*No se permite el ingreso de ubicación fuera de rango. <br>
*No se permite el ingreso de ubicación repetida. <br> 
*No se permite el ingreso del barco, si por su tamaño (cantidad de casillas que ocupe), no entra completo en el Tablero. <br> 

![Flota](<img/image-2 Flota.png>)

**Disparos. Ataque a Flota Oponente.**
El Atacante es el jugador de Turno. Solo tiene permitido hacer un (1) único disparo por turno. <br> 
El Disparo consiste en elegir (y escribir) una Fila y Una Columana.
Si el Disparo alcanzó un Barco, se confirma por Pantalla y se actualizan los tableros. En el Tablero Impacto a Barco es X . <br> 
Si el Disparo fue a agua, entonces en pantalla "AGUA". En el tablero Impacto a Agua se representa con un 0. <br> 
No se permite el ingreso de ubicación repetida. De ser el caso, se solicitan nuevas coordenadas.<br> 

![Disparos](<img/image-3 Disparos.png>)


**Partidas:**
Al iniciar la Partida, se solicita el nombre del jugador, y a traves de un Menú el usuario accederá a opciones sobre Modo de Juego y Dificultad que debera elegir para continuar la partida. <br> 
Entre turno de jugadores, se ha implementado una pantalla cortina para separar los turnos, para evitar ver el tablero del adversario. <br>
El Ganador de la Partida es el Jugador que lográ Hundir los 5 barcos de la flota Enemiga. Y con ello, se le suma 1 (una) Victoria en *ranking* <br> 

![Mensaje Tormenta](<img/image-5 Tormenta.png>)
![Tormenta "/"](img/image-6.Tormenta.png)

### MODOS DE JUEGO 
1) JUGADOR1 VS JUGADOR2 
    DIFICULTAD: PARTIDA CLASICA 
    DIFICULTAD: PARTIDA CON TORMENTA
2) JUGADOR VS IA (MÁQUINA)

**JUGADOR1 VS JUGADOR2:**
Enfrentamiento entre Personas.
La interfaz muestra al jugador de turno sus tableros. En **Tablero** se muestra la situación actual de su Flota, y en **Tablero de Disparos** lleva un registro de sus operaciones tácticas. <br>
Como se mencionó anteriormente, mediente una cortina, se concede el turno al siguiente jugador y así se oculta la ubicación de la flota del oponente. <br> 

**MODO VS IA**
Enfrentamiento entre un Jugador y la Máquina.

## DIFICULTAD NORMAL || DIFICULTAD TORMENTA 

**MODO NORMAL**
Juego Clásico, sea el modo elegido (Entre Jugadores o Contra IA). No difiere en uso, ni métodos adicionales, ni estratégicas ya conocidas y/o antes mencionadas. <br>

**MODO TORMENTA APTO PARA JUGAR EN AMBOS MODOS**.
**TORMENTA POR TURNOS**:
Se activa cada 6 turnos. 
**TORMENTA POR IMPACTOS CONSECUTIVOS A AGUA.**
Se Activa Tormenta Cuando el jugador de Turno, haya hecho Impacto a Agua, en sus tres turnos consecutivos.
**¿Pero de que se trata?**
1) El jugador de turno dispara a flota enemiga.  <br>
2) Sin importar si acertó o no, con Tormenta el adversario tendrá su flota en peligro debido a que por coordenada aleatoria elegido por el sistema, un rayo impactará sobre un barco activo (una de sus casillas).  <br> 
No permite el hundimiento total del barco, (excepto que se trate de una lancha, que por su tamaño solo ocupa una casilla).
El impacto por Tormenta se verá en el tablero con el simbolo "/". <br> 

¿Cómo funciona Tormenta por Impactos Consecutivos a Agua? <br> 
Caso 1:<br> 
impacto_agua_consecutivo=0 (contador inicializado en 0) <br> 
Turno 1: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 2: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 3: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
impacto_agua_consecutivo=3 Se Activa Tormenta. <br> 

Caso 2: <br> 
impacto_agua_consecutivo=0 (contador inicializado en 0). <br> 
Turno 1: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 2: Atacante dispara a Agua (impacto_agua_consecutivo + 1). <br> 
Turno 3: Atacante dispara a Barco (impacto_agua_consecutivo= 0) -> se reinicia el contador. <br> 




### JUGUEMOS UNA PARTIDA / FLUJO GENERAL                           
1. Presentación.
2. Reglamento.
3. Solicitud del nombre del primer jugador.
4. Menú Modo: para elegir Modo "jugador contra jugador" o "jugador contra máquina". 
5. Menú Dificultad: "Partida Normal" o "Tormenta".
6. En jugador contra jugador, solicitud del nombre del segundo jugador.
7. Cada jugador coloca sus barcos manualmente, en horizontal o vertical.
8. Desarrollo de la partida. 
En jugador contra jugador, una pantalla de espera separa los turnos, para que el siguiente jugador no vea el tablero del adversario.
En jugador contra la IA, el desarrollo se ejecuta sin interveción de una cortina.
9. Pantalla de victoria.
10. Actualización y presentación del ranking.
11. Opción de volver al menú o salir.



### ARCHIVOS: 
**De que trata cada Archivo y cual es su relación con el resto**

 %% Comentario que no se ve;


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


```

├── bin
│   └── batalla_naval.exe
├── data
│   └── ranking.dat
├── files
│   └── Reglamento.txt
├── include
│   ├── config.h
│   ├── game.h
│   ├── ranking.h
│   ├── tablero.h
│   └── ui_cli.h
├── obj
│   ├── game.o
│   ├── main.o
│   ├── ranking.o
│   ├── tablero.o
│   └── ui_cli.o
├── src
│   ├── demo_ranking.c
│   ├── game.c
│   ├── main.c
│   ├── ranking.c
│   ├── tablero.c
│   └── ui_cli.c
├── README.md
├── git.txt
```
  
Los arhivos cuya terminación sea ".h" solo contienen las funciones y declaraciones de variables necesarias que se implementarán en sus respectivos archivos.c"<br>

**main.c** <br>
Se encarga de la Presentación y Reglamento. Permite el flujo general del programa, cuya ejecución difiere según la decisión del/los jugadores respecto a Modo y Dificultad. También se encarga de la coordinación de la colocación de Barcos y Cambios de turno. <br>


**ui_cli.c**<br>
Se encarga de la interacción por consola con el jugador. Contiene funciones para la solicitud del nombre, coordenadas y orientación de la flota (Requiere validación, para traducir letras de columnas a índices numéricos). Coordenadas de ataque. Mensaje del lugar de Impacto y Victoria. Como así también se encarga de limpiar la consola, de acuerdo al Sistema Operativo que se esté usando. <br>


**game.c** <br>
Administra el inicio de cada partida, los cambios de turno, comprueba si la partida finalizó (por inexistencia de barcos activos). Procesa los disparos y administra los cambios pertinentes en Tablero de Disparos (Jugador de Turno) y Tablero (Defensor). Y la lógica del juego en Modo tormenta. <br>


**tablero.c** <br>
Inicializa los tableros con Agua. Valida que los barcos se ubiquen dentro de los límites del tablero y que los mismos no se superpongan.<br>
El tamaño del barco, en terminos de ubicación, refiere a la cantidad de cuadrantes que ocupa, por lo que también "tablero.c" se encarga de validar que el barco entre completamente. <br> 
Cuando cumpla con las condiciones, solo entonces, actualiza Agua por Barco. <br>

**config.h** <br>

**ranking.c**<br>

**ranking.h**<br>


### COMPILACIÓN Y EJECUCIÓN ###<br>
Para Compilar Batalla Naval se requiere la herramienta Make, que leerá el archivo Makefile. Con Makefile, automatizamos la compilación y vinculamos los archivos. <br>










## ELIMINAR ESTO ANTES DE ENTREGAR EL README ##





/*  TENER EN CUENTA: CONSIGNA DEL TRABAJO. BORRAR DDEL README, AL COMPLETARLO
1) El programa pedirá el nombre del jugador, lo saludará y presentará las reglas o instrucciones antes de
jugar.

2) El programa debe permitir completar una partida. Al finalizar, mostrará el ranking o un resumen de
resultados/puntajes, según corresponda al juego elegido, y permanecerá en esa pantalla hasta que el
usuario presione una tecla; luego finalizará.  
Como alternativa optativa se admite una interfaz gráfica; se sugiere SDL3, biblioteca para C. 

3) Se entregará el código fuente .c y .h, los recursos indispensables y las instrucciones para compilar y
ejecutar el programa. El ejecutable solo no constituye una entrega suficiente.

4) El archivo README.md será el único informe obligatorio. De manera breve, incluirá:
    a. Integrantes y aportes/horas aproximadas por cada uno.
    b. Reglas y alcance del juego.
    c. Instrucciones para compilar, ejecutar y controlar el juego
    d. Organización general de datos y funciones
    e. Si hay interfaz gráfica, también indicará qué funciones conectan la interfaz con la lógica y qué
    parte gráfica se generó mediante IA.
● El README.md podrá incluir opcionalmente capturas de pantalla, imágenes, tablas o diagramas para
facilitar la explicación. Se sugiere Mermaid para elaborar diagramas sencillos (por ejemplo, el recorrido
de una jugada o la relación entre las partes del programa) directamente en el README.md, donde
GitHub puede visualizarlos. Los archivos gráficos utilizados deberán formar parte del repositorio y
estar enlazados desde el README.md.

5) EXPOSICIÓN ORAL: 
    GRUPO: 8 MINUTOS.  
    Mostrar una partida o secuencia representativa y explicar, de forma breve, cómo se organizaron los datos y las funciones.

    INDIVIDUAL: 3- 4 MINUTOS
    Responder preguntas sobre localizar una función, seguir manualmente una jugada, identificar un caso
    límite o proponer o realizar un cambio pequeño y acotado. Se evaluará la comprensión del proyecto
    desarrollado.

6) Preguntas orientativas:

    ● ¿Qué datos representan el estado actual de la partida? Señalalos en el código y explicá cuándo
    cambian.

    ● ¿Por dónde empieza el programa? Mostrá el recorrido desde el menú/inicio hasta una jugada
    concreta.

    ● ¿Qué función decide si una jugada es válida? ¿Qué devuelve o qué cambia cuando es inválida?

    ● ¿Qué funciones elegiste separar y por qué? ¿Qué problema habría si las unieras en una sola?

    ● ¿Qué parámetros recibe una función X y qué resultado produce? ¿Por qué?

    ● ¿Cómo se determina el fin de la partida y el resultado? ¿Dónde se calcula el ranking?

    ● ¿Qué ocurre si el jugador ingresa una opción fuera de rango, repite una jugada o no quedan
    movimientos?

    ● ¿Qué parte del programa cambiarías para modificar una regla sin alterar el resto?

    ● Si aparece un resultado incorrecto, ¿cómo localizarías la función responsable y cómo lo probarías?

    ● ¿Qué decidió o programó cada integrante? Elegí una parte hecha por otra persona y explicá cómo se
    comunica con la tuya.

    ● Si utilizaron una interfaz gráfica generada con IA: ¿qué le pidieron, qué archivos o funciones integraron
    y cómo comprobaron que la interfaz usa correctamente la lógica? Si hicieron todo en consola: ¿cómo
    separaron la presentación de las reglas?

    ● Si hay gráficos: ¿qué acciones manda la interfaz a la lógica, qué datos recibe y qué código habría que
    tocar para cambiar solamente el aspecto visual?

    ● Si hay gráficos: ¿qué pasaría si mañana reemplazaran raylib o SDL3 por otra forma de mostrar el
    juego? ¿Qué funciones deberían permanecer sin cambios?





Crear un Diagrama de Flujo. MERMAID.
Dirección	Significado
TD o TB	    De arriba a abajo
BT	        De abajo a arriba
LR	        De izquierda a derecha
RL	        De derecha a izquierda

```mermaid
graph LR
    A --> B --> C      // para hacer una lista enlazada
```
```mermaid

    A[Rectángulo]           // para las formas de los nodos.
    B(Redondeado)
    C([Estadio])
    D[[Subrutina]]
    E{Rombo / Decisión}
    F{{Hexágono}}
    G>Bandera]
    H[(Base de datos)]
```
```mermaid
    flowchart LR
    A --> B
    A --- C
    A -.-> D
    A ==> E
    A -->|texto| F
    A -- texto --> G
```

FORMA DE LOS NODOS
A[[texto]]	Subrutina
A{texto}	Rombo (decisión)
A{{texto}}	Hexágono
A>texto]	Bandera
A[(texto)]	Base de datos (cilindro)
A((texto))	Círculo
A[/texto/]	Paralelogramo
A[\texto\]	Paralelogramo invertido
A[/texto\]	Trapecio
A[\texto/]	Trapecio invertido


TIPOS DE CONEXIÓN
A --> B	Flecha sólida
A --- B	Línea sin flecha
A -.-> B	Flecha punteada
A ==> B	Flecha gruesa
A -->\|texto\| B	Flecha con etiqueta
A -- texto --> B	Flecha con etiqueta (alternativa)
A ~~~ B	Enlace invisible (para controlar el layout)

// IMAGEN DE PRUEBA BORRAR
<!-- COMENTARIOS QUE NO SE VERAN AL EJECUTARSE EL README -->
para una imagen, copiar y pegar. 


 lleer los include de arriba de cada archiovo para conectar los main.c y eso. 