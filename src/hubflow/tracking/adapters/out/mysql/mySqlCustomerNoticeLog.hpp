#ifndef HUBFLOW_TRACKING_ADAPTERS_OUT_MYSQL_MY_SQL_CUSTOMER_NOTICE_LOG_HPP
#define HUBFLOW_TRACKING_ADAPTERS_OUT_MYSQL_MY_SQL_CUSTOMER_NOTICE_LOG_HPP

#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/tracking/application/ports/out/customerNoticeLog.hpp"

namespace hubflow::tracking::adapters
{

    /// @brief Avisos a clientes guardados en la tabla @c notifications.
    class MySqlCustomerNoticeLog final : public application::CustomerNoticeLog
    {
    public:
        explicit MySqlCustomerNoticeLog(platform::MySqlConnection& connection);

    private:
        void append(const application::CustomerNotice& notice) override;
        void forEachRecent(std::size_t limit,
                           std::size_t offset,
                           application::CustomerNoticeVisitor& visitor) const override;
        std::size_t count() const override;

        platform::MySqlConnection& m_connection;
    };

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_OUT_MYSQL_MY_SQL_CUSTOMER_NOTICE_LOG_HPP
