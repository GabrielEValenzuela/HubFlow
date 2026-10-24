#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_RESCHEDULE_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_RESCHEDULE_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

    /**
     * @brief Reprograma un envío que estaba en reparto: suma un intento fallido y lo devuelve a pendientes respetando
     * su prioridad. Los bultos que ya se habían entregado quedan registrados como entregados.
     *
     * Si el paquete viaja junto con otros que también están en reparto, se reprograman todos.
     *
     * @par Quién puede ejecutarlo
     * ADMIN, OPERADOR y PUM.
     *
     * @par Errores
     * - Forbidden: el rol no puede resolver paradas.
     * - NotFound: el envío no existe.
     * - InvalidState: el envío no está en reparto.
     *
     * @par Idempotencia
     * No. Cada ejecución suma un intento y agrega un movimiento al historial. Con Idempotency-Key el borde HTTP evita
     * el duplicado.
     */
    class RescheduleShipmentUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

        virtual ~RescheduleShipmentUseCase() = default;

        virtual const domain::Shipment& execute(const shared::application::Principal& principal,
                                                const std::string& code,
                                                const std::string& reason) = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_RESCHEDULE_SHIPMENT_USE_CASE_HPP
