#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_CONFIRM_STOP_PARCEL_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_CONFIRM_STOP_PARCEL_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace hubflow::routing::application
{

/// @brief Bulto que el repartidor confirma en una parada: ruta, número de parada, paquete y número de bulto.
struct ConfirmParcelRequest
{
    std::uint64_t routeId {0};
    std::size_t   order {0};
    std::string   shipmentCode;
    std::size_t   parcelOrdinal {0};
};

/**
 * @brief El repartidor confirma que un bulto de una parada llegó a destino, sin cerrar todavía la parada.
 *
 * Sirve para registrar bulto por bulto. Al cerrar la parada, si faltan bultos el paquete vuelve a la planta con los
 * restantes y los confirmados quedan registrados.
 *
 * @par Quién puede ejecutarlo
 * ADMIN, OPERADOR y PUM; un PUM sólo puede confirmar bultos de sus propias rutas.
 *
 * @par Errores
 * - Forbidden: el rol no puede resolver paradas o la ruta pertenece a otro PUM.
 * - NotFound: la ruta, la parada, el paquete dentro de la parada o el bulto no existen.
 * - Conflict: la parada ya fue cerrada.
 * - InvalidState: el paquete no está en reparto.
 *
 * @par Idempotencia
 * Sí. Confirmar un bulto ya confirmado no cambia nada, no emite eventos y responde igual que la primera vez.
 */
class ConfirmStopParcelUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~ConfirmStopParcelUseCase() = default;

    virtual const logistics::domain::Shipment& execute(const shared::application::Principal& principal,
                                                       const ConfirmParcelRequest&           request) = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_CONFIRM_STOP_PARCEL_USE_CASE_HPP
