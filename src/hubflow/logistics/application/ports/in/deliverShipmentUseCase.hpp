#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DELIVER_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DELIVER_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

/**
 * @brief Confirma la entrega de un envío que estaba en reparto; es un estado final.
 *
 * Un envío sólo se da por entregado cuando todos sus bultos fueron confirmados. Con @c confirmAllParcels el operador
 * declara que los bultos que faltan también llegaron y se confirman en el mismo paso; sin él, si falta alguno se
 * rechaza. Para una entrega parcial se confirman los bultos entregados y se reprograma el envío.
 *
 * @par Quién puede ejecutarlo
 * ADMIN, OPERADOR y PUM.
 *
 * @par Errores
 * - Forbidden: el rol no puede resolver paradas.
 * - NotFound: el envío no existe.
 * - InvalidState: el envío no está en reparto.
 * - Conflict: faltan bultos por confirmar y no se pidió confirmarlos todos.
 *
 * @par Idempotencia
 * No. Agrega un movimiento al historial. Repetirla falla por estado (ya entregado), pero no reproduce la respuesta
 * original salvo que se use Idempotency-Key en el borde HTTP.
 */
class DeliverShipmentUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~DeliverShipmentUseCase() = default;

    virtual const domain::Shipment& execute(const shared::application::Principal& principal,
                                            const std::string&                    code,
                                            const std::string&                    note,
                                            bool                                  confirmAllParcels) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DELIVER_SHIPMENT_USE_CASE_HPP
