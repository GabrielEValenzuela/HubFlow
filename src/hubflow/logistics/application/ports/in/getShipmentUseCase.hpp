#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

    /**
     * @brief Devuelve un envío por código de seguimiento.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede operar envíos.
     * - NotFound: el envío no existe.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class GetShipmentUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~GetShipmentUseCase() = default;

        virtual const domain::Shipment& execute(const shared::application::Principal& principal,
                                                const std::string& code) const = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_SHIPMENT_USE_CASE_HPP
