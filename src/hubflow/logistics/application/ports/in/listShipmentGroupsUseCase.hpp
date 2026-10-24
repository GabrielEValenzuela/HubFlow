#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_SHIPMENT_GROUPS_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_SHIPMENT_GROUPS_USE_CASE_HPP

#include "hubflow/logistics/domain/services/shipmentGroupBook.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::logistics::application
{

    /**
     * @brief Devuelve todos los grupos de paquetes que viajan juntos.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede operar envíos.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class ListShipmentGroupsUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ListShipmentGroupsUseCase() = default;

        virtual const domain::ShipmentGroupBook& execute(const shared::application::Principal& principal) const = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_SHIPMENT_GROUPS_USE_CASE_HPP
