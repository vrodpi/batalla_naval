# Contrato del modulo ranking (BNR1 v1)

Este documento es el acuerdo de integracion entre `ranking.c` y el resto
del equipo (app / ui / tablero). Define la API, las reglas, el formato
binario del archivo, el manejo de errores y las decisiones tomadas por el
grupo.

El grupo eligio la **Opcion B**: serializacion binaria explicita,
versionada y con checksum. No se guarda el `struct` con `fwrite`.

## 1. Archivos del modulo

| Archivo | Rol |
|---|---|
| `include/ranking.h` | API publica y contrato. Unico header que incluyen los demas modulos. |
| `src/ranking.c` | Logica, validacion, orden y persistencia binaria. |
| `src/demo_ranking.c` | Demo de fin de partida (maqueta de integracion). |
| `tests/test_ranking.c` | Pruebas automaticas sin frameworks externos. |
| `include/config.h` | `CONFIG_RANKING_RUTA` = `data/ranking.dat`. |
| `data/ranking.dat` | Datos en formato BNR1 v1. No editar a mano. |

Dependencias: solo `<stdio.h>`, `<string.h>` y `<ctype.h>`. No usa
`windows.h`, `unistd.h`, `conio.h` ni `stdint.h`; sigue siendo ANSI C
(C89). El destino de compilacion es Linux con GCC/Clang.

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
3. `nombre` conserva la primera grafia guardada, ya recortada.

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
| `RANKING_ERROR_ARGUMENTO` | 1 | Puntero NULL o struct inconsistente. |
| `RANKING_ERROR_NOMBRE_INVALIDO` | 2 | Nombre vacio, largo o con controles. |
| `RANKING_ERROR_LLENO` | 3 | Ya hay 100 jugadores. |
| `RANKING_ERROR_ARCHIVO` | 4 | No se pudo abrir, leer o escribir. |
| `RANKING_ERROR_FORMATO` | 5 | Estructura binaria invalida. |
| `RANKING_ERROR_INTEGRIDAD` | 6 | El CRC32 no coincide. |
| `RANKING_ERROR_VERSION` | 7 | Magic o version desconocidos. |

## 4. Recorrido de fin de partida

```
tablero detecta victoria -> app/ui obtiene el nombre del ganador ->
  ranking_inicializar(&r)
  rc = ranking_cargar(&r, CONFIG_RANKING_RUTA)
       ARCHIVO   -> primera partida: seguir con ranking vacio (no fatal)
       VERSION   -> ranking.dat viejo o de otra version: informar
       INTEGRIDAD-> archivo corrupto: informar, arrancar vacio
       FORMATO   -> datos invalidos: informar, arrancar vacio
  rc = ranking_registrar_victoria(&r, ganador)
       NOMBRE_INVALIDO / LLENO -> ui muestra ranking_mensaje_error(rc)
  rc = ranking_guardar(&r, CONFIG_RANKING_RUTA)
       ARCHIVO   -> ui informa "no se pudo guardar"
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

## 5. Formato BNR1 v1

Little-endian, offsets en bytes, sin padding implicito.

| Offset | Tamano | Campo |
|---:|---:|---|
| 0 | 4 | Magic: `'B' 'N' 'R' '1'` |
| 4 | 2 | Version: `1` (uint16 LE) |
| 6 | 2 | Cantidad de jugadores (uint16 LE, `0..100`) |
| 8 | `36*n` | Entradas, ordenadas de mejor a peor |
| `8+36*n` | 4 | CRC32 LE de todos los bytes anteriores |

Cada entrada de 36 bytes:

| Offset relativo | Tamano | Campo |
|---:|---:|---|
| +0 | 32 | Nombre: NUL + relleno de ceros |
| +32 | 4 | Victorias (uint32 LE, `0..1000000`) |

- Tamano total: `12 + 36*n`.
- Minimo (`n=0`): 12 bytes.
- Maximo (`n=100`): 3612 bytes.

La serializacion se hace byte a byte con `unsigned long` (al menos 32
bits en C89). No se escribe el `struct` directamente: el archivo no
depende del padding, del `sizeof(int)` ni del endianness de la maquina.

### CRC32

- Especificacion: IEEE 802.3.
- Polinomio reflejado: `0xEDB88320`.
- Valor inicial: `0xFFFFFFFF`.
- XOR final: `0xFFFFFFFF`.
- Se calcula sobre todos los bytes del archivo **excepto** los 4 del CRC.
- Valor de control: `CRC32("123456789") = 0xCBF43926`.

### Ejemplo real

`Ana` con 1 victoria genera un archivo de 48 bytes:

```
00000000: 424e 5231 0100 0100 416e 6100 0000 0000  BNR1....Ana.....
00000010: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000020: 0000 0000 0000 0000 0100 0000 9944 f1fd  .............D..
```

Lectura: `BNR1`, version 1, cantidad 1, `"Ana"` + relleno, victorias
`01 00 00 00` y CRC `fd f1 44 99` (little-endian).

## 6. Escritura atomica

`ranking_guardar()` no abre el archivo destino. Escribe primero
`ruta.tmp` en el mismo directorio y despues hace `rename()` sobre la
ruta final:

```
1. serializar todo en memoria y calcular CRC;
2. fopen("data/ranking.dat.tmp", "wb");
3. fwrite exacto + fclose comprobado;
4. rename("data/ranking.dat.tmp", "data/ranking.dat");
5. si algo falla: remove(tmp) y devolver RANKING_ERROR_ARCHIVO.
```

En Linux `rename()` reemplaza el destino de forma atomica dentro del
mismo filesystem. Si el proceso muere a mitad de escritura, el ranking
anterior queda intacto y solo se pierde el temporal. No se hace `fsync()`:
la garantia es frente a procesos interrumpidos, no frente a un corte de
energia.

## 7. Reglas de negocio

### Nombres

- Se recortan espacios de los extremos.
- Largo valido: 1 a 31 caracteres.
- Se rechazan caracteres de control (`< 32`): `\n`, `\r`, `\t`, etc.
- El `;` **si** se permite: en binario no hay separadores.
- La comparacion de identidad es insensible a mayusculas:
  `Ana == ANA == " ana "`.
- Se conserva la primera grafia guardada.
- Al leer se exige la forma canonica (sin espacios en los extremos) y
  relleno de ceros; un archivo con `"Ana "` o padding distinto de cero
  se rechaza con `RANKING_ERROR_FORMATO`.

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

## 8. Validacion al cargar

Estrategia **todo o nada**: se lee el archivo completo en un buffer
acotado (maximo 3612 bytes), se valida el CRC y recien despues se parsean
las entradas en un `Ranking` temporal. Ante cualquier error, el ranking
del llamador queda vacio; nunca queda a medio llenar.

Orden de validacion:

1. Tamano mayor al maximo -> `FORMATO`.
2. Magic `BNR1` y version 1 -> `VERSION`.
3. Cantidad `0..100` y tamano exacto `12 + 36*n` -> `FORMATO`.
4. CRC32 -> `INTEGRIDAD`.
5. Cada entrada: nombre canonico, victorias en rango, sin duplicados
   insensibles a mayusculas -> `FORMATO`.

## 9. Migracion desde el formato texto

La version anterior guardaba `nombre;victorias` en texto. Un
`data/ranking.dat` de ese formato no empieza con `BNR1`, por lo que
`ranking_cargar()` devuelve `RANKING_ERROR_VERSION`.

**No hay migracion automatica.** Como el archivo esta ignorado por git y
solo contiene victorias de prueba, la accion indicada es borrarlo:

```sh
rm -f data/ranking.dat data/ranking.dat.tmp
```

La demo avisa cuando detecta esta situacion.

## 10. Modelo de amenaza

El CRC32 detecta **corrupcion accidental**: un archivo truncado, un
sector defectuoso, una edicion torpe o una copia incompleta.

**No es seguridad.** El codigo fuente se entrega y el jugador es dueno
del archivo; cualquiera puede recompilar el formato y recalcular el CRC.
El anti-trampa real requeriria validacion en un servidor, fuera del
alcance de este trabajo. Esta distincion es deliberada y se documenta
para no prometer una garantia que el diseno no puede dar.

## 11. Casos limite cubiertos

- `ranking.dat` no existe -> vacio + `RANKING_ERROR_ARCHIVO`.
- Archivo de 0 bytes / menor al minimo -> `FORMATO`.
- Solo el magic `BNR1` -> `FORMATO` (truncado).
- Version distinta o magic alterado -> `VERSION`.
- Ranking de texto viejo -> `VERSION`.
- CRC alterado -> `INTEGRIDAD`.
- Byte extra, archivo truncado o cantidad mayor a 100 -> `FORMATO`.
- Nombre vacio, con control o no canonico -> `FORMATO`.
- Duplicados insensibles a mayusculas dentro del archivo -> `FORMATO`.
- Victorias fuera de rango -> `FORMATO`.
- `NULL` en cualquier puntero -> `ARGUMENTO`.
- Guardar con struct inconsistente -> `ARGUMENTO`, sin tocar el disco.
- Guardar en un directorio inexistente -> `ARCHIVO`; no queda `.tmp`.

## 12. Como probar solo este modulo

Desde la raiz del proyecto:

```sh
make            # compila build/test_ranking y build/demo_ranking
make test       # ejecuta las pruebas (132 aserciones)
make demo       # recuerda como correr la demo

rm -f data/ranking.dat
./build/demo_ranking "Ana"
xxd data/ranking.dat
./build/demo_ranking "ANA"
xxd data/ranking.dat      # Ana acumula 2 victorias
```

La suite verifica el formato con un CRC32 independiente del modulo, para
que un error en `ranking.c` no quede oculto por un roundtrip que use la
misma funcion para escribir y leer. Ver `tests/test_ranking.c`.

## 13. Integracion pendiente

`make app` compila el juego completo, pero a la fecha de este commit
`src/app.c`, `src/ui_cli.c`, `src/tablero.c` y `src/main.c` todavia no
compilan en `-ansi` (comentarios `//`, declaraciones dentro de bloques,
errores de sintaxis y retornos de variables locales). Esos archivos
pertenecen a otros integrantes y no se modificaron desde este modulo.

Mientras tanto, `make` y `make test` cubren el ranking de punta a punta.
