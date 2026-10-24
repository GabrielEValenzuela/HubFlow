#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_ROUTE_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_ROUTE_USE_CASE_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstdint>

namespace hubflow::routing::application
{

    /**
     * @brief Devuelve una hoja de ruta por su identificador, con sus paradas y tramos.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR para cualquier ruta; un PUM sólo las propias.
     *
     * @par Errores
     * - Forbidden: el rol no puede resolver paradas o la ruta pertenece a otro PUM.
     * - NotFound: la ruta no existe.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class GetRouteUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~GetRouteUseCase() = default;

        virtual const domain::DeliveryRoute& execute(const shared::application::Principal& principal,
                                                     std::uint64_t routeId) const = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_ROUTE_USE_CASE_HPP
