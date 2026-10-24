#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_HEAVIEST_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_HEAVIEST_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/services/zoneReport.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::logistics::application
{

    /**
     * @brief Encuentra el envío pendiente más pesado de una zona, o ninguno (nullptr) si la zona no tiene pendientes.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede operar envíos.
     * - NotFound: la zona no existe.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class FindHeaviestShipmentUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~FindHeaviestShipmentUseCase() = default;

        virtual const domain::Shipment* execute(const shared::application::Principal& principal,
                                                const std::string& zoneCode) const = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_HEAVIEST_SHIPMENT_USE_CASE_HPP
