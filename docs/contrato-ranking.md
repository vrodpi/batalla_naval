# Contrato del modulo ranking (binario, Opcion A)

Este documento es el acuerdo de integracion entre `ranking.c` y el resto
del equipo (app / ui / tablero). Define la API, las reglas, el formato
binario del archivo, el manejo de errores y las decisiones tomadas por el
grupo.

El grupo eligio la **Opcion A**: el archivo `data/ranking.dat` es un
**volcado directo del struct `Ranking`** (`fwrite`/`fread`), sin
serializacion byte a byte, sin magic number, sin version y sin checksum.
La Opcion B (BNR1 v1 + CRC32 + escritura atomica) fue evaluada e
implementada en un momento, y luego reemplazada por esta decision del
grupo. Las consecuencias de A estan documentadas en la seccion 9 para que
nadie las descubra en la defensa.

## 1. Archivos del modulo

| Archivo | Rol |
|---|---|
| `include/ranking.h` | API publica y contrato. Unico header que incluyen los demas modulos. |
| `src/ranking.c` | Logica, validacion, orden y persistencia binaria. |
| `src/demo_ranking.c` | Demo de fin de partida (maqueta de integracion). |
| `tests/test_ranking.c` | Pruebas automaticas sin frameworks externos. |
| `include/config.h` | `CONFIG_RANKING_RUTA` = `data/ranking.dat`. |
| `data/ranking.dat` | Datos binarios (3604 bytes). No editar a mano. |

Dependencias: solo `<stdio.h>`, `<string.h>` y `<ctype.h>`. No usa
`windows.h`, `unistd.h`, `conio.h` ni `stdint.h`; sigue siendo ANSI C
(C89). El objetivo de compilacion acordado es Linux (probado tambien en
Windows con LLVM-MinGW).

## 2. Modelo en memoria

```c
#define RANKING_NOMBRE_MAX 32      /* incluye '\0': 31 caracteres utiles */
#define RANKING_MAX_JUGADORES 100
#define RANKING_VICTORIAS_MAX 1000000
#define RANKING_RUTA_DEFAULT "data/ranking.dat"

typedef struct { char nombre[32]; int victorias; } RankingEntrada;
typedef struct { RankingEntrada entradas[100]; int cantidad; } Ranking;
```

`Ranking` vive en la pila del llamador. No hay `malloc` ni `free`.

Invariantes:

1. Solo las primeras `cantidad` posiciones son validas.
2. Tras registrar o cargar, el arreglo queda ordenado de mejor a peor.
3. `nombre` de cada entrada usada tiene `'\0'` dentro de sus 32 bytes y
   conserva la primera grafia guardada, ya recortada.

## 3. API publica

```c
void ranking_inicializar(Ranking *r);
int  ranking_cantidad(const Ranking *r);
const RankingEntrada *ranking_obtener(const Ranking *r, int indice);
int  ranking_buscar(const Ranking *r, const char *nombre);
int  ranking_obtener_victorias(const Ranking *r, const char *nombre);
int  ranking_registrar_victoria(Ranking *r, const char *nombre);
void ranking_ordenar(Ranking *r);
int  ranking_cargar(Ranking *r, const char *ruta);
int  ranking_guardar(const Ranking *r, const char *ruta);
const char *ranking_mensaje_error(int codigo);
int  ranking_mostrar_en(FILE *salida, const Ranking *r);
void ranking_mostrar(const Ranking *r);
```

La interfaz **no** modifica `Ranking.entradas` ni suma victorias por su
cuenta. Toda regla (validar, comparar, ordenar, persistir) vive en
`ranking.c`. La UI pide el nombre y traduce el codigo de retorno con
`ranking_mensaje_error()`.

### Codigos de retorno

| Codigo | Valor | Significado |
|---|---:|---|
| `RANKING_OK` | 0 | Exito. |
| `RANKING_ERROR_ARGUMENTO` | 1 | Puntero NULL. |
| `RANKING_ERROR_NOMBRE_INVALIDO` | 2 | Nombre vacio, largo o con controles. |
| `RANKING_ERROR_LLENO` | 3 | Ya hay 100 jugadores. |
| `RANKING_ERROR_ARCHIVO` | 4 | No se pudo abrir, leer o escribir. |
| `RANKING_ERROR_FORMATO` | 5 | Archivo binario invalido o corrupto. |

## 4. Recorrido de fin de partida

```
tablero detecta victoria -> app/ui obtiene el nombre del ganador ->
  ranking_inicializar(&r)
  rc = ranking_cargar(&r, CONFIG_RANKING_RUTA)
       ARCHIVO -> primera partida o archivo ilegible: seguir con vacio
       FORMATO -> archivo corrupto o de otro formato: informar y seguir
  rc = ranking_registrar_victoria(&r, ganador)
       NOMBRE_INVALIDO / LLENO -> ui muestra ranking_mensaje_error(rc)
  rc = ranking_guardar(&r, CONFIG_RANKING_RUTA)
       ARCHIVO -> ui informa "no se pudo guardar"
  ranking_mostrar(&r)   [o la UI recorre cantidad/obtener]
  ui espera ENTER -> menu/salir
```

Ejemplo minimo:

```c
Ranking r;
int rc;

ranking_inicializar(&r);
rc = ranking_cargar(&r, CONFIG_RANKING_RUTA);
/* RANKING_ERROR_ARCHIVO en la primera partida es normal. */
rc = ranking_registrar_victoria(&r, ganador);
if (rc == RANKING_OK) {
    rc = ranking_guardar(&r, CONFIG_RANKING_RUTA);
}
ranking_mostrar(&r);
```

## 5. Formato del archivo (Opcion A)

Un solo bloque de `sizeof(Ranking)` bytes, little-endian en x86,
**sin** cabecera, magic, version ni checksum:

| Offset | Tamano | Campo |
|---:|---:|---|
| 0 | 3600 | `entradas[0..99]`, 100 x 36 bytes |
| 3600 | 4 | `cantidad` (`int`, 0..100) |

Cada entrada de 36 bytes:

| Offset relativo | Tamano | Campo |
|---:|---:|---|
| +0 | 32 | Nombre: ASCII, `'\0'` final, resto en ceros |
| +32 | 4 | Victorias (`int`, 0..1000000) |

- Tamano total: **3604 bytes siempre**, haya 0 o 100 jugadores.
- No se guarda cuantos bytes siguen: `cantidad` es la unica guia.
- El layout no tiene relleno (padding) en x86_64 con `int` de 4 bytes.
  `ranking.c` incluye dos `typedef char ...[...]` de "assert" que hacen
  fallar la compilacion si un compilador inserta padding o cambia el
  tamano de `int`. Es la unica proteccion posible sin cambiar el formato.
- Se abre SIEMPRE en modo binario (`"wb"`/`"rb"`). En modo texto,
  Windows traduce `0x0A` a `0x0D 0x0A` y corromperia los enteros (por
  ejemplo, 10 victorias).

### Escritura deterministicamente cero

`ranking_guardar()` no hace `fwrite(r)` directo. Copia el struct a una
variable local, la pone toda a cero con `memset` y copia solo los datos
validos (nombre hasta el `'\0'`, con tope de 31, y victorias). Motivo:
los slots libres `entradas[cantidad..99]` y la cola de cada nombre
contienen basura de la pila; volcarlos haria que el archivo cambie en
cada partida y filtraria memoria del proceso. El layout en disco es el
mismo; solo se garantiza que los bytes no usados sean cero (y por lo
tanto, mismos datos -> mismos bytes).

## 6. Validacion al cargar

`ranking_cargar()` es tolerante por entrada, pero estricta con el
contenedor:

1. Abre en `"rb"`. Si no existe -> `RANKING_ERROR_ARCHIVO` (no fatal).
2. `fread` de un struct completo:
   - error de E/S -> `ARCHIVO`;
   - archivo vacio o truncado -> `FORMATO`.
3. `fgetc` extra: si hay **cualquier** byte despues del struct, el
   archivo es de otro tamano/version -> `FORMATO`.
4. `cantidad` fuera de `0..100` -> `FORMATO` y ranking vacio (no se
   recorre memoria invalida).
5. Por cada entrada declarada:
   - se exige un `'\0'` dentro de `nombre[32]` (si no, `strlen` se
     saldria del struct);
   - se valida el nombre con las reglas normales;
   - se validan las victorias en `0..1000000`;
   - entrada invalida: se descarta y se marca `FORMATO`, pero se sigue
     con las demas (carga parcial: las validas quedan cargadas).
6. Duplicados insensibles a mayusculas dentro del archivo se fusionan
   sumando con tope. `ranking_guardar()` nunca los genera; solo pueden
   aparecer en un archivo manipulado.
7. Al final se ordena. Devuelve `OK`, o `FORMATO` si hubo al menos una
   entrada descartada.

## 7. Reglas de negocio

### Nombres

- Se recortan espacios de los extremos.
- Largo valido: 1 a 31 caracteres.
- Se rechazan caracteres de control (`< 32`): `\n`, `\r`, `\t`, etc.
- El `;` **si** se permite: en binario no hay separadores de campo.
- La comparacion de identidad es insensible a mayusculas:
  `Ana == ANA == " ana "`.
- Se conserva la primera grafia guardada.

### Orden

1. Victorias descendente.
2. A igual victorias, nombre ascendente insensible a mayusculas.
3. A igual todo, `strcmp` sensible, solo para determinismo.

### Victorias

- Cada partida ganada suma exactamente 1.
- Tope: `RANKING_VICTORIAS_MAX` (1.000.000). Al llegar, se queda en el
  tope; no desborda.
- Un jugador nuevo entra con 1 victoria.
- Maximo 100 jugadores; el jugador 101 devuelve `RANKING_ERROR_LLENO`.

## 8. Migracion desde el formato texto

La version anterior guardaba `nombre;victorias` en texto. Ese archivo no
mide 3604 bytes, por lo que `ranking_cargar()` devuelve
`RANKING_ERROR_FORMATO`. **No hay migracion automatica.** Como el archivo
esta ignorado por git y solo contiene victorias de prueba, la accion
indicada es borrarlo:

```sh
rm -f data/ranking.dat
```

La demo avisa cuando detecta esta situacion.

## 9. Modelo de amenaza y limitaciones de la Opcion A

**Que protege:** evita que el archivo se edite por accidente con un
editor de texto y que un cambio de formato pase desapercibido por el
tamano (3604 bytes exactos).

**Que NO protege:**

- **No detecta corrupcion fina.** Un byte alterado dentro de un nombre o
  de las victorias puede seguir siendo un valor "valido" (por ejemplo,
  cambiar 3 victorias por 4). Sin checksum no hay forma de saberlo. La
  unica deteccion es por rango y por nombres sin `'\0'`.
- **No es portable entre arquitecturas.** Solo funciona donde el struct
  mida 3604 bytes y el `int` sea de 4 bytes little-endian (Windows x64 y
  Linux x86_64). En otra plataforma el archivo no es intercambiable.
- **No tiene version.** Si el struct cambia (otro tamano de nombre, otro
  campo), los archivos viejos no se pueden distinguir de un archivo
  corrupto: ambos dan `FORMATO`.
- **No es seguridad.** El codigo fuente se entrega y el jugador es dueno
  del archivo; reescribir el binario es trivial. El anti-trampa real
  requeriria validacion en un servidor, fuera del alcance del trabajo.

Estas limitaciones son el motivo por el que existio la Opcion B
(BNR1 v1 + CRC32 + escritura atomica). Fue descartada por decision del
grupo; si en la defensa preguntan "¿como detectan un archivo alterado?",
la respuesta correcta es: **no se detecta**, y explicar por que se acepto
esa limitacion.

## 10. Casos limite cubiertos

- `ranking.dat` no existe -> vacio + `RANKING_ERROR_ARCHIVO`.
- Archivo de 0 bytes -> `FORMATO`.
- Archivo truncado (menos de 3604 bytes) -> `FORMATO`.
- Archivo con bytes de mas -> `FORMATO`.
- `cantidad` = -1 o 101 -> `FORMATO`, ranking vacio.
- Nombre sin `'\0'` dentro de sus 32 bytes -> entrada descartada + `FORMATO`.
- Victorias negativas o mayores al tope -> entrada descartada + `FORMATO`.
- Entrada mala + entrada buena -> carga parcial: la buena se conserva.
- Ranking de texto viejo -> `FORMATO` (tamano distinto).
- Duplicados insensibles a mayusculas -> se fusionan sumando.
- `NULL` en cualquier puntero -> `ARGUMENTO`.
- Guardar con `cantidad` inconsistente -> se acota a `0..100` antes de
  escribir; nunca se genera un archivo con cabecera invalida.
- Guardar en un directorio inexistente -> `ARCHIVO`.

## 11. Como probar solo este modulo

Desde la raiz del proyecto:

```sh
make            # compila build/test_ranking y build/demo_ranking
make test       # ejecuta las pruebas (68 aserciones)
make demo       # recuerda como correr la demo

rm -f data/ranking.dat
./build/demo_ranking "Ana"
ls -l data/ranking.dat     # 3604 bytes
./build/demo_ranking "ANA"
./build/demo_ranking "Ana" # Ana acumula 3 victorias
```

La suite verifica tamano exacto, archivos vacios/truncados/con bytes de
mas, campos invalidos, carga parcial, fusion de duplicados y determinismo
del guardado (dos copias con basura distinta en los slots libres producen
archivos identicos). Ver `tests/test_ranking.c`.

## 12. Integracion pendiente

`make app` compila el juego completo, pero a la fecha de este commit
`src/app.c`, `src/ui_cli.c`, `src/tablero.c` y `src/main.c` todavia no
compilan en `-ansi` (comentarios `//`, declaraciones dentro de bloques,
errores de sintaxis y retornos de variables locales). Esos archivos
pertenecen a otros integrantes y no se modificaron desde este modulo.

Mientras tanto, `make` y `make test` cubren el ranking de punta a punta.

## 13. Historial de decision

1. Version texto `nombre;victorias` (portable, legible).
2. Opcion B: BNR1 v1, serializacion explicita, CRC32 y escritura
   atomica (commit `d524db5` de esta rama).
3. **Opcion A (actual):** volcado directo del struct, sin checksum ni
   version, por decision del grupo. Se conservan de B las reglas de
   nombres, orden y topes; se eliminan `RANKING_ERROR_VERSION` y
   `RANKING_ERROR_INTEGRIDAD`.
