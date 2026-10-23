#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_REASSIGN_DELIVERY_ROUTE_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_REASSIGN_DELIVERY_ROUTE_USE_CASE_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstdint>

namespace hubflow::routing::application
{

/**
 * @brief Pasa un reparto en curso a otro chofer activo, con sus paradas y su recorrido tal como están. Es el paso
 * previo para poder desactivar a un chofer que todavía tiene una ruta sin terminar.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR (jefe de planta).
 *
 * @par Errores
 * - Forbidden: el rol no puede asignar rutas.
 * - NotFound: la ruta no existe.
 * - Validation: el nuevo chofer no es un PUM activo.
 * - Conflict: el reparto ya terminó.
 *
 * @par Idempotencia
 * Sí. Describe el chofer final deseado: pasar la ruta al chofer que ya la tiene no cambia nada ni registra eventos.
 */
class ReassignDeliveryRouteUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~ReassignDeliveryRouteUseCase() = default;

    virtual const domain::DeliveryRoute&
    execute(const shared::application::Principal& principal, std::uint64_t routeId, std::uint64_t courierId) = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_REASSIGN_DELIVERY_ROUTE_USE_CASE_HPP
