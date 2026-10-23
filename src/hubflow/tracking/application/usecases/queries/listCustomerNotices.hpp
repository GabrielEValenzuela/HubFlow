#ifndef HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_LIST_CUSTOMER_NOTICES_HPP
#define HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_LIST_CUSTOMER_NOTICES_HPP

#include "hubflow/tracking/application/ports/in/listCustomerNoticesUseCase.hpp"

namespace hubflow::tracking::application
{

/// @brief Implementación de ListCustomerNoticesUseCase.
class ListCustomerNotices final : public ListCustomerNoticesUseCase
{
public:
    explicit ListCustomerNotices(const CustomerNoticeLog& log);

private:
    std::size_t execute(const shared::application::Principal& principal,
                        std::size_t                           limit,
                        std::size_t                           offset,
                        CustomerNoticeVisitor&                visitor) const override;

    const CustomerNoticeLog& m_log;
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_LIST_CUSTOMER_NOTICES_HPP
