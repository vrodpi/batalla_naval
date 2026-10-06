# Contrato del modulo ranking

Este documento es el acuerdo de integracion entre `ranking.c` y el resto
del equipo (app / ui / game). Define funciones, parametros, retornos,
formato del archivo y decisiones de reglas pendientes.

## 1. Archivos

- `include/ranking.h`: API publica. Unico archivo que los demas modulos deben incluir.
- `src/ranking.c`: implementacion. Solo usa `<stdio.h>`, `<string.h>`, `<ctype.h>`.
- `include/config.h`: `CONFIG_RANKING_RUTA` (`data/ranking.dat`) y parametros del tablero.
- `data/ranking.dat`: archivo de datos (texto). No editar a mano salvo pruebas.

## 2. Datos

```c
#define RANKING_NOMBRE_MAX 32      /* incluye '\0': 31 caracteres utiles */
#define RANKING_MAX_JUGADORES 100
#define RANKING_RUTA_DEFAULT "data/ranking.dat"

typedef struct { char nombre[32]; int victorias; } RankingEntrada;
typedef struct { RankingEntrada entradas[100]; int cantidad; } Ranking;
```

El `Ranking` vive en la pila del llamador (app.c). No usa `malloc`, no hay
que liberarlo.

## 3. Funciones que usa la app / ui

```c
void ranking_inicializar(Ranking *r);
int ranking_cargar(Ranking *r, const char *ruta);
int ranking_registrar_victoria(Ranking *r, const char *nombre);
int ranking_guardar(const Ranking *r, const char *ruta);
void ranking_mostrar(const Ranking *r);
int ranking_mostrar_en(FILE *salida, const Ranking *r);
int ranking_cantidad(const Ranking *r);
const RankingEntrada *ranking_obtener(const Ranking *r, int indice);
int ranking_obtener_victorias(const Ranking *r, const char *nombre);
const char *ranking_mensaje_error(int codigo);
```

Códigos: `RANKING_OK (0)`, `RANKING_ERROR_NOMBRE_INVALIDO`,
`RANKING_ERROR_LLENO`, `RANKING_ERROR_ARCHIVO`, `RANKING_ERROR_FORMATO`.

## 4. Recorrido de fin de partida (quien llama a quien)

```
game.c detecta victoria -> app.c obtiene nombre ganador ->
  ranking_inicializar(&r) ->
  ranking_cargar(&r, CONFIG_RANKING_RUTA) ->
    si ARCHIVO: seguir con ranking vacio (avisar, no es fatal) ->
  ranking_registrar_victoria(&r, ganador) ->
    si NOMBRE_INVALIDO/LLENO: ui muestra ranking_mensaje_error(rc) ->
  ranking_guardar(&r, CONFIG_RANKING_RUTA) ->
    si ARCHIVO: ui informa "no se pudo guardar" ->
  ranking_mostrar(&r)  (o ui propia con cantidad/obtener para GUI) ->
  ui espera ENTER -> menu/salir
```

La interfaz JAMAS modifica `Ranking.entradas` directamente ni suma
victorias por su cuenta. Toda regla (validar, ordenar, fusionar) esta en
`ranking.c`.

Ejemplo minimo (ver `src/demo_ranking.c`):

```c
Ranking r;
int rc;
ranking_inicializar(&r);
rc = ranking_cargar(&r, CONFIG_RANKING_RUTA);
/* si rc == RANKING_ERROR_ARCHIVO: ranking vacio inicial, avisar */
rc = ranking_registrar_victoria(&r, ganador);
if (rc == RANKING_OK)
    ranking_guardar(&r, CONFIG_RANKING_RUTA);
ranking_mostrar(&r);
```

## 5. Formato portable de ranking.dat

- Texto, una entrada por linea: `nombre;victorias\n` (ej. `Ana;3`).
- Se abre en modo texto `"r"`/`"w"` para que Windows traduzca `\n` <-> `\r\n`.
- NO se usa `fwrite` del struct (no portable entre compiladores).
- Nombre: 1..31 caracteres tras recortar espacios; prohibido `;`.
- Victorias: entero `0..1000000`.
- Lineas en blanco se ignoran. Lineas malas se saltean y `cargar`
  devuelve `RANKING_ERROR_FORMATO` (conserva las validas).
- Duplicados insensibles a mayusculas dentro del archivo se fusionan sumando.
- `guardar` sobrescribe el archivo completo, ya ordenado.

## 6. Decisiones pendientes (propuesta del modulo ranking)

| Tema | Propuesta implementada |
|---|---|
| Orden del ranking | Victorias desc, luego nombre asc insensible. Desempate final sensible para determinismo. |
| Empates | Mismo criterio alfabetico; posiciones 1,2,3 sin saltos. |
| Mayusculas | `Ana` == `ANA` == ` ana `. Se conserva la primera grafia. |
| Barcos: orden de colocacion | Sugerido mayor a menor (5,4,3,2,1); ver `config.h`. No lo impone ranking. |
| Turno extra por impacto | No lo decide ranking. Sugerido: un disparo por turno (decide game.c). |
| Contacto entre barcos | No lo decide ranking. Sugerido: permitir contacto pero no superposicion (decide board.c). |

Si el grupo cambia alguna fila, solo hay que tocar `ranking_ordenar` /
`comparar_insensible` y este documento, sin tocar ui ni game.

## 7. Casos limite que ya maneja ranking.c

- `ranking.dat` no existe -> vacio + `RANKING_ERROR_ARCHIVO`.
- Nombre vacio / con `;` / >31 chars -> `RANKING_ERROR_NOMBRE_INVALIDO`.
- Mas de 100 jugadores -> `RANKING_ERROR_LLENO`.
- Archivo con lineas corruptas -> `RANKING_ERROR_FORMATO` + validas cargadas.
- `NULL` en cualquier puntero -> `RANKING_ERROR_ARGUMENTO`.

## 8. Como probar solo este modulo

```
make test
./build/test_ranking
./build/demo_ranking "Ana"
cat data/ranking.dat
```

Ver `tests/test_ranking.c` para la lista de casos.
