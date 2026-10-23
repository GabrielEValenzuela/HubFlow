#ifndef HUBFLOW_AUDIT_APPLICATION_USECASES_QUERIES_LIST_AUDIT_EVENTS_HPP
#define HUBFLOW_AUDIT_APPLICATION_USECASES_QUERIES_LIST_AUDIT_EVENTS_HPP

#include "hubflow/audit/application/ports/in/listAuditEventsUseCase.hpp"
#include "hubflow/audit/application/ports/out/auditLog.hpp"

namespace hubflow::audit::application
{

/// @brief Implementación de ListAuditEventsUseCase.
class ListAuditEvents final : public ListAuditEventsUseCase
{
public:
    explicit ListAuditEvents(const AuditLog& log);

private:
    void execute(const shared::application::Principal& principal,
                 std::size_t                           limit,
                 std::size_t                           offset,
                 AuditEntryVisitor&                    visitor) const override;

    const AuditLog& m_log;
};

} // namespace hubflow::audit::application

#endif // HUBFLOW_AUDIT_APPLICATION_USECASES_QUERIES_LIST_AUDIT_EVENTS_HPP
