#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_IN_VERIFY_TRACKING_USE_CASE_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_IN_VERIFY_TRACKING_USE_CASE_HPP

#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/tracking/application/trackingViews.hpp"

namespace hubflow::tracking::application
{

    /**
     * @brief Confirma que quien consulta es quien espera el paquete, con los últimos cuatro dígitos del teléfono o el
     * código postal, y le entrega una constancia de corta duración para ver el detalle.
     *
     * La constancia sirve sólo para ese código de seguimiento y vence a los 15 minutos. Cada intento, acierte o no,
     * cuenta contra un límite de 5 en 15 minutos por dirección de origen y código; superado el límite, los intentos se
     * rechazan sin ni siquiera mirar el dato. Un código inexistente y un dato equivocado fallan con el mismo error para
     * que no se pueda averiguar qué códigos existen.
     *
     * @par Quién puede ejecutarlo
     * Cualquier persona, sin sesión.
     *
     * @par Errores
     * - NotFound: el código no existe o el dato no coincide (indistinguibles a propósito).
     * - Validation: no se informó ningún dato de verificación.
     * - RateLimited: se superó el límite de intentos; indica cuántos segundos esperar.
     *
     * @par Idempotencia
     * No. Cada intento gasta un lugar del límite y cada acierto emite una constancia nueva. Es inofensiva: no cambia
     * ningún dato del negocio y no se protege con Idempotency-Key porque no hay usuario; repetirla sólo consume
     * intentos.
     */
    class VerifyTrackingUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

        virtual ~VerifyTrackingUseCase() = default;

        virtual TrackingAccess execute(const VerificationRequest& request) = 0;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_IN_VERIFY_TRACKING_USE_CASE_HPP
