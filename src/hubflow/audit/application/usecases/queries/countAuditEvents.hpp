#ifndef HUBFLOW_AUDIT_APPLICATION_USECASES_QUERIES_COUNT_AUDIT_EVENTS_HPP
#define HUBFLOW_AUDIT_APPLICATION_USECASES_QUERIES_COUNT_AUDIT_EVENTS_HPP

#include "hubflow/audit/application/ports/in/countAuditEventsUseCase.hpp"
#include "hubflow/audit/application/ports/out/auditLog.hpp"

namespace hubflow::audit::application
{

    /// @brief Implementación de CountAuditEventsUseCase.
    class CountAuditEvents final : public CountAuditEventsUseCase
    {
    public:
        explicit CountAuditEvents(const AuditLog& log);

    private:
        std::size_t execute(const shared::application::Principal& principal) const override;

        const AuditLog& m_log;
    };

} // namespace hubflow::audit::application

#endif // HUBFLOW_AUDIT_APPLICATION_USECASES_QUERIES_COUNT_AUDIT_EVENTS_HPP
