#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_MOVEMENT_HISTORY_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_MOVEMENT_HISTORY_HPP

#include "datastructures/doublyLinkedList.hpp"
#include "hubflow/logistics/domain/valueobjects/movement.hpp"
#include "hubflow/shared/domain/actorContext.hpp"

namespace hubflow::logistics::domain
{

/**
 * @brief Historial bidireccional de movimientos de un envío; forma parte del agregado Shipment.
 *
 * Se apoya en una lista doblemente enlazada propia: agregar es O(1) gracias al puntero a la cola
 * y el recorrido inverso usa los enlaces @c pPrev. El historial muere con su envío.
 */
class MovementHistory final
{
public:
    /// @brief Agrega al final asignando el siguiente número secuencial. O(1).
    const Movement& record(ShipmentStatus status, const std::string& note, const shared::domain::ActorContext& context);

    /// @brief Restaura un movimiento persistido respetando su número. O(1).
    void restore(const Movement& movement);

    /// @brief Del más antiguo al más reciente. O(n).
    template <typename TVisitor>
    void forEachChronological(TVisitor visit) const
    {
        m_movements.forEach(visit);
    }

    /// @brief Del más reciente al más antiguo, usando los punteros anteriores. O(n).
    template <typename TVisitor>
    void forEachReverse(TVisitor visit) const
    {
        m_movements.forEachReverse(visit);
    }

    /// @brief Último movimiento, o nullptr si el historial está vacío.
    const Movement* latest() const noexcept;

    std::size_t size() const noexcept;

private:
    ds::DoublyLinkedList<Movement> m_movements;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_MOVEMENT_HISTORY_HPP
