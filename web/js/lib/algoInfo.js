export const ALGORITHM = { BFS: "Amplitud (BFS)", DFS: "Profundidad (DFS)", GREEDY: "Voraz (Greedy)", ASTAR: "A*" };
export const ALGORITHMS = Object.keys(ALGORITHM);

export const SEARCH_FORMULATION = [
  ["Estado", "nodo del mapa donde está el PUM."],
  ["Estado inicial / objetivo", "origen y destino del tramo (HUB o una parada)."],
  ["Acciones", "moverse por un tramo (arista) a un nodo adyacente. El grafo es no dirigido."],
  ["Costo", "km del tramo. g(n) es la suma de los km del camino de la raíz a n."],
  ["Heurística", "h(n) = distancia euclídea de n al objetivo. f(n) = g(n) + h(n)."],
  ["Búsqueda", "en grafo, con registro de explorados por nodo: un nodo se expande a lo sumo una vez."],
  ["Test de objetivo", "al expandir (al extraer de la frontera), no al generar el sucesor."],
  ["Esqueleto", "SearchStrategy::search (Template Method): extraer de la frontera, descartar si ya fue explorado, testear objetivo, generar sucesores. Cada estrategia define la frontera, la prioridad y qué sucesores admite (Strategy)."],
];

export const ALGORITHM_INFO = [
  {
    key: "BFS", name: "BFS (amplitud)", business: "Menos cruces",
    frontier: "Queue FIFO propia", file: "datastructures/queue.hpp (FifoFrontier)",
    tie: "Sin prioridad. Los sucesores se encolan en el orden de la lista de adyacencia; un nodo ya presente en la frontera no se vuelve a encolar.",
    order: "Expande los nodos por profundidad creciente (cantidad de tramos desde el origen), y dentro de cada nivel en orden de llegada.",
    complete: "Sí, si el grafo es finito (con explorados no hay ciclos infinitos).",
    optimal: "Óptimo en cantidad de tramos, que equivale a costo unitario por arista. No lo es en km: el camino con menos tramos puede sumar más km.",
    time: "O(V + E)", space: "O(V)", tree: "Árbol: O(b^d) en tiempo y espacio, con b = factor de ramificación y d = profundidad de la solución.",
    watch: "La frontera crece como un anillo por nivel de profundidad y el área explorada se expande en todas las direcciones, también hacia el lado opuesto al destino.",
    metric: "nodes_explored y nodes_in_frontier: suelen ser los más altos de los cuatro. max_depth es el mínimo posible.",
  },
  {
    key: "DFS", name: "DFS (profundidad)", business: "No se ofrece al operador",
    frontier: "Stack LIFO propia", file: "datastructures/stack.hpp (LifoFrontier)",
    tie: "Sin prioridad. Los sucesores se apilan en el orden de la lista de adyacencia, por lo que se expande primero el último apilado. Un nodo ya presente en la pila no se vuelve a apilar.",
    order: "Expande siempre el último nodo generado: baja por una rama hasta quedarse sin sucesores no explorados y recién entonces retrocede.",
    complete: "Sí, si el grafo es finito y se usa el registro de explorados. Sin él, un ciclo lo haría no terminar.",
    optimal: "No. Devuelve el primer camino que alcanza el objetivo, que puede ser arbitrariamente largo en tramos y en km.",
    time: "O(V + E)", space: "O(V)", tree: "Árbol: O(b^m) en tiempo, con m = profundidad máxima; O(b·m) en espacio. Con explorados la implementación usa O(V).",
    watch: "Expande siempre el último nodo apilado: avanza por una rama hasta que no quedan vecinos sin explorar y recién entonces vuelve a nodos apilados antes. El camino final no tiene por qué ser el de menos tramos ni el de menos km.",
    metric: "max_depth y cost_total: suelen ser los más altos. nodes_explored depende del orden de las aristas, no de la ubicación del destino.",
  },
  {
    key: "GREEDY", name: "Greedy best-first (voraz)", business: "Salida inmediata",
    frontier: "MinHeap propio ordenado por h(n)", file: "datastructures/minHeap.hpp (PriorityFrontier)",
    tie: "Menor h(n) primero; a igual h(n), el nodo creado antes (campo order). Un nodo ya presente en la frontera no se vuelve a insertar, aunque aparezca por un camino más barato.",
    order: "Expande el nodo de la frontera con menor distancia euclídea al objetivo, sin considerar lo ya recorrido.",
    complete: "Sí, si el grafo es finito y se usa el registro de explorados.",
    optimal: "No. g(n) no interviene en la prioridad, así que puede entrar por un camino largo cuyo último nodo está cerca del destino.",
    time: "O((V + E) log V)", space: "O(V)", tree: "Árbol: O(b^m) en el peor caso; en la práctica depende de la calidad de h.",
    watch: "Avanza en línea casi recta hacia el destino. Si hay un obstáculo (nodo cercano sin salida), explora ese entorno y puede rodear de más.",
    metric: "nodes_explored suele ser bajo, pero cost_total puede superar al de A*. Comparar ambos en el mismo tramo.",
  },
  {
    key: "ASTAR", name: "A*", business: "Menos kilómetros",
    frontier: "MinHeap propio ordenado por f(n) = g(n) + h(n)", file: "datastructures/minHeap.hpp (PriorityFrontier)",
    tie: "Menor f(n) primero; a igual f(n), el nodo creado antes (campo order). Un sucesor se vuelve a insertar si llega con g(n) menor que bestCost.",
    order: "Expande el nodo de la frontera con menor costo total estimado f(n) = g(n) + h(n): km recorridos más línea recta restante.",
    complete: "Sí, si el grafo es finito y los costos son positivos.",
    optimal: "Sí, en km. Cada arista cuesta al menos la línea recta entre sus extremos (invariante de RoadMap::connect), y todo camino es una poligonal, así que h(n) nunca sobreestima (admisible) y cumple h(n) ≤ c(n, n') + h(n') (desigualdad triangular, consistente). Con h consistente, el primer pop de un nodo ya tiene su g mínimo, y el test al expandir devuelve el camino de menor costo. La implementación guarda bestCost por nodo: un sucesor con menor g se re-inserta en el heap y la entrada anterior queda obsoleta; al extraerla se descarta porque el nodo ya fue explorado. Nunca reabre un nodo explorado.",
    time: "O((V + E) log V)", space: "O(V + E)", tree: "Árbol: O(b^d) en el peor caso; el exponente efectivo baja con una h más informada.",
    watch: "Sólo expande nodos con f(n) ≤ C*, el costo óptimo, por lo que concentra la exploración entre origen y destino. Su cost_total es siempre el mínimo de los cuatro; comparar nodes_explored contra BFS en el mismo tramo (suele ser menor, pero no está garantizado).",
    metric: "cost_total (mínimo) y nodes_explored frente a BFS. nodes_in_frontier no cuenta entradas obsoletas del heap.",
  },
];

export const ALGO_NOTES = [
  "V = nodos, E = aristas del mapa. Las cotas de tiempo cuentan expansiones, generación de sucesores y operaciones de frontera: Queue y Stack son O(1) por operación; MinHeap push/pop es O(log n). En A* el heap tiene hasta 2E entradas (cada arista no dirigida se recorre en ambos sentidos), y log E = O(log V).",
  "Las cotas de árbol (b^d, b^m) son la referencia teórica sin conjunto de explorados; no describen esta implementación.",
  "La traza copia la frontera y los explorados en cada paso: guardarla cuesta O(V) por paso, aparte del costo de la búsqueda.",
];
