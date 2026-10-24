#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PENDING_SHIPMENTS_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PENDING_SHIPMENTS_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/logistics/domain/services/priorityPolicy.hpp"

namespace hubflow::logistics::domain
{

/// @brief Nodo de pendientes: referencia NO propietaria a un envío y enlace al siguiente.
struct PendingNode
{
    Shipment*    pShipment;
    PendingNode* pNext;
};

/**
 * @brief Lista simplemente enlazada de envíos que esperan despacho.
 *
 * Funciona como cola de prioridad estable: mayor prioridad primero (según la PriorityPolicy recibida) y, a igual
 * prioridad, menor QueueTicket primero. Como los tickets crecen con cada llegada, insertar equivale a ubicar el
 * nodo detrás del último con prioridad mayor o igual.
 *
 * Ownership: crea y destruye sólo sus PendingNode; nunca ejecuta @c delete sobre un Shipment.
 *
 * | Operación | Mejor | Peor |
 * |-----------|-------|------|
 * | insert    | O(1)  | O(n) |
 * | popFront  | O(1)  | O(1) |
 * | remove    | O(1)  | O(n) |
 */
class PendingShipments final
{
public:
    /// @brief La política debe sobrevivir a la lista.
    explicit PendingShipments(const PriorityPolicy& policy) noexcept;

    ~PendingShipments();

    PendingShipments(const PendingShipments&)            = delete;
    PendingShipments& operator=(const PendingShipments&) = delete;

    /// @brief Inserta contemplando lista vacía, inicio, medio y final.
    void insert(Shipment* pShipment);

    /// @brief Extrae el primer nodo sin destruir el envío. nullptr si está vacía.
    Shipment* popFront() noexcept;

    /// @brief Quita el nodo de @p pShipment sin destruir el envío. false si no estaba.
    bool remove(const Shipment* pShipment) noexcept;

    bool               contains(const Shipment* pShipment) const noexcept;
    const PendingNode* head() const noexcept;
    std::size_t        size() const noexcept;
    bool               isEmpty() const noexcept;

    /// @brief Visita los envíos en orden de despacho.
    template <typename TVisitor>
    void forEach(TVisitor visit) const
    {
        for (const PendingNode* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext)
        {
            visit(*pNode->pShipment);
        }
    }

private:
    bool goesBefore(const Shipment* pCandidate, const Shipment* pExisting) const;

    const PriorityPolicy& m_policy;
    PendingNode*          m_pHead {nullptr};
    std::size_t           m_size {0};
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PENDING_SHIPMENTS_HPP
