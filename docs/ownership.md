# Ownership: quién crea y destruye cada objeto

Regla general: **un único dueño** por objeto; el resto guarda punteros no propietarios (`T*` o `const T*`
que nunca se pasan a `delete`). Solo `datastructures/` y `domain/` administran memoria con `new`/`delete`.

## Dueños

| Objeto | Lo crea | Lo destruye | Quién lo referencia sin poseerlo |
|---|---|---|---|
| `Shipment` | el caso de uso o la hidratación (`new`), entregado a `ShipmentCatalog::adopt` | `~ShipmentCatalog` | `PendingNode`, nodos del BST y del B+, `SortingBatch`, `Set` de códigos |
| nodos de la lista del catálogo, del BST y del B+ | cada estructura | el destructor de la estructura (solo sus nodos) | |
| `PendingNode` | `PendingShipments::insert` | `popFront`, `remove`, `~PendingShipments` (nunca borra el `Shipment`) | |
| `Movement` / nodos del historial | `MovementHistory` (lista doble) | `~MovementHistory` | |
| eventos de dominio | el agregado (`raise`, con `new`) | el agregado, tras `publishEvents` o en su destructor | los suscriptores los reciben por referencia |
| `SortingBatch` | `SortingDesk` | `SortingDesk` (reemplaza el lote activo) | |
| arreglo `Shipment**` del lote | `SortingBatch` (`new[]`) | `~SortingBatch` (`delete[]`) | |
| nodos de la `Queue` de preparación y de la `Stack` de procesados | las propias estructuras | sus destructores | punteros a `Shipment` del catálogo |
| `RoadMap`, `DeliveryRoute` | `RoutingModel` / `RouteBook` | su dueño al destruirse el modelo | |
| `SearchNode` | `SearchNodePool` (uno por búsqueda) | el pool al terminar la búsqueda | `Frontier` guarda punteros no propietarios |
| `Frontier` | la estrategia (`createFrontier`, ownership al llamador) | `FrontierOwner` dentro de `search` | |
| nodos del `Graph` | `Graph` | `~Graph` | |

## Adaptadores y composition root

- `bootstrap::Application` es dueña de todo el grafo de objetos de la aplicación (bus, modelos,
  repositorios, casos de uso, controllers) por valor o `std::unique_ptr`; los destruye en orden inverso.
- Las conexiones MySQL y los recursos de libsodium/OpenSSL se envuelven en `std::unique_ptr` con deleters
  propios.
- Los casos de uso y suscriptores guardan **referencias** a puertos y modelos que viven más que ellos.

## Por qué los envíos no se borran

Los envíos nunca se eliminan mientras el sistema vive. Así ningún puntero no propietario (pendientes, lote,
cola, pila, conjuntos, índices) puede quedar colgando, y despachar un envío solo quita su nodo de pendientes.
El lote copia punteros, no objetos: ordenar el lote reordena punteros y no mueve ningún `Shipment`.

Las estructuras son no copiables (`= delete`) para evitar un doble `delete`.
