#include "hubflow/logistics/domain/services/ticketDispenser.hpp"

namespace hubflow::logistics::domain
{

QueueTicket TicketDispenser::next() noexcept
{
    return QueueTicket {++m_last};
}

void TicketDispenser::acknowledge(const QueueTicket& issued) noexcept
{
    if (issued.number() > m_last)
    {
        m_last = issued.number();
    }
}

} // namespace hubflow::logistics::domain
