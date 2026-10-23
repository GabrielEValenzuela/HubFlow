#include "hubflow/sorting/domain/entities/sortingBatch.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <stdexcept>

namespace hubflow::sorting::domain
{
namespace
{
std::size_t countInZone(const logistics::domain::PendingNode* pNode, const logistics::domain::Zone zone)
{
    std::size_t count {0};
    for (; pNode != nullptr; pNode = pNode->pNext)
    {
        count += pNode->pShipment->zone() == zone ? 1 : 0;
    }
    return count;
}
} // namespace

SortingBatch::SortingBatch(const std::uint64_t                        id,
                           const logistics::domain::Zone              zone,
                           const logistics::domain::PendingShipments& pending)
    : m_id {id}
    , m_zone {zone}
    , m_size {countInZone(pending.head(), zone)}
{
    if (m_size == 0)
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                              "sorting.empty_zone",
                                              "No hay envíos pendientes para la zona seleccionada."};
    }
    m_ppItems = new logistics::domain::Shipment*[m_size];
    std::size_t index {0};
    for (const logistics::domain::PendingNode* pNode = pending.head(); pNode != nullptr; pNode = pNode->pNext)
    {
        if (pNode->pShipment->zone() == zone)
        {
            m_ppItems[index++] = pNode->pShipment;
        }
    }
}

SortingBatch::~SortingBatch()
{
    delete[] m_ppItems;
}

SortStats SortingBatch::sort(const ShipmentOrdering& ordering)
{
    const SortStats stats {insertionSort(m_ppItems, m_size, ordering)};
    m_sortedBy = ordering.criteria();
    return stats;
}

void SortingBatch::prepare()
{
    m_preparation.clear();
    for (std::size_t i = 0; i < m_size; ++i)
    {
        m_preparation.enqueue(m_ppItems[i]);
    }
}

logistics::domain::Shipment& SortingBatch::processNext()
{
    if (m_preparation.isEmpty())
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Conflict, "sorting.queue_empty", "La cola de preparación está vacía"};
    }
    logistics::domain::Shipment* pShipment = m_preparation.dequeue();
    m_processed.push(pShipment);
    return *pShipment;
}

logistics::domain::Shipment& SortingBatch::returnProcessed(const std::string& code)
{
    if (m_processed.isEmpty() || m_processed.top()->code() != code)
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::NotFound,
                                              "sorting.not_last_processed",
                                              "El envío no es el último procesado del lote"};
    }
    return *m_processed.pop();
}

ds::DynamicArray<logistics::domain::Shipment*> SortingBatch::unloadProcessed()
{
    ds::DynamicArray<logistics::domain::Shipment*> unloaded;
    while (!m_processed.isEmpty())
    {
        unloaded.pushBack(m_processed.pop());
    }
    return unloaded;
}

std::uint64_t SortingBatch::id() const noexcept
{
    return m_id;
}

logistics::domain::Zone SortingBatch::zone() const noexcept
{
    return m_zone;
}

std::size_t SortingBatch::size() const noexcept
{
    return m_size;
}

SortCriteria SortingBatch::sortedBy() const noexcept
{
    return m_sortedBy;
}

const logistics::domain::Shipment& SortingBatch::at(const std::size_t index) const
{
    if (index >= m_size)
    {
        throw std::out_of_range("Índice de lote fuera de rango");
    }
    return *m_ppItems[index];
}

const ds::Queue<logistics::domain::Shipment*>& SortingBatch::preparation() const noexcept
{
    return m_preparation;
}

const ds::Stack<logistics::domain::Shipment*>& SortingBatch::processed() const noexcept
{
    return m_processed;
}

} // namespace hubflow::sorting::domain
