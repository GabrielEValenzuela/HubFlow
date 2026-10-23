#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_TICKET_DISPENSER_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_TICKET_DISPENSER_HPP

#include "hubflow/logistics/domain/valueobjects/queueTicket.hpp"

namespace hubflow::logistics::domain
{

/// @brief Emite QueueTicket crecientes para cada llegada a pendientes.
class TicketDispenser final
{
public:
    /// @brief Próximo ticket. O(1).
    QueueTicket next() noexcept;

    /// @brief Garantiza que los próximos tickets sean mayores que @p issued (hidratación).
    void acknowledge(const QueueTicket& issued) noexcept;

private:
    std::uint64_t m_last {0};
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_TICKET_DISPENSER_HPP
