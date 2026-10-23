# HubFlow {#mainpage}

HubFlow es una API REST en C++17 para gestionar envíos de última milla. Es un proyecto de referencia de
la cátedra de Algoritmos y Estructuras de Datos: las estructuras (listas, pila, cola, BST, B+, BitSet,
Set, heap, grafo) son propias y están en `hubflow::ds`; el dominio usa DDD y una arquitectura
hexagonal por contexto.

## Contextos

- `hubflow::logistics`: recepción, pendientes por prioridad, despacho, historial de movimientos.
- `hubflow::sorting`: mesa de clasificación (lote por zona, ordenamiento, cola, pila, búsquedas).
- `hubflow::routing`: mapa de la ciudad (grafo) y agente PUM con BFS, DFS, Greedy y A*.
- `hubflow::identity`: usuarios, roles y sesiones JWT.
- `hubflow::insights`: índices (BST, B+), comparación de estados con BitSet y conjuntos.
- `hubflow::audit`: bitácora de eventos de dominio.
- `hubflow::shared`: núcleo compartido (errores de negocio, eventos, bus, idempotencia).

## Documentación complementaria

Los documentos en `docs/` (arquitectura, ownership, complejidades, grafos, idempotencia y trazabilidad)
se leen en el repositorio; este sitio documenta el código con Doxygen.
