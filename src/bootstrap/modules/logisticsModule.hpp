#ifndef HUBFLOW_BOOTSTRAP_MODULES_LOGISTICS_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_MODULES_LOGISTICS_MODULE_HPP

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

namespace hubflow::identity::application
{
class UserRepository;
}

namespace hubflow::identity::domain
{
class AccessPolicy;
}

namespace hubflow::bootstrap
{

/// @brief Lo que logística necesita de otros contextos: el mapa de ruteo y quién es quién en el sistema.
struct LogisticsNeighbours
{
    const routing::application::RoutingModel&    routing;
    const identity::application::UserRepository& users;
    const identity::domain::AccessPolicy&        policy;
};

/**
 * @brief Arma el contexto de logística: envíos, bultos, grupos, despacho y catálogos de zonas, servicios y
 * características.
 *
 * Mantiene al día la lista de pendientes (etapa de proyección), guarda envíos, grupos y catálogos (etapa de
 * persistencia), reconstruye el modelo en memoria al iniciar y registra el dataset de ejemplo si no hay envíos.
 */
class LogisticsModule final : public BoundedContextModule
{
public:
    LogisticsModule(const SharedServices&                   shared,
                    logistics::application::LogisticsModel& model,
                    const LogisticsNeighbours&              neighbours);

    ~LogisticsModule() override;

    LogisticsModule(const LogisticsModule&)            = delete;
    LogisticsModule& operator=(const LogisticsModule&) = delete;

private:
    void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus) override;

    void hydrate() override;

    void seed() override;

    void registerRoutes(platform::ApiRouter& router) override;

    struct Parts;

    std::unique_ptr<Parts> m_spParts;
};

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULES_LOGISTICS_MODULE_HPP
