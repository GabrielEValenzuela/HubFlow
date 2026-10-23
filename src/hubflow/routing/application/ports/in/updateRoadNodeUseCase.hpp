#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_UPDATE_ROAD_NODE_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_UPDATE_ROAD_NODE_USE_CASE_HPP

#include "hubflow/routing/domain/entities/roadMap.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <optional>
#include <string>

namespace hubflow::routing::application
{

/// @brief Cambios parciales sobre un punto del mapa: sólo se aplican los campos presentes.
struct UpdateRoadNodeRequest
{
    std::optional<std::string> zone;
    std::optional<bool>        active;
};

/**
 * @brief Pasa un punto del mapa a otra zona o lo activa o desactiva. Es lo que permite liberar una zona antes de
 * desactivarla.
 *
 * @par Quién puede ejecutarlo
 * Sólo ADMIN.
 *
 * @par Errores
 * - Forbidden: el rol no puede administrar la configuración.
 * - NotFound: el punto no existe.
 * - Validation: la zona indicada no existe o está desactivada, o el punto quedaría activo en una zona desactivada.
 * - Conflict: el centro de distribución no se puede desactivar.
 *
 * @par Idempotencia
 * Sí. Describe el estado final deseado: aplicar el mismo cambio dos veces deja el punto igual y la segunda vez no
 * registra ningún evento.
 */
class UpdateRoadNodeUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~UpdateRoadNodeUseCase() = default;

    virtual const domain::RoadNode& execute(const shared::application::Principal& principal,
                                            const std::string&                    nodeId,
                                            const UpdateRoadNodeRequest&          request) = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_UPDATE_ROAD_NODE_USE_CASE_HPP
