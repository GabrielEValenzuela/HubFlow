#include "bootstrap/application.hpp"

#include "bootstrap/infrastructureModule.hpp"
#include "bootstrap/moduleSet.hpp"
#include "bootstrap/modules/auditModule.hpp"
#include "bootstrap/modules/identityModule.hpp"
#include "bootstrap/modules/insightsModule.hpp"
#include "bootstrap/modules/logisticsModule.hpp"
#include "bootstrap/modules/routingModule.hpp"
#include "bootstrap/modules/sortingModule.hpp"
#include "bootstrap/modules/trackingModule.hpp"
#include "bootstrap/plantModels.hpp"

#include <utility>

namespace hubflow::bootstrap
{

    struct Application::Parts
    {
        explicit Parts(platform::Settings settings)
            : infrastructure {std::move(settings)}
        {
            const SharedServices shared = infrastructure.services();
            auto& identity = modules.add(std::make_unique<IdentityModule>(shared, models.routing));
            modules.add(std::make_unique<AuditModule>(shared));
            modules.add(std::make_unique<LogisticsModule>(
                shared, models.logistics, LogisticsNeighbours {models.routing, identity.users(), identity.policy()}));
            modules.add(std::make_unique<SortingModule>(models.desk, models.logistics));
            modules.add(std::make_unique<RoutingModule>(
                shared, models.routing, RoutingNeighbours {models.logistics, models.desk, identity.users()}));
            modules.add(std::make_unique<InsightsModule>(models.logistics));
            modules.add(std::make_unique<TrackingModule>(shared, models.logistics, models.routing));

            modules.subscribeAll(shared.bus);
            modules.hydrateAll();
            modules.seedAll();
            modules.registerAllRoutes(infrastructure.openRouter(identity.authenticator()));
        }

        InfrastructureModule infrastructure;
        PlantModels models;
        ModuleSet modules;
    };

    Application::Application(platform::Settings settings)
        : m_spParts {std::make_unique<Parts>(std::move(settings))}
    {
    }

    Application::~Application() = default;

    int Application::run()
    {
        return m_spParts->infrastructure.serve();
    }

} // namespace hubflow::bootstrap
