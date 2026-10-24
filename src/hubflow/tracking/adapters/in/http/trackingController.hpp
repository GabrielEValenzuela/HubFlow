#ifndef HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_TRACKING_CONTROLLER_HPP
#define HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_TRACKING_CONTROLLER_HPP

#include "hubflow/platform/http/controller.hpp"
#include "hubflow/tracking/application/ports/in/getPublicTrackingUseCase.hpp"
#include "hubflow/tracking/application/ports/in/getVerifiedTrackingUseCase.hpp"
#include "hubflow/tracking/application/ports/in/verifyTrackingUseCase.hpp"

#include <string>

namespace hubflow::tracking::adapters
{

    /// @brief Casos de uso que consume el seguimiento público.
    struct TrackingUseCases
    {
        application::GetPublicTrackingUseCase& getPublicTracking;
        application::VerifyTrackingUseCase& verifyTracking;
        application::GetVerifiedTrackingUseCase& getVerifiedTracking;
    };

    /**
     * @brief Seguimiento para el cliente, sin sesión, bajo @c /public.
     *
     * - @c GET /public/tracking/:code situación y recorrido; con @c Authorization: Bearer de una verificación vigente
     * de ese código devuelve además el detalle ampliado.
     * - @c POST /public/tracking/:code/verifications confirma teléfono (últimos 4) o código postal y entrega la
     *   constancia.
     * - @c GET /public/tracking-settings dirección base de la página de seguimiento, para armar enlaces y etiquetas.
     *
     * La dirección de origen del pedido es la de la conexión: no se confía en cabeceras reenviadas porque cualquiera
     * puede falsearlas para esquivar el límite de intentos. Las respuestas no se guardan en cachés.
     */
    class TrackingController final : public platform::Controller
    {
    public:
        TrackingController(TrackingUseCases useCases, std::string trackingLinkBase);

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        platform::HttpResponse show(const platform::RequestContext& context) const;
        platform::HttpResponse verify(const platform::RequestContext& context) const;

        TrackingUseCases m_useCases;
        std::string m_trackingLinkBase;
    };

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_TRACKING_CONTROLLER_HPP
