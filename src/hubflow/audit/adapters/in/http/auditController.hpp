#ifndef HUBFLOW_AUDIT_ADAPTERS_IN_HTTP_AUDIT_CONTROLLER_HPP
#define HUBFLOW_AUDIT_ADAPTERS_IN_HTTP_AUDIT_CONTROLLER_HPP

#include "hubflow/audit/application/ports/in/countAuditEventsUseCase.hpp"
#include "hubflow/audit/application/ports/in/listAuditEventsUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::audit::adapters
{

    /// @brief Bitácora de auditoría: @c GET /audit-events (administrador), más recientes primero, paginada.
    class AuditController final : public platform::Controller
    {
    public:
        AuditController(const application::ListAuditEventsUseCase& list,
                        const application::CountAuditEventsUseCase& count);

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        const application::ListAuditEventsUseCase& m_list;
        const application::CountAuditEventsUseCase& m_count;
    };

} // namespace hubflow::audit::adapters

#endif // HUBFLOW_AUDIT_ADAPTERS_IN_HTTP_AUDIT_CONTROLLER_HPP
