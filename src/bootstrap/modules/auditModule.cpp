#include "bootstrap/modules/auditModule.hpp"

#include "hubflow/audit/adapters/in/http/auditController.hpp"
#include "hubflow/audit/adapters/out/mysql/mySqlAuditLog.hpp"
#include "hubflow/audit/application/subscribers/auditLogSubscriber.hpp"
#include "hubflow/audit/application/usecases/queries/countAuditEvents.hpp"
#include "hubflow/audit/application/usecases/queries/listAuditEvents.hpp"
#include "hubflow/platform/http/apiRouter.hpp"

namespace hubflow::bootstrap
{

    struct AuditModule::Parts
    {
        explicit Parts(const SharedServices& shared)
            : audit {shared.connection}
            , auditLog {audit}
            , listAuditEvents {audit}
            , countAuditEvents {audit}
        {
            subscribers.add(SubscriptionPhase::Audit, auditLog);
        }

        audit::adapters::MySqlAuditLog audit;
        audit::application::AuditLogSubscriber auditLog;
        audit::application::ListAuditEvents listAuditEvents;
        audit::application::CountAuditEvents countAuditEvents;
        PhasedSubscribers subscribers;
        std::unique_ptr<platform::Controller> spController;
    };

    AuditModule::AuditModule(const SharedServices& shared)
        : m_spParts {std::make_unique<Parts>(shared)}
    {
    }

    AuditModule::~AuditModule() = default;

    void AuditModule::subscribe(const SubscriptionPhase phase, shared::application::EventBus& bus)
    {
        m_spParts->subscribers.subscribe(phase, bus);
    }

    void AuditModule::registerRoutes(platform::ApiRouter& router)
    {
        Parts& parts = *m_spParts;
        parts.spController =
            std::make_unique<audit::adapters::AuditController>(parts.listAuditEvents, parts.countAuditEvents);
        parts.spController->registerRoutes(router);
    }

} // namespace hubflow::bootstrap
