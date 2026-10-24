# Complejidades (Big-O)

`n` = elementos de la estructura, `k` = resultados devueltos, `V`/`E` = vértices/aristas del mapa,
`g` = grado de un vértice. Los valores salen de la implementación actual (ver los comentarios Doxygen de cada
cabecera).

## Estructuras propias (`hubflow::ds`)

| Estructura | Operación | Mejor | Peor / nota |
|---|---|---|---|
| DynamicArray | acceso por índice | O(1) | O(1), valida rango |
| | pushBack / popBack | O(1) | O(n) al redimensionar; O(1) amortizado (duplica capacidad) |
| SinglyLinkedList | pushFront / pushBack / popFront | O(1) | O(1) (mantiene puntero a la cola) |
| | removeFirstIf / findFirst | O(1) | O(n) |
| DoublyLinkedList | pushBack | O(1) | O(1) |
| | forEach / forEachReverse | O(n) | O(n) (reverso por `pPrev`, sin copiar) |
| Stack | push / pop / top | O(1) | O(1) |
| Queue | enqueue / dequeue / front / back | O(1) | O(1) (mantiene `m_pRear`) |
| BST | insert / find | O(1) | O(log n) balanceado; O(n) degenerado (claves ordenadas) |
| | inOrder / height | O(n) | O(n) |
| Árbol B+ (orden 4) | insert / find | O(log n) | O(log n), hojas a igual profundidad |
| | range | O(log n) | O(log n + k) por hojas enlazadas |
| BitSet | set / unset / isSet / isAnySet / isNoneSet | O(1) | O(1) |
| | AND / OR / XOR | O(1) | O(1), sobre la palabra entera |
| | count | O(k) | O(k), k = bits activos (Kernighan) |
| Set (lista ordenada) | insert / remove / contains | O(1) | O(n) |
| | unite / intersect / difference | O(n + m) | O(n + m), un solo merge |
| MinHeap | push / pop | O(log n) | O(log n) |
| | top | O(1) | O(1) |
| Graph | addVertex | O(1) amortizado | |
| | addEdge | O(1) | |
| | forEachNeighbor | O(g) | |
| | findVertex | O(V) | |
| | espacio | O(V + E) | |

## Dominio y casos de uso

| Operación | Mejor | Peor / nota |
|---|---|---|
| Registrar envío en pendientes (`PendingShipments::insert`) | O(1) | O(n): recorre hasta el último con prioridad mayor o igual |
| Despachar (`popFront`) | O(1) | O(1) |
| Quitar un pendiente concreto (`remove`) | O(1) | O(n) |
| Listar pendientes | O(n) | O(n) |
| Alta en el catálogo (`adopt`: lista + BST + B+) | O(log n) | O(n) si el BST degenera |
| Envío por código en catálogo (`require`, B+) | O(log n) | O(log n) |
| Buscar por código: lista (`index=list`) | O(1) | O(n) |
| Buscar por código: BST (`index=bst`) | O(1) | O(log n) esperado; O(n) degenerado |
| Buscar por código: B+ (`index=bplus`) | O(log n) | O(log n) |
| Rango de códigos en B+ | O(log n) | O(log n + k) |
| Resumen / más pesado de una zona (recursivo) | O(n) | O(n) tiempo, O(n) pila de llamadas |
| Agregar movimiento al historial | O(1) | O(1) |
| Leer historial asc / desc | O(n) | O(n) |
| Comparar historiales de estados (BitSet) | O(1) | O(1) |
| Armar conjunto por zona / servicio | O(n) | O(n²): cada alta en el Set es O(n) |
| Unión / intersección / diferencia de conjuntos | O(n + m) | O(n + m) |
| Abrir lote (copiar punteros de la zona) | O(n) | O(n) |
| Ordenar lote (Insertion Sort, estable) | O(n) ordenado | O(n²) inverso; espacio O(1) |
| Preparar cola (`prepare`) | O(n) | O(n) |
| Procesar siguiente / devolver el último | O(1) | O(1) |
| Búsqueda lineal en el lote | O(1) | O(n) |
| Búsqueda binaria en el lote (ordenado por código) | O(1) | O(log n) |

## Búsquedas del agente PUM

Cada algoritmo se ejecuta una vez por tramo. Cada nodo se expande a lo sumo una vez; cada arista genera a
lo sumo un nodo de búsqueda.

| Algoritmo | Frontera | Tiempo | Espacio | Óptimo en km | Completo |
|---|---|---|---|---|---|
| BFS | Queue (FIFO) | O(V + E) | O(V) | No (óptimo en saltos) | Sí |
| DFS | Stack (LIFO) | O(V + E) | O(V) | No | Sí (lista de explorados) |
| Greedy | MinHeap por h(n) | O((V + E) log V) | O(V + E) | No | Sí (lista de explorados) |
| A* | MinHeap por g(n) + h(n) | O((V + E) log V) | O(V + E) | Sí (h admisible y consistente) | Sí |

- La heurística h(n) es la distancia euclídea: O(1) por nodo.
- A* admite un sucesor solo si mejora el mejor costo conocido; puede dejar entradas obsoletas en el heap
  (se descartan al extraerlas), por eso el espacio es O(V + E).
- La traza completa que se exporta guarda una foto de la frontera y de los explorados en cada paso
  (`Frontier::snapshot`): agrega O(V) por paso, es decir O(V²) en total. En el mapa de referencia (15 nodos)
  es despreciable; para mapas grandes habría que registrar solo las diferencias.
- `RoadMap::indexOf` es O(V) (búsqueda lineal por id).
- Una ruta con m paradas resuelve m + 1 tramos (ida y vuelta al centro): m + 1 búsquedas.
