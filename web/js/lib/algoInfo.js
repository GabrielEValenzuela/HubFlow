// Vocabulario técnico: solo lo usa la vista "Cómo decide el PUM".
export const ALGORITHM = { BFS: "Amplitud (BFS)", DFS: "Profundidad (DFS)", GREEDY: "Voraz (Greedy)", ASTAR: "A*" };
export const ALGORITHMS = Object.keys(ALGORITHM);

export const ALGORITHM_INFO = [
  { key: "BFS", name: "BFS (amplitud)", business: "Menos cruces", text: "Explora por capas: primero todo lo que está a una calle, después a dos, y así. Encuentra el camino con menos intersecciones, aunque no siempre el más corto en km." },
  { key: "DFS", name: "DFS (profundidad)", business: "No se ofrece al operador", text: "Se interna por un camino hasta el final y recién entonces prueba otro. Encuentra un camino, pero suele ser largo; sirve para comparar." },
  { key: "GREEDY", name: "Greedy (voraz)", business: "Salida inmediata", text: "Siempre avanza hacia el nodo que parece más cercano al destino en línea recta. Calcula muy rápido, pero puede dar vueltas de más." },
  { key: "ASTAR", name: "A*", business: "Menos kilómetros", text: "Suma lo recorrido más lo que falta en línea recta y expande lo más prometedor. Con esta estimación nunca se pasa, por eso garantiza el camino más corto en km." },
];
