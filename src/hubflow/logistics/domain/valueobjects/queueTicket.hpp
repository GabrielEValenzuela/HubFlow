#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_QUEUE_TICKET_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_QUEUE_TICKET_HPP

#include <cstdint>

namespace hubflow::logistics::domain
{

/**
 * @brief Número de llegada a la lista de pendientes.
 *
 * Desempata envíos de igual prioridad: el de ticket menor llegó antes y sale antes, lo que hace
 * estable a la cola de prioridad.
 */
class QueueTicket final
{
public:
    explicit QueueTicket(std::uint64_t number = 0) noexcept;

    std::uint64_t number() const noexcept;

    bool operator<(const QueueTicket& other) const noexcept;

private:
    std::uint64_t m_number;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_QUEUE_TICKET_HPP
