#include "hubflow/tracking/adapters/out/mysql/mySqlCustomerNoticeLog.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"

namespace hubflow::tracking::adapters
{
namespace
{
using platform::SqlColumns;

constexpr const char* INSERT_NOTICE {R"SQL(
    INSERT INTO notifications (shipment_code, channel, recipient_masked, message, sent_at) VALUES (?, ?, ?, ?, ?)
)SQL"};

constexpr const char* SELECT_NOTICES {R"SQL(
    SELECT id, shipment_code, channel, recipient_masked, message, sent_at
    FROM notifications ORDER BY id DESC LIMIT ? OFFSET ?
)SQL"};

constexpr const char* COUNT_NOTICES {"SELECT COUNT(*) FROM notifications"};
} // namespace

MySqlCustomerNoticeLog::MySqlCustomerNoticeLog(platform::MySqlConnection& connection)
    : m_connection {connection}
{
}

void MySqlCustomerNoticeLog::append(const application::CustomerNotice& notice)
{
    m_connection.execute(
        INSERT_NOTICE,
        {notice.shipmentCode, notice.channel, notice.recipient, notice.message, std::int64_t {notice.sentAt}});
}

void MySqlCustomerNoticeLog::forEachRecent(const std::size_t                   limit,
                                           const std::size_t                   offset,
                                           application::CustomerNoticeVisitor& visitor) const
{
    for (const auto& row : m_connection.query(SELECT_NOTICES, {std::uint64_t {limit}, std::uint64_t {offset}}))
    {
        visitor.visit(
            application::StoredCustomerNotice {SqlColumns::unsignedNumber(row, 0),
                                               application::CustomerNotice {SqlColumns::text(row, 1),
                                                                            SqlColumns::text(row, 2),
                                                                            SqlColumns::text(row, 3),
                                                                            SqlColumns::text(row, 4),
                                                                            SqlColumns::signedNumber(row, 5)}});
    }
}

std::size_t MySqlCustomerNoticeLog::count() const
{
    return SqlColumns::unsignedNumber(m_connection.query(COUNT_NOTICES).front(), 0);
}

} // namespace hubflow::tracking::adapters
