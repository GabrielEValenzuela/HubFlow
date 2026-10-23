# Trazabilidad: enunciado del TP -> código y tests

Este es el único lugar donde se nombran los requerimientos del enunciado (RF01..RF14, "Caso N").
El código y los tests describen el comportamiento de negocio sin referirse a ellos.

Convenciones de rutas (todas bajo `src/hubflow/` salvo que se indique otra cosa):

- `<ctx>/domain/...` reglas de negocio y estructuras de dominio.
- `<ctx>/application/ports/in/...` puertos de entrada (un caso de uso por interfaz).
- `<ctx>/application/usecases/{commands,queries}/...` implementación de cada caso de uso.
- `src/datastructures/...` estructuras de datos propias (`hubflow::ds`).
- Los tests están en `tests/unit/{ds,domain}` y `tests/application`.

## Requerimientos funcionales

| Req. | Qué pide | Dónde se resuelve | Tests |
|---|---|---|---|
| RF01 | Registrar un envío (código único, nodo en la lista, orden por prioridad estable, movimiento inicial) | `logistics/domain/services/receptionService`, `logistics/domain/services/pendingShipments` (`insert`), `logistics/domain/entities/shipment` (`receive`); caso de uso `logistics/application/usecases/commands/registerShipment` | `PendingShipmentsTest.OrdersByPriority`, `PendingShipmentsTest.KeepsArrivalOrderWithinSamePriority`, `LogisticsFlow.RegisteringProjectsPersistsAndAudits`, `LogisticsFlow.RejectsDuplicatesForeignDeliveryPointsAndForbiddenRoles` |
| RF02 | Mostrar pendientes en el orden de la lista | `logistics/domain/services/pendingShipments` (`forEach`); `logistics/application/usecases/queries/listPendingShipments` | `PendingShipmentsTest.OrdersByPriority`, `LogisticsFlow.DispatchRescheduleAndDeliverKeepPendingConsistent` |
| RF03 | Buscar un envío por código recorriendo la estructura enlazada | `logistics/domain/services/shipmentLookup` (`LinearShipmentLookup`); `logistics/application/usecases/queries/findShipment`, `getShipment` | `ShipmentLookupTest.StrategiesCompareVisitedNodes`, `LogisticsFlow.LookupsOverAllStructuresAgree` |
| RF04 | Cambiar estado generando un movimiento al final del historial | `logistics/domain/entities/shipment` (`classify`, máquina de estados), `logistics/domain/entities/movementHistory`; `logistics/application/usecases/commands/classifyShipment` | `Shipment.EnforcesStateMachineAndTracksStateBits`, `Shipment.RaisesOneEventPerBehaviour` |
| RF05 | Despachar el primer pendiente sin destruir el envío | `shipment.dispatch`, `PendingShipments::remove/popFront`, el catálogo es dueño de los envíos (`logistics/domain/services/shipmentCatalog`); `logistics/application/usecases/commands/dispatchNextShipment`; proyección en `logistics/application/subscribers/pendingShipmentsProjector` | `PendingShipmentsTest.DispatchRemovesNodeButKeepsShipment`, `LogisticsFlow.DispatchOnEmptyPendingIsConflict` |
| RF06 | Reprogramar: suma intento, vuelve a pendientes por prioridad | `shipment.reschedule`, `PendingShipments::insert`; `logistics/application/usecases/commands/rescheduleShipment` | `PendingShipmentsTest.RescheduleIncrementsAttemptsAndReinsertsByPriority`, `LogisticsFlow.DispatchRescheduleAndDeliverKeepPendingConsistent` |
| RF07 | Finalizar entrega (estado final, no vuelve a pendientes) | `shipment.deliver`; `logistics/application/usecases/commands/deliverShipment` | `LogisticsFlow.DispatchRescheduleAndDeliverKeepPendingConsistent` |
| RF08 | Historial bidireccional (lista doblemente enlazada) | `logistics/domain/entities/movementHistory`, `datastructures/doublyLinkedList.hpp` | `ShipmentHistoryTest.ReadsForwardAndBackward`, `DoublyLinkedList.TraversesForwardAndBackwardWithPrevLinks`, `DoublyLinkedList.SingleNodeIsHeadAndTail` |
| RF09.1 | Lote por zona en un arreglo dinámico de punteros | `sorting/domain/entities/sortingBatch` (constructor); `sorting/application/usecases/commands/openSortingBatch` | `SortingBatchTest.SortsByCodeAndByWeightKeepingReferences`, `SortingBatchTest.EmptyZoneIsReported` |
| RF09.2 | Ordenar manualmente por código o por peso | `sorting/domain/services/insertionSort`, `sorting/domain/services/shipmentOrdering`; `sorting/application/usecases/commands/sortSortingBatch` | `SortingBatchTest.SortsByCodeAndByWeightKeepingReferences`, `SortingBatchTest.InsertionSortBestCaseIsLinear`, `SortingFlow.SortingTwiceWithTheSameCriteriaKeepsTheSameOrder` |
| RF09.3 | Cola de preparación FIFO | `datastructures/queue.hpp`, `SortingBatch::prepare`; `sorting/application/usecases/commands/prepareSortingBatch` | `Queue.IsFifoAndUpdatesFrontAndRear`, `Queue.ThrowsOnEmptyAccess`, `SortingBatchTest.PreparationQueueIsFifoAndProcessedStackIsLifo` |
| RF09.4 | Pila de procesados LIFO | `datastructures/stack.hpp`, `SortingBatch::processNext/returnProcessed`; `sorting/application/usecases/commands/processSortingBatchItem`, `returnProcessedItem` | `Stack.IsLifo`, `SortingBatchTest.PreparationQueueIsFifoAndProcessedStackIsLifo`, `SortingBatchTest.ReturnProcessedItemOnlyAcceptsTheLastProcessedCode`, `SortingFlow.*` |
| RF09.5 | Búsqueda lineal en el lote | `sorting/domain/services/batchSearch` (`LinearBatchSearch`); `sorting/application/usecases/queries/linearSearchInSortingBatch` | `BatchSearchTest.LinearAndBinaryReportComparisons` |
| RF09.6 | Búsqueda binaria sólo con el lote ordenado por código | `BinaryBatchSearch`; `sorting/application/usecases/queries/binarySearchInSortingBatch` | `BatchSearchTest.LinearAndBinaryReportComparisons` |
| RF09.7 | Comparar comparaciones de ambas búsquedas | `sorting/application/usecases/queries/compareSortingBatchSearches` | `BatchSearchTest.LinearAndBinaryReportComparisons` |
| RF09.8 | Ownership no propietario del lote, la cola y la pila | Documentado en `sorting/domain/entities/sortingBatch.hpp`; destructores en `datastructures/{queue,stack}.hpp` | `SortingBatchTest.DeskReplacesActiveBatch`, `SortingBatchTest.SortsByCodeAndByWeightKeepingReferences` |
| Recursividad | Resumen recursivo por zona sobre los nodos de pendientes (y envío más pesado como desafío) | `logistics/domain/services/zoneReport` (`summarizeZone`, `heaviestInZone`); `logistics/application/usecases/queries/summarizeZone`, `findHeaviestShipment` | `PendingShipmentsTest.RecursiveZoneSummaryCountsWeightAndExpress` |
| RF10 | Índice BST por código, búsqueda con camino, in-order, altura | `datastructures/binarySearchTree.hpp`, `ShipmentCatalog::bst`; `insights/application/usecases/queries/lookupInBst`, `getBst` | `BinarySearchTree.InsertsRejectsDuplicatesAndTraversesInOrder`, `BinarySearchTree.FindReportsPathAndVisitedNodes`, `BinarySearchTree.HeightConventionAndDegeneration` |
| RF11 | Índice B+ (orden 4), búsqueda y rango por hojas enlazadas | `datastructures/bPlusTree.hpp`, `ShipmentCatalog::bPlusTree`; `insights/application/usecases/queries/lookupInBPlusTree`, `rangeInBPlusTree`, `getBPlusTree` | `BPlusTree.*` |
| RF12 | BitSet con los estados por los que pasó cada envío y su comparación | `datastructures/bitSet.hpp`, `logistics/domain/entities/shipment` (`StateHistory`), `insights/domain/services/stateComparison`; `insights/application/usecases/queries/compareShipmentStates` | `BitSet.SetResetTestCountAndAny`, `BitSet.BitwiseOperators`, `StateComparison.UsesBitwiseOperations`, `Shipment.EnforcesStateMachineAndTracksStateBits` |
| RF13 | Set propio y conjuntos por zona/servicio con unión, intersección y diferencia | `datastructures/set.hpp`, `insights/domain/services/codeSet`; `insights/application/usecases/queries/getCodeSet`, `combineCodeSets` | `Set.KeepsUniqueSortedElements`, `Set.UnionIntersectionAndDifference`, `Set.EdgeCasesWithEmptyAndDisjointSets`, `CodeSets.BuildsByZoneAndServiceAndCombines`, `InsightsFlow.CombinesSetsAndResolvesShipmentsThroughBPlus` |
| RF14 | Consulta avanzada integrada (B+, rango, BitSet, conjuntos) | Casos de uso del contexto `insights` (ver RF10..RF13) | `InsightsFlow.*` |

## Casos de prueba del enunciado base

| Caso | Tests |
|---|---|
| 1 Prioridades | `PendingShipmentsTest.OrdersByPriority` |
| 2 Prioridad estable | `PendingShipmentsTest.KeepsArrivalOrderWithinSamePriority` |
| 3 Despacho | `PendingShipmentsTest.DispatchRemovesNodeButKeepsShipment` |
| 4 Reprogramación | `PendingShipmentsTest.RescheduleIncrementsAttemptsAndReinsertsByPriority` |
| 5 Historial | `ShipmentHistoryTest.ReadsForwardAndBackward` |
| 6 Recursividad | `PendingShipmentsTest.RecursiveZoneSummaryCountsWeightAndExpress` |
| 7 Casos límite | `PendingShipmentsTest.HandlesEmptyDuplicatedAndSingleElementCases`, `SinglyLinkedList.PopOnEmptyListReturnsFalse` |
| 8 Sorting del lote | `SortingBatchTest.SortsByCodeAndByWeightKeepingReferences`, `SortingBatchTest.InsertionSortBestCaseIsLinear`, `SortingBatchTest.EmptyZoneIsReported`, `SortingBatchEdgeCases.SingleElementBatchSortsTrivially` |
| 9 Queue de preparación | `Queue.IsFifoAndUpdatesFrontAndRear`, `Queue.ThrowsOnEmptyAccess`, `SortingBatchTest.PreparationQueueIsFifoAndProcessedStackIsLifo` |
| 10 Stack de procesados | `Stack.IsLifo`, `SortingBatchTest.PreparationQueueIsFifoAndProcessedStackIsLifo`, `SortingBatchTest.ReturnProcessedItemOnlyAcceptsTheLastProcessedCode` |
| 11 Búsqueda lineal y binaria | `BatchSearchTest.LinearAndBinaryReportComparisons` |
| 12 BST / ABB | `BinarySearchTree.InsertsRejectsDuplicatesAndTraversesInOrder`, `BinarySearchTree.FindReportsPathAndVisitedNodes`, `BinarySearchTree.HeightConventionAndDegeneration`, `ShipmentLookupTest.StrategiesCompareVisitedNodes` |

## Casos de prueba de la extensión (B+, BitSet, Set)

| Caso | Tests |
|---|---|
| 1 Árbol B+ | `BPlusTree.SplitsLeavesAndInternalNodesKeepingLeavesAtSameDepth`, `BPlusTree.RootSplitsAfterFourthKey` |
| 2 Búsqueda por rango | `BPlusTree.RangeUsesLeafLinksAndReturnsAscendingKeys` |
| 3 BitSet | `BitSet.SetResetTestCountAndAny`, `BitSet.BitwiseOperators`, `StateComparison.UsesBitwiseOperations` |
| 4 Set | `Set.KeepsUniqueSortedElements`, `Set.UnionIntersectionAndDifference`, `CodeSets.BuildsByZoneAndServiceAndCombines` |
| 5 Casos límite | `BPlusTree.EmptyTreeQueriesAreSafe`, `Set.EdgeCasesWithEmptyAndDisjointSets` |
| 6 Integración | `InsightsFlow.NewShipmentIsReachableThroughEveryStructure`, `InsightsFlow.CombinesSetsAndResolvesShipmentsThroughBPlus` |

## Grafos y agente PUM (funcionalidad adicional)

| Tema | Dónde | Tests |
|---|---|---|
| Mapa de la ciudad como grafo no dirigido ponderado | `datastructures/graph.hpp`, `routing/domain/entities/roadMap` | `Graph.StoresUndirectedWeightedEdges`, `RoutingFixture.MapRejectsEdgesCheaperThanStraightLine` |
| Búsquedas BFS, DFS, Greedy y A* (Strategy + Template Method) y su frontera | `routing/domain/services/search/{searchStrategy,frontier,searchNodePool}`, `datastructures/minHeap.hpp` | `RoutingFixture.BreadthFirstFindsFewestHops`, `RoutingFixture.AStarIsOptimalAndNeverWorseThanOthers`, `RoutingFixture.DepthFirstAndGreedyReturnValidPaths`, `RoutingFixture.TraceRecordsFrontierAndExploredPerStep`, `RoutingFixture.StartEqualsGoalAndUnreachableGoal`, `MinHeap.PopsInPriorityOrder` |
| Agente PUM y recorrido ida y vuelta al centro | `routing/domain/services/pumAgent` | `RoutingFixture.DeliveryRoutePlansRoundTripAndResolvesStops` |
| Hoja de ruta, paradas y resolución idempotente | `routing/domain/entities/deliveryRoute`, `routing/application/usecases/commands/{assignDeliveryRoute,resolveRouteStop}` | `RoutingFixture.ResolvingAStopWithTheSameOutcomeIsANoOp`, `RoutingFlow.*` |
| Estado inicial y secuencia exportables (formato del simulador) | `routing/domain/services/search/searchTrace.hpp` (traza completa); exportación a archivos JSON: etapa de adaptadores | (pendiente con los adaptadores) |

## Idempotencia

La tabla completa de casos de uso y su idempotencia está en la documentación de cada puerto de entrada
(`@par Idempotencia`) y se verifica en `IdempotencyContract.*`.
