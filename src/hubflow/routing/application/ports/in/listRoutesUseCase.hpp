#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_LIST_ROUTES_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_LIST_ROUTES_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::routing::application
{

    /**
     * @brief Lista las hojas de ruta visibles para el usuario: todas para ADMIN y OPERADOR, sólo las propias para un
     * PUM.
     *
     * @par Quién puede ejecutarlo
     * ADMIN, OPERADOR y PUM.
     *
     * @par Errores
     * - Forbidden: el rol no puede resolver paradas.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class ListRoutesUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ListRoutesUseCase() = default;

        virtual ds::DynamicArray<const domain::DeliveryRoute*>
        execute(const shared::application::Principal& principal) const = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_LIST_ROUTES_USE_CASE_HPP
