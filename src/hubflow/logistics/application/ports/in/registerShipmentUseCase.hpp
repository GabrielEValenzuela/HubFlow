#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_REGISTER_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_REGISTER_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Recibe un paquete en el centro de distribución: valida sus datos, lo incorpora al catálogo y lo deja pendiente
 * de despacho.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 * - Conflict: ya existe un envío con ese código de seguimiento.
 * - Validation: datos inválidos, o punto de entrega inexistente o de otra zona.
 *
 * @par Idempotencia
 * No. Crea un envío nuevo. La clave natural es el código de seguimiento: repetir la petición falla con Conflict sin
 * duplicar. Con el encabezado Idempotency-Key el borde HTTP devuelve la respuesta original.
 */
class RegisterShipmentUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~RegisterShipmentUseCase() = default;

    virtual const domain::Shipment& execute(const shared::application::Principal& principal,
                                            const domain::ShipmentDraft&          draft) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_REGISTER_SHIPMENT_USE_CASE_HPP
