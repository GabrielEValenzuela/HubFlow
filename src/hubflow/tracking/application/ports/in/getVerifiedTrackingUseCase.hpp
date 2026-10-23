#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_IN_GET_VERIFIED_TRACKING_USE_CASE_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_IN_GET_VERIFIED_TRACKING_USE_CASE_HPP

#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/tracking/application/trackingViews.hpp"

#include <string>

namespace hubflow::tracking::application
{

/**
 * @brief Muestra el detalle ampliado del paquete a quien tiene una constancia de verificación vigente de ese código:
 * primer nombre del destinatario, zona y barrio, cuántos bultos son y cuántos se entregaron, y la franja estimada de
 * entrega si el paquete va en camino.
 *
 * La franja se calcula con el avance de la hoja de ruta y la misma estimación de tiempos que usa la planta.
 *
 * @par Quién puede ejecutarlo
 * Quien tenga una constancia emitida por la verificación para ese mismo código.
 *
 * @par Errores
 * - Unauthorized: falta la constancia, es inválida, venció o es de otro paquete. Es el mismo error aunque el código
 *   no exista.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class GetVerifiedTrackingUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~GetVerifiedTrackingUseCase() = default;

    virtual VerifiedTracking execute(const std::string& code, const std::string& token) const = 0;
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_IN_GET_VERIFIED_TRACKING_USE_CASE_HPP
