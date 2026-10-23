# Idempotencia

Cada puerto de entrada (`ports/in`) declara `static constexpr Idempotency IDEMPOTENCY` y documenta con
`@par Idempotencia` por qué lo es o no. Todas las consultas (`GET`) son idempotentes y sin efectos.

## Comandos

| Caso de uso | Endpoint | Idempotente | Cómo se maneja |
|---|---|---|---|
| SignIn | `POST /sessions` | No (token nuevo) | sin `Idempotency-Key` |
| RegisterUser | `POST /users` | No | clave natural `email` (repetir = 409) + `Idempotency-Key` |
| UpdateUser | `PATCH /users/:id` | Sí | mismo cuerpo, mismo estado final |
| RegisterShipment | `POST /shipments` | No | clave natural `code` (repetir = 409) + `Idempotency-Key` |
| Classify / Reschedule / Deliver | `POST /shipments/:code/movements` | No (agrega un movimiento) | `Idempotency-Key` |
| DispatchNextShipment | `POST /dispatches` | No | `Idempotency-Key` |
| OpenSortingBatch | `POST /sorting-batches` | No | `Idempotency-Key` |
| SortSortingBatch | `PATCH /sorting-batches/:id` | Sí | mismo criterio, mismo orden |
| PrepareSortingBatch | `PUT /sorting-batches/:id/preparation-queue` | Sí | recarga la cola desde el lote |
| ProcessSortingBatchItem | `POST /sorting-batches/:id/processed-items` | No | `Idempotency-Key` |
| ReturnProcessedItem | `DELETE /sorting-batches/:id/processed-items/:code` | Sí | solo si `code` es el tope; repetir devuelve 404 sin efecto |
| AssignDeliveryRoute | `POST /routes` | No | `Idempotency-Key` |
| ResolveRouteStop | `PATCH /routes/:id/stops/:order` | Sí | mismo resultado = 200 sin cambios; otro resultado = 409 |
| ReassignDeliveryRoute | `PUT /routes/:id/courier` | Sí | pasar la ruta al chofer que ya la tiene = 200 sin eventos |
| CreateZone / CreateServiceLevel / CreateShipmentFeature | `POST /zones`, `/service-levels`, `/shipment-features` | No | clave natural `code` (repetir = 409) + `Idempotency-Key` |
| UpdateZone / UpdateServiceLevel / UpdateShipmentFeature | `PATCH /zones/:code`, `/service-levels/:code`, `/shipment-features/:code` | Sí | estado final deseado; repetir = 200 sin eventos |
| AddRoadNode | `POST /road-nodes` | No | clave natural `id` (repetir = 409) + `Idempotency-Key` |
| UpdateRoadNode | `PATCH /road-nodes/:id` | Sí | estado final deseado; repetir = 200 sin eventos |

## Header `Idempotency-Key`

Se acepta en los `POST` no idempotentes autenticados (1 a 80 caracteres).

1. Sin header: el pedido se ejecuta normalmente.
2. Con header y clave **nueva** para ese usuario: se ejecuta y, si la respuesta fue 2xx, se guarda
   (estado, cuerpo y `Location`) en `idempotency_keys`.
3. Con header y clave **ya usada** por el mismo usuario, método y ruta: se devuelve la respuesta guardada
   sin ejecutar nada, con el header `Idempotent-Replayed: true`.
4. Misma clave con otro método o ruta: `422` (problem+json con el campo `Idempotency-Key`).
5. Header vacío o de más de 80 caracteres: `422`.

Las claves son por usuario (`PK(user_id, idem_key)`). Las respuestas de error no se guardan, así que se
puede reintentar con la misma clave tras corregir el pedido.

```bash
curl -X POST $URL/api/v1/dispatches -H "Authorization: Bearer $TOKEN" -H "Idempotency-Key: despacho-001"
curl -X POST $URL/api/v1/dispatches -H "Authorization: Bearer $TOKEN" -H "Idempotency-Key: despacho-001"  # repetición: misma respuesta
```
