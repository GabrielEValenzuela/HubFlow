#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CLASSIFY_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CLASSIFY_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

/**
 * @brief Marca un envío recibido (o reprogramado) como clasificado, listo para salir a reparto.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 * - NotFound: el envío no existe.
 * - InvalidState: el envío no está en un estado desde el que pueda clasificarse.
 *
 * @par Idempotencia
 * No. Cada ejecución agrega un movimiento al historial del envío, así que repetirla produce un segundo movimiento (o
 * falla por estado). Con Idempotency-Key el borde HTTP evita el duplicado.
 */
class ClassifyShipmentUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~ClassifyShipmentUseCase() = default;

    virtual const domain::Shipment&
    execute(const shared::application::Principal& principal, const std::string& code, const std::string& note) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CLASSIFY_SHIPMENT_USE_CASE_HPP
