# HubFlow

API REST en C++17 para gestionar envíos de última milla, pensada como proyecto de referencia de Algoritmos
y Estructuras de Datos. Las estructuras de datos (listas, pila, cola, BST, árbol B+, BitSet, Set, heap,
grafo) son propias y usan punteros crudos; el dominio sigue DDD y Clean Code. Incluye login con JWT y roles,
una mesa de clasificación y un agente PUM (Proveedor de Última Milla) que planifica rutas con BFS, DFS,
Greedy y A* sobre un mapa de la ciudad y exporta `initial_state.json` y `sequence.json`.

## Quick start

Requisitos: Docker con Compose v2, `make`, `curl` y `openssl`.

```bash
make up
```

La primera vez compila la imagen (varios minutos, incluye correr los tests), crea `.env` con un secreto JWT
aleatorio, levanta MySQL y la API, espera `/api/v1/health` y abre el dashboard.

- Dashboard y API: <http://localhost:8480> (puerto configurable con `HUBFLOW_HTTP_PORT` en `.env`)
- MySQL: `127.0.0.1:3317` (solo local, `HUBFLOW_DB_PORT`)

### Credenciales semilla

Clave de todos: el valor de `HUBFLOW_SEED_PASSWORD` (`hubflow123` por defecto).

| Usuario | Rol |
|---|---|
| `admin@hubflow.local` | ADMIN |
| `operador@hubflow.local` | OPERADOR |
| `pum1@hubflow.local`, `pum2@hubflow.local` | PUM |

Al primer arranque también se cargan 8 paquetes iniciales (puntos de entrega C1, N2, S3, C2, N3, S5, C4, N5).

```bash
TOKEN=$(curl -s -X POST localhost:8480/api/v1/sessions -H 'Content-Type: application/json' \
  -d '{"email":"operador@hubflow.local","password":"hubflow123"}' | python3 -c 'import sys,json;print(json.load(sys.stdin)["token"])')
curl -s localhost:8480/api/v1/pending-shipments -H "Authorization: Bearer $TOKEN"
```

## Arquitectura en un párrafo

Hexagonal por contexto: `logistics`, `sorting`, `routing`, `identity`, `insights` y `audit` tienen cada uno
su `domain` (entidades, value objects, servicios y eventos), su `application` (puertos de entrada y salida,
casos de uso, suscriptores) y sus `adapters` (HTTP, MySQL, seguridad, exportación). Los casos de uso publican
eventos de dominio en un bus síncrono; las proyecciones, la persistencia y la auditoría son suscriptores.
`bootstrap/` es el composition root. Más detalle en [docs/arquitectura.md](docs/arquitectura.md).

## Rutas: opciones de negocio

Al asignar una ruta el operador elige **Menos kilómetros** (A*, recomendada), **Menos cruces** (BFS) o
**Salida inmediata** (Greedy), con una comparación de km, tiempo estimado y cruces para la carga actual. Los
algoritmos y sus trazas (incluido DFS) se explican en la pantalla "Cómo decide el PUM"; el detalle está en
[docs/grafos-pum.md](docs/grafos-pum.md).

## Comandos

| Comando | Qué hace |
|---|---|
| `make help` | lista los targets |
| `make up` / `make down` | levanta (construye, espera la salud, abre el navegador) / detiene el stack |
| `make logs` / `make ps` | logs y estado de los contenedores |
| `make reset-db` | borra el volumen `hubflow-db` (pierde los datos) |
| `make build` / `make test` | compila y corre los tests en el host (preset `debug`) |
| `make asan-test` | tests con AddressSanitizer y UBSan |
| `make itest` | tests de integración contra MySQL (`hubflow_test`) dentro del contenedor |
| `make format` / `make format-check` | `clang-format` sobre `src` y `tests` |
| `make docs` | documentación Doxygen en `build/docs/html` |
| `make clean` | borra `build/` |

Build en el host sin Docker: `cmake --preset debug && cmake --build --preset debug && ctest --preset debug`
(solo núcleo y tests; el servidor necesita `-DHUBFLOW_BUILD_SERVER=ON` con libmysqlclient, libsodium y OpenSSL).

## Estructura

```text
src/datastructures/   estructuras de datos propias (hubflow::ds)
src/hubflow/          contextos (domain / application / adapters) y platform
src/bootstrap/        composition root y datos iniciales
tests/                unit, application, adapters, http, integration
db/migrations/        esquema, mapa de la ciudad e idempotencia (se cargan al crear la base)
docker/               Dockerfile de la API
web/                  dashboard estático servido por la API
exports/              archivos initial_state.json / sequence.json por ruta
docs/                 documentación (abajo)
```

## Documentación

- [Arquitectura](docs/arquitectura.md): capas, diagrama, flujo de un caso de uso y orden de eventos
- [Ownership](docs/ownership.md): quién crea y destruye cada objeto
- [Complejidades](docs/big-o.md): Big-O de estructuras y búsquedas
- [Grafos y PUM](docs/grafos-pum.md): formulación del problema, algoritmos y formatos de exportación
- [Idempotencia](docs/idempotencia.md): endpoints e `Idempotency-Key`
- [Trazabilidad](docs/trazabilidad.md): enunciado del TP a código y tests
