#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_IN_GET_PUBLIC_TRACKING_USE_CASE_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_IN_GET_PUBLIC_TRACKING_USE_CASE_HPP

#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/tracking/application/trackingViews.hpp"

#include <string>

namespace hubflow::tracking::application
{

    /**
     * @brief Muestra a cualquier persona con el código de seguimiento dónde está su paquete: la situación actual y el
     * recorrido, en lenguaje de cliente.
     *
     * El código puede escribirse en minúsculas. No hace falta iniciar sesión. No muestra notas internas, nombres del
     * personal ni datos del destinatario.
     *
     * @par Quién puede ejecutarlo
     * Cualquier persona, sin sesión.
     *
     * @par Errores
     * - NotFound: no hay un paquete con ese código (o el código está mal escrito). Es el mismo error que produce una
     *   verificación fallida.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class GetPublicTrackingUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~GetPublicTrackingUseCase() = default;

        virtual PublicTracking execute(const std::string& code) const = 0;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_IN_GET_PUBLIC_TRACKING_USE_CASE_HPP
