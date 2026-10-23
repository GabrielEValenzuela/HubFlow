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

Todos comparten el esqueleto de `SearchStrategy::search` (Template Method); cada estrategia solo decide la
frontera, la prioridad y qué sucesores admite (Strategy). Se evita repetir estados con un registro de explorados.

| Algoritmo | Frontera | Prioridad | Propiedades |
|---|---|---|---|
| BFS | cola FIFO | orden de llegada | óptimo en cantidad de saltos, no en km |
| DFS | pila LIFO | orden inverso | encuentra una solución, no necesariamente corta |
| Greedy | heap | h(n) | expande poco, no es óptimo |
| A* | heap | f(n) = g(n) + h(n) | óptimo en km si h es admisible |

Desempate en el heap: menor prioridad primero y, a igualdad, el nodo creado antes. Las aristas se recorren en
el orden en que se agregaron, por lo que toda la búsqueda es determinista.

## Heurística euclídea admisible

h(n) es la distancia en línea recta entre el nodo y el destino, con las coordenadas (x, y) en km de cada
nodo. Es **admisible** porque el mapa garantiza un invariante: el costo de cada arista es mayor o igual que
la distancia euclídea entre sus extremos (`RoadMap::connect` rechaza las aristas más baratas que la línea
recta). Como cualquier camino es una poligonal que une los extremos, su costo nunca baja de la línea recta, así
que h nunca sobreestima. La desigualdad triangular además la vuelve **consistente**, y con ella A* no necesita
reabrir nodos y devuelve el camino de costo mínimo.

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
