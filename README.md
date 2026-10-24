# HubFlow

**Última milla en C++17, desde el puntero crudo hasta la puerta del cliente.**

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)
![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C?logo=cmake)
![Tests](https://img.shields.io/badge/tests-230%20GoogleTest-brightgreen)
![Sanitizers](https://img.shields.io/badge/sanitizers-ASan%20%2B%20UBSan-success)
![Docker](https://img.shields.io/badge/docker-make%20up-2496ED?logo=docker)

> [!NOTE]
> Ni `std::list`, ni `std::map`, ni Boost Graph. Cada lista, árbol, heap y grafo que mueve un paquete en
> HubFlow está escrito a mano en este repositorio, con punteros crudos, Doxygen y tests.

HubFlow es una API REST completa para gestionar envíos de última milla: un paquete entra a la planta, se
clasifica, se carga en un vehículo, un chofer lo entrega y el cliente lo sigue desde el celular. Nació como
proyecto de referencia de **Algoritmos y Estructuras de Datos**, así que todo lo que suele venir de una
biblioteca acá se construye a mano y se puede ver funcionando.

Lo que trae:

* **Estructuras de datos propias** (`hubflow::ds`): listas simple y doblemente enlazadas, pila, cola, arreglo
  dinámico, BST, **árbol B+**, BitSet, Set, min-heap y grafo con listas de adyacencia.
* Un **agente de ruteo** que planifica el reparto con **BFS, DFS, Greedy y A\*** sobre el mapa de la ciudad y
  exporta cada decisión en `initial_state.json` y `sequence.json`.
* **Arquitectura hexagonal por contexto** con DDD de verdad: entidades, value objects, eventos de dominio en un
  bus y casos de uso con idempotencia explícita.
* Un **producto usable** en lenguaje de logística para chofer, jefe de planta y administrador, más una
  **página pública de seguimiento** para el cliente.
* Un **laboratorio** (`/lab/`) que abre la caja: simulador del agente, reproductor paso a paso de las búsquedas
  e índices BST y B+ en vivo.
* **JWT y roles**, contraseñas con libsodium, MySQL, Docker, sanitizers y análisis estático en un solo `make`.

---

# Contenido

* [Quick start](#quick-start)
  * [En acción](#en-acción)
* [Por qué HubFlow](#por-qué-hubflow)
* [Arquitectura](#arquitectura)
* [El producto](#el-producto)
  * [Paquetes y bultos](#paquetes-y-bultos)
  * [Rutas](#rutas)
  * [Seguimiento para el cliente](#seguimiento-para-el-cliente)
  * [Configuración como datos](#configuración-como-datos)
* [El laboratorio](#el-laboratorio)
* [Calidad](#calidad)
* [Comandos](#comandos)
* [Estructura del repositorio](#estructura-del-repositorio)
* [Documentación](#documentación)

# Quick start

Requisitos: Docker con Compose v2, `make`, `curl` y `openssl`.

```bash
make up
```

Un solo comando compila la imagen (corriendo los tests en el camino), genera `.env` con un secreto JWT
aleatorio, levanta MySQL y la API, espera a `/api/v1/health` y abre el navegador.

| Qué | Dónde |
|---|---|
| Producto y API | <http://localhost:8480> (`HUBFLOW_HTTP_PORT`) |
| Seguimiento público | <http://localhost:8480/seguimiento/?codigo=PKG-1002> |
| Laboratorio | <http://localhost:8480/lab/> |
| MySQL | `127.0.0.1:3317`, solo local (`HUBFLOW_DB_PORT`) |

Usuarios semilla, todos con la clave `HUBFLOW_SEED_PASSWORD` (`hubflow123` por defecto):

| Usuario | Puesto (rol) |
|---|---|
| `admin@hubflow.local` | Administrador (`ADMIN`) |
| `operador@hubflow.local` | Jefe de planta (`OPERADOR`) |
| `pum1@hubflow.local`, `pum2@hubflow.local` | Chofer (`PUM`) |

En el primer arranque se cargan además 8 paquetes de ejemplo, `PKG-1001` a `PKG-1008`.

## En acción

```bash
TOKEN=$(curl -s -X POST localhost:8480/api/v1/sessions -H 'Content-Type: application/json' \
  -d '{"email":"operador@hubflow.local","password":"hubflow123"}' \
  | python3 -c 'import sys,json;print(json.load(sys.stdin)["token"])')

curl -s localhost:8480/api/v1/pending-shipments -H "Authorization: Bearer $TOKEN"
```

Y por dentro, el grafo que recorre el agente de ruteo es una estructura propia con una API mínima:

```cpp
#include "datastructures/graph.hpp"

hubflow::ds::Graph<RoadNode> city;
const std::size_t plant = city.addVertex(RoadNode {"HUB", "Planta", true});
const std::size_t corner = city.addVertex(RoadNode {"C1", "Centro 1"});
city.addEdge(plant, corner, 1.4);

city.forEachNeighbor(plant, [](std::size_t destination, double kilometers) {
    // BFS, DFS, Greedy y A* se apoyan solo en esto
});
```

# Por qué HubFlow

Los ejemplos de estructuras de datos suelen terminar en un `main` que inserta diez números. HubFlow responde la
pregunta siguiente: **¿dónde vive esa estructura en un sistema real?**

* La lista de pendientes es una **lista enlazada con prioridad estable**, porque el orden de llegada importa.
* La mesa de clasificación es una **cola FIFO** y la carga del vehículo una **pila LIFO**: lo último que sube es
  lo primero que se entrega.
* Los paquetes se buscan por código en un **BST** y por tramo de códigos en un **árbol B+**.
* El historial de movimientos de un paquete es una **lista doblemente enlazada** y los estados posibles un
  **BitSet**.
* El mapa de la ciudad es un **grafo**, y la frontera de la búsqueda es una **cola** en BFS, una **pila** en DFS y
  un **min-heap** en Greedy y A\*.

Cada decisión de ownership, cada complejidad y cada algoritmo está documentado, y cada uno se puede ver
ejecutándose en el laboratorio.

# Arquitectura

Hexagonal, cortada por contexto: `logistics`, `sorting`, `routing`, `tracking`, `identity`, `insights` y `audit`.
Cada uno tiene su `domain` (entidades, value objects, servicios y eventos), su `application` (un puerto de entrada
por caso de uso, puertos de salida, comandos y consultas) y sus `adapters` (HTTP, MySQL, seguridad, exportación).

Los casos de uso publican eventos de dominio en un bus síncrono; persistencia, proyecciones y auditoría son
suscriptores. `bootstrap/` es la raíz de composición, con un módulo por contexto. Los `POST` no idempotentes
aceptan `Idempotency-Key`.

Más detalle en [docs/arquitectura.md](docs/arquitectura.md) e [docs/idempotencia.md](docs/idempotencia.md).

# El producto

La web (`web/`) habla el idioma de la planta, sin vocabulario técnico. Cada puesto ve solo lo suyo:

| Puesto | Pantallas |
|---|---|
| **Chofer** | **Mi ruta**: próxima parada con sus bultos (confirmar uno por uno, "Entregar todo" o "Terminar con lo entregado"), "No se pudo entregar" con motivo, lista de paradas y mapa |
| **Jefe de planta** | **Paquetes en planta** (ingresar, imprimir etiquetas, clasificar, despachar, hacer que viajen juntos), **Armar carga** (zona, orden, "Por cargar" / "En el vehículo", asignar chofer), **Repartos en curso** (avance por chofer, "Pasar a otro chofer"), **Buscar paquete** (por código, por tramo, por zona y servicio) |
| **Administrador** | **Resumen de la operación**, **Repartos**, **Personal**, **Configuración** (zonas, servicios y cuidados), **Actividad** y **Avisos a clientes** |

## Paquetes y bultos

El jefe de planta **no escribe el código**: el sistema asigna una identidad interna (UUID v7) y un código de
seguimiento que se imprime en una etiqueta por bulto. Por defecto es `HF-` + 9 caracteres no adivinables con
dígito verificador; con `HUBFLOW_TRACKING_CODES=sequential` los nuevos son correlativos `PKG-NNNN`.

Un paquete tiene de 1 a 20 bultos con peso y medidas, y se factura lo que más pese entre el peso real y el
volumétrico (largo × ancho × alto / 5000). Si el chofer entrega solo algunos bultos, el paquete vuelve a la planta
con los restantes. Varios paquetes al mismo destino pueden **viajar juntos**: se cargan juntos, son una sola
parada y se reprograman juntos.

## Rutas

Al asignar una ruta, el jefe de planta elige entre **Menos kilómetros** (A\*, recomendada), **Menos cruces** (BFS)
o **Salida inmediata** (Greedy), con una comparación de km, tiempo estimado y cruces para la carga actual. La
formulación del problema y los formatos de exportación están en [docs/grafos-pum.md](docs/grafos-pum.md).

## Seguimiento para el cliente

<http://localhost:8480/seguimiento/> es pública, no pide login y está pensada para el celular. Dice dónde está el
paquete en lenguaje llano. Con los últimos 4 dígitos del teléfono o el código postal del destinatario muestra
además primer nombre, zona y barrio, bultos y la franja estimada de entrega, con intentos limitados (5 cada 15
minutos por código y origen).

El enlace va en la etiqueta y en los avisos al cliente (por ahora se registran, no se envían); su dirección base
se configura con `HUBFLOW_PUBLIC_BASE_URL`. Para probar: `PKG-1002` con teléfono terminado en `4410` o código
postal `1640`.

## Configuración como datos

Zonas, niveles de servicio con su prioridad de despacho, características de los paquetes y permisos por rol
viven en la base y se administran desde **Configuración** o la API (`/zones`, `/service-levels`,
`/shipment-features`, `/road-nodes`), sin recompilar. Nada se borra, solo se desactiva, y no se puede desactivar
algo en uso.

> [!WARNING]
> Las migraciones de `db/migrations/` solo corren sobre un volumen nuevo. Si ya tenías datos de una versión
> anterior, ejecutá `make reset-db` y luego `make up` (el dataset de ejemplo se vuelve a cargar solo).

# El laboratorio

<http://localhost:8480/lab/> es una página aparte, sin enlaces desde el producto y con su propio ingreso (jefe de
planta o administrador). Ahí está todo lo que el producto esconde:

* La lista de pendientes con prioridad estable y recursión por zona.
* La mesa con **Queue FIFO**, **Stack LIFO**, **Insertion Sort** y búsqueda lineal frente a binaria.
* Índices **BST** y **B+**, **Set** y **BitSet**.
* **Cómo decide el agente de ruteo**: formulación, simulador, reproductor paso a paso de BFS, DFS, Greedy y A\* y
  descarga de `initial_state.json` / `sequence.json`.

# Calidad

| | |
|---|---|
| Tests | 230 casos con GoogleTest: dominio, aplicación, adaptadores, HTTP e integración contra MySQL |
| Memoria | `make asan-test` corre todo con AddressSanitizer y UBSan |
| Formato | `clang-format` con `.clang-format` del repo |
| Análisis estático | `clang-tidy` (warnings como errores) y `cppcheck`, juntos en `make lint` |
| Documentación | Doxygen en cada header público (`make docs`) |

# Comandos

| Comando | Qué hace |
|---|---|
| `make help` | lista los targets |
| `make up` / `make down` | levanta (construye, espera la salud, abre el navegador) / detiene el stack |
| `make logs` / `make ps` | logs y estado de los contenedores |
| `make reset-db` | borra el volumen `hubflow-db` (pierde los datos) |
| `make build` / `make test` | compila y corre los tests en el host (preset `debug`) |
| `make asan-test` | tests con AddressSanitizer y UBSan |
| `make itest` | tests de integración contra MySQL (`hubflow_test`) dentro del contenedor |
| `make format` / `make format-check` | aplica / verifica `clang-format` sobre `src` y `tests` |
| `make tidy` / `make tidy-fix` | `clang-tidy` sobre el proyecto / con correcciones automáticas |
| `make cppcheck` | análisis con `cppcheck` |
| `make lint` | formato, `clang-tidy` y `cppcheck` con resumen final |
| `make docs` | documentación Doxygen en `build/docs/html` |
| `make clean` | borra `build/` |

Sin Docker: `cmake --preset debug && cmake --build --preset debug && ctest --preset debug` compila el núcleo y
los tests. El servidor necesita `-DHUBFLOW_BUILD_SERVER=ON` con libmysqlclient, libsodium y OpenSSL.

# Estructura del repositorio

```text
src/datastructures/   estructuras de datos propias (hubflow::ds)
src/hubflow/          contextos (domain / application / adapters) y platform
src/bootstrap/        raíz de composición y datos iniciales
tests/                unit, application, adapters, http, integration
db/migrations/        esquema, mapa de la ciudad, idempotencia y catálogos
docker/               Dockerfile de la API
scripts/              formato y análisis estático
web/                  producto estático; web/lab/ es el laboratorio y web/seguimiento/ la página pública
exports/              initial_state.json / sequence.json por ruta
docs/                 documentación
```

# Documentación

* [Arquitectura](docs/arquitectura.md): capas, diagrama, flujo de un caso de uso y orden de eventos
* [Ownership](docs/ownership.md): quién crea y destruye cada objeto
* [Complejidades](docs/big-o.md): Big-O de estructuras y búsquedas
* [Grafos y agente de ruteo](docs/grafos-pum.md): formulación del problema, algoritmos y formatos de exportación
* [Idempotencia](docs/idempotencia.md): endpoints e `Idempotency-Key`
* [Trazabilidad](docs/trazabilidad.md): del enunciado del TP al código y los tests
