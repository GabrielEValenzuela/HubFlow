#ifndef HUBFLOW_BOOTSTRAP_MODULES_TRACKING_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_MODULES_TRACKING_MODULE_HPP

#include "bootstrap/boundedContextModule.hpp"
#include "bootstrap/sharedServices.hpp"

#include <memory>

namespace hubflow::logistics::application
{
struct LogisticsModel;
}

namespace hubflow::routing::application
{
struct RoutingModel;
}

namespace hubflow::bootstrap
{

/**
 * @brief Arma el contexto de seguimiento al cliente: consulta pública del estado, verificación de identidad del
 * destinatario y avisos por cada hito del envío.
 *
 * Avisa al cliente en la etapa de notificación, después de que el hecho quedó aplicado, guardado y exportado.
 * Los avisos se escriben en el registro de la planta y se guardan en la base de datos. Lee los envíos de
 * logística y el avance de las rutas.
 */
class TrackingModule final : public BoundedContextModule
{
public:
    TrackingModule(const SharedServices&                         shared,
                   const logistics::application::LogisticsModel& logistics,
                   const routing::application::RoutingModel&     routing);

    ~TrackingModule() override;

    TrackingModule(const TrackingModule&)            = delete;
    TrackingModule& operator=(const TrackingModule&) = delete;

private:
    void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus) override;

    void registerRoutes(platform::ApiRouter& router) override;

    struct Parts;

    std::unique_ptr<Parts> m_spParts;
};

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULES_TRACKING_MODULE_HPP
