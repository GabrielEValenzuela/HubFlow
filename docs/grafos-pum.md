# Grafos y agente PUM

El PUM (Proveedor de Última Milla) es un **agente basado en objetivos**: recibe una lista de paradas y,
para cada tramo, busca un camino en el mapa de la ciudad. La formulación sigue la de la clase 2 de
Inteligencia Artificial (problema de búsqueda, como el simulador de Hanoi).

## Formulación del problema

| Elemento | En HubFlow |
|---|---|
| Estados | el nodo del mapa donde está el PUM |
| Estado inicial | el origen del tramo (el centro de distribución `HUB` para el primero) |
| Test de objetivo | el nodo actual es el destino del tramo (se aplica **al expandir**) |
| Acciones | moverse a un nodo adyacente |
| Modelo de transición | la arista lleva al nodo vecino |
| Costo de camino | suma de los km de las aristas |
| Solución | camino de nodos del origen al destino |

El mapa es un grafo **no dirigido y ponderado** (`ds::Graph`, listas de adyacencia). Un recorrido completo es
`HUB -> parada 1 -> ... -> parada n -> HUB`: n + 1 tramos, cada uno con su traza.

## Algoritmos

Todos comparten el esqueleto de `SearchStrategy::search` (Template Method): extraer un nodo de la frontera,
descartarlo si ya fue explorado, aplicar el test de objetivo **al expandir** y generar los sucesores. Cada
estrategia define la frontera, la prioridad y qué sucesores admite (Strategy). Es búsqueda **en grafo**: un
nodo se expande a lo sumo una vez. g(n) es la suma de km desde el origen, h(n) la distancia euclídea al
destino y f(n) = g(n) + h(n).

| Algoritmo | Frontera (estructura propia) | Prioridad | Completo | Óptimo | Tiempo / espacio (grafo) |
|---|---|---|---|---|---|
| BFS | `Queue` FIFO | ninguna (orden de llegada) | sí (grafo finito) | en cantidad de tramos, no en km | O(V + E) / O(V) |
| DFS | `Stack` LIFO | ninguna (se expande el último apilado) | sí (grafo finito, con explorados) | no | O(V + E) / O(V) |
| Greedy | `MinHeap` | h(n) | sí (grafo finito, con explorados) | no | O((V + E) log V) / O(V) |
| A* | `MinHeap` | f(n) = g(n) + h(n) | sí | en km, con h admisible y consistente | O((V + E) log V) / O(V + E) |

Los sucesores se generan en el orden de la lista de adyacencia. BFS, DFS y Greedy no re-insertan un nodo que ya
está en la frontera; A* sí lo hace si llega con menor g(n) (`bestCost`) y deja la entrada anterior obsoleta en el
heap, que se descarta al extraerla porque el nodo ya fue explorado. Desempate en el heap: menor prioridad
primero y, a igualdad, el nodo creado antes. Toda la búsqueda es determinista.

Las cotas son sobre el grafo (V nodos, E aristas). Las formas de árbol O(b^d) (BFS, A*) y O(b^m) (DFS) son la
referencia teórica sin conjunto de explorados. La traza copia frontera y explorados en cada paso, lo que
agrega O(V) por paso.

## Opciones de ruta (lenguaje de negocio)

El tablero no muestra nombres de algoritmos al operador ni al repartidor; la API conserva los valores técnicos
(`algorithm`: `BFS`, `DFS`, `GREEDY`, `ASTAR`). La correspondencia vive en `web/js/lib/routeOptions.js`:

| Opción en pantalla | `algorithm` | Descripción |
|---|---|---|
| Menos kilómetros (recomendada, por defecto) | `ASTAR` | La ruta más corta, garantizada |
| Menos cruces | `BFS` | Menos tramos (intersecciones); puede sumar km |
| Salida inmediata | `GREEDY` | Prioriza el punto más cercano al destino en línea recta; calcula rápido y puede sumar km |

`DFS` no se ofrece en el producto; solo aparece en la página de estudio `/lab/` (pestaña "Cómo decide el PUM"; ingreso de jefe de planta o administrador),
donde también están el simulador lado a lado, el reproductor paso a paso con las métricas y la descarga de
`initial_state.json` / `sequence.json`. Antes de asignar, la pantalla Armar carga compara las tres opciones con
`GET /route-previews` por tramo `HUB -> paradas -> HUB`, en el orden de descarga del lote (el último procesado
baja primero). El tiempo estimado supone 25 km/h promedio urbano + 4 min por parada (constante `TRAVEL_ESTIMATE`).

## Heurística euclídea admisible

h(n) es la distancia en línea recta entre el nodo y el destino, con las coordenadas (x, y) en km de cada
nodo. Es **admisible** porque el mapa garantiza un invariante: el costo de cada arista es mayor o igual que
la distancia euclídea entre sus extremos (`RoadMap::connect` rechaza las aristas más baratas que la línea
recta). Como cualquier camino es una poligonal que une los extremos, su costo nunca baja de la línea recta, así
que h nunca sobreestima. La desigualdad triangular además la vuelve **consistente**: h(n) ≤ c(n, n') + h(n').
Con h consistente, el primer pop de un nodo ya tiene su g mínimo, por lo que A* no necesita reabrir nodos
explorados y devuelve el camino de costo mínimo.

## API

- `GET /api/v1/road-map`: nodos y aristas.
- `GET /api/v1/route-previews?from=HUB&to=N2&algorithm=ASTAR`: traza de un tramo sin crear ruta
  (`algorithm`: `BFS`, `DFS`, `GREEDY`, `ASTAR`).
- `POST /api/v1/routes` con `{"batchId", "courierId", "algorithm"}`: asigna la ruta del lote procesado.
- `GET /api/v1/routes/:id/initial-state` y `/sequence`: los dos archivos descritos abajo.
- `PATCH /api/v1/routes/:id/stops/:order`: el PUM resuelve cada parada.

Al asignar una ruta, el suscriptor de exportación escribe ambos archivos en
`exports/route-<id>/` (carpeta compartida con el host).

## Formato de `initial_state.json`

Descripción del problema (qué se resuelve):

```json
{
  "routeId": 1,
  "courier": "Repartidor Uno",
  "algorithm": "ASTAR",
  "start": "HUB",
  "goals": ["N2", "N5"],
  "nodes": [{"id": "HUB", "label": "Centro de Distribución", "zone": "CENTRO", "isHub": true, "x": 10.0, "y": 10.0}],
  "edges": [{"from": "HUB", "to": "C1", "costKm": 3.0}]
}
```

`goals` son las paradas en orden de descarga; el recorrido vuelve a `start` al final.

## Formato de `sequence.json`

Cómo se resolvió, paso a paso, para animarlo (como la secuencia de movimientos de Hanoi):

```json
{
  "algorithm": "ASTAR",
  "segments": [
    {
      "from": "HUB",
      "to": "N2",
      "steps": [{"step": 1, "expanded": "HUB", "frontier": ["C3", "C1", "C2", "S1"], "explored": ["HUB"]}],
      "path": ["HUB", "C3", "N1", "N2"],
      "metrics": {"solution_found": true, "nodes_explored": 5, "states_visited": 5,
                  "nodes_in_frontier": 6, "max_depth": 3, "cost_total": 13.2}
    }
  ],
  "path": ["HUB", "C3", "N1", "N2", "..."],
  "metrics": {"cost_total": 40.5, "nodes_explored": 31}
}
```

- Cada `step` registra el nodo expandido, la frontera y los explorados **después** de expandirlo.
- `path` global concatena los caminos de los tramos sin repetir los nodos de unión.
- `metrics`: `nodes_explored` (expandidos), `states_visited` (distintos explorados), `nodes_in_frontier`
  (pendientes al terminar), `max_depth` (profundidad de la solución) y `cost_total` en km.

(Los valores del ejemplo son ilustrativos; el formato es el de los archivos reales.)

## Mapa de referencia

15 nodos (`HUB`, `C1..C4`, `N1..N5`, `S1..S5`) y 28 aristas en `db/migrations/002_road_map.sql`. Los
paquetes iniciales se entregan en `C1, N2, S3, C2, N3, S5, C4, N5`.
