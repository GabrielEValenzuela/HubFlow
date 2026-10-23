#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_ADD_ROAD_NODE_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_ADD_ROAD_NODE_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::routing::application
{

/// @brief Datos de un punto nuevo del mapa: dónde está, a qué zona pertenece y con qué calles se une a la red.
struct AddRoadNodeRequest
{
    std::string                        id;
    std::string                        label;
    std::string                        zone;
    double                             x {0.0};
    double                             y {0.0};
    ds::DynamicArray<domain::RoadLink> links;
};

/**
 * @brief Agrega un punto de entrega al mapa de la ciudad, asignado a una zona habilitada y unido por calles a puntos
 * que ya existen. Es lo que permite empezar a repartir en una zona nueva sin tocar el programa.
 *
 * @par Quién puede ejecutarlo
 * Sólo ADMIN.
 *
 * @par Errores
 * - Forbidden: el rol no puede administrar la configuración.
 * - Conflict: ya existe un punto con ese código.
 * - Validation: datos inválidos, zona inexistente o desactivada, sin calles, calle repetida o más corta que la línea
 *   recta.
 * - NotFound: alguna calle apunta a un punto que no existe.
 *
 * @par Idempotencia
 * No. Crea un punto nuevo. La clave natural es el código: repetir la petición falla con Conflict sin duplicar. Con el
 * encabezado Idempotency-Key el borde HTTP devuelve la respuesta original.
 */
class AddRoadNodeUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~AddRoadNodeUseCase() = default;

    virtual const domain::RoadNode& execute(const shared::application::Principal& principal,
                                            const AddRoadNodeRequest&             request) = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_ADD_ROAD_NODE_USE_CASE_HPP
