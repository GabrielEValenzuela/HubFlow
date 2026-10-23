#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DISPATCH_NEXT_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DISPATCH_NEXT_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

/**
 * @brief Despacha el envío que encabeza la lista de pendientes (el de mayor prioridad y, a igual prioridad, el que
 * llegó primero). Si viaja junto con otros paquetes, salen todos los de su grupo.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 * - Conflict: no hay envíos pendientes.
 *
 * @par Idempotencia
 * No. Cada ejecución despacha un envío distinto (el siguiente de la lista). Con Idempotency-Key el borde HTTP evita
 * despachar dos veces por un reintento.
 */
class DispatchNextShipmentUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~DispatchNextShipmentUseCase() = default;

    virtual const domain::Shipment& execute(const shared::application::Principal& principal,
                                            const std::string&                    note) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DISPATCH_NEXT_SHIPMENT_USE_CASE_HPP
