#include "hubflow/logistics/domain/valueobjects/queueTicket.hpp"

namespace hubflow::logistics::domain
{

    QueueTicket::QueueTicket(const std::uint64_t number) noexcept
        : m_number {number}
    {
    }

    std::uint64_t QueueTicket::number() const noexcept
    {
        return m_number;
    }

    bool QueueTicket::operator<(const QueueTicket& other) const noexcept
    {
        return m_number < other.m_number;
    }

} // namespace hubflow::logistics::domain
