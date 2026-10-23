#include "hubflow/logistics/domain/entities/movementHistory.hpp"

namespace hubflow::logistics::domain
{

const Movement& MovementHistory::record(const ShipmentStatus                status,
                                        const std::string&                  note,
                                        const shared::domain::ActorContext& context)
{
    m_movements.pushBack(Movement {m_movements.size() + 1, status, note, context.actorName, context.occurredAt});
    return m_movements.tail()->value;
}

void MovementHistory::restore(const Movement& movement)
{
    m_movements.pushBack(movement);
}

const Movement* MovementHistory::latest() const noexcept
{
    return m_movements.tail() == nullptr ? nullptr : &m_movements.tail()->value;
}

std::size_t MovementHistory::size() const noexcept
{
    return m_movements.size();
}

} // namespace hubflow::logistics::domain
