#include "hubflow/tracking/application/usecases/queries/listCustomerNotices.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::tracking::application
{

ListCustomerNotices::ListCustomerNotices(const CustomerNoticeLog& log)
    : m_log {log}
{
}

std::size_t ListCustomerNotices::execute(const shared::application::Principal& principal,
                                         const std::size_t                     limit,
                                         const std::size_t                     offset,
                                         CustomerNoticeVisitor&                visitor) const
{
    principal.require(identity::domain::Permission::ViewAudit);
    m_log.forEachRecent(limit, offset, visitor);
    return m_log.count();
}

} // namespace hubflow::tracking::application
