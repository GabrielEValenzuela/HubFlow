#include "hubflow/sorting/domain/entities/sortingBatch.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <stdexcept>

namespace hubflow::sorting::domain
{
    namespace
    {
        std::size_t countInZone(const logistics::domain::PendingNode* pNode, const std::string& zone)
        {
            std::size_t count {0};
            for (; pNode != nullptr; pNode = pNode->pNext)
            {
                count += pNode->pShipment->zone() == zone ? 1 : 0;
            }
            return count;
        }
    } // namespace

    SortingBatch::SortingBatch(const std::uint64_t id,
                               const std::string& zone,
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

    bool SortingBatch::isProcessed(const logistics::domain::Shipment* pShipment) const
    {
        bool found {false};
        m_processed.forEach([&found, pShipment](const logistics::domain::Shipment* pLoaded)
                            { found = found || pLoaded == pShipment; });
        return found;
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
            if (!isProcessed(m_ppItems[i]))
            {
                m_preparation.enqueue(m_ppItems[i]);
            }
        }
    }

    ds::DynamicArray<logistics::domain::Shipment*>
    SortingBatch::processNext(const logistics::domain::ShipmentGroupBook& groups)
    {
        if (m_preparation.isEmpty())
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Conflict, "sorting.queue_empty", "La cola de preparación está vacía"};
        }
        ds::DynamicArray<logistics::domain::Shipment*> selection;
        selection.pushBack(m_preparation.front());
        const logistics::domain::ShipmentGroup* pGroup = groups.groupOf(m_preparation.front()->code());
        if (pGroup != nullptr)
        {
            m_preparation.forEach(
                [&selection, pGroup](logistics::domain::Shipment* pQueued)
                {
                    if (pQueued != selection[0] && pGroup->includes(pQueued->code()))
                    {
                        selection.pushBack(pQueued);
                    }
                });
        }
        return load(selection, groups);
    }

    ds::DynamicArray<logistics::domain::Shipment*>
    SortingBatch::processByCodes(const ds::DynamicArray<std::string>& codes,
                                 const logistics::domain::ShipmentGroupBook& groups)
    {
        ds::DynamicArray<logistics::domain::Shipment*> selection;
        for (std::size_t codeIndex = 0; codeIndex < codes.size(); ++codeIndex)
        {
            logistics::domain::Shipment* pFound = nullptr;
            m_preparation.forEach([&pFound, &codes, codeIndex](logistics::domain::Shipment* pQueued)
                                  { pFound = pQueued->code() == codes[codeIndex] ? pQueued : pFound; });
            if (pFound == nullptr)
            {
                throw shared::domain::business_error {shared::domain::ErrorKind::NotFound,
                                                      "sorting.not_in_queue",
                                                      "El paquete " + codes[codeIndex] +
                                                          " no está en la lista por cargar"};
            }
            bool repeated {false};
            for (std::size_t selectedIndex = 0; selectedIndex < selection.size(); ++selectedIndex)
            {
                repeated = repeated || selection[selectedIndex] == pFound;
            }
            if (!repeated)
            {
                selection.pushBack(pFound);
            }
        }
        return load(selection, groups);
    }

    ds::DynamicArray<logistics::domain::Shipment*>
    SortingBatch::load(const ds::DynamicArray<logistics::domain::Shipment*>& selection,
                       const logistics::domain::ShipmentGroupBook& groups)
    {
        if (selection.isEmpty())
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Validation, "sorting.nothing_selected", "Indicá qué paquetes cargar"};
        }
        for (std::size_t requestedIndex = 0; requestedIndex < selection.size(); ++requestedIndex)
        {
            const logistics::domain::ShipmentGroup* pGroup = groups.groupOf(selection[requestedIndex]->code());
            for (std::size_t groupCodeIndex = 0; pGroup != nullptr && groupCodeIndex < pGroup->codes().size();
                 ++groupCodeIndex)
            {
                bool selected {false};
                for (std::size_t candidateIndex = 0; candidateIndex < selection.size(); ++candidateIndex)
                {
                    selected = selected || selection[candidateIndex]->code() == pGroup->codes()[groupCodeIndex];
                }
                if (!selected)
                {
                    throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                          "sorting.group_incomplete",
                                                          "El paquete " + selection[requestedIndex]->code() +
                                                              " viaja junto con " + pGroup->codes()[groupCodeIndex] +
                                                              ": hay que cargarlos todos juntos"};
                }
            }
        }
        ds::DynamicArray<logistics::domain::Shipment*> waiting;
        while (!m_preparation.isEmpty())
        {
            waiting.pushBack(m_preparation.dequeue());
        }
        for (std::size_t waitingIndex = 0; waitingIndex < waiting.size(); ++waitingIndex)
        {
            bool selected {false};
            for (std::size_t selectedIndex = 0; selectedIndex < selection.size(); ++selectedIndex)
            {
                selected = selected || selection[selectedIndex] == waiting[waitingIndex];
            }
            if (!selected)
            {
                m_preparation.enqueue(waiting[waitingIndex]);
            }
        }
        for (std::size_t selectedIndex = 0; selectedIndex < selection.size(); ++selectedIndex)
        {
            m_processed.push(selection[selectedIndex]);
        }
        return selection;
    }

    ds::DynamicArray<logistics::domain::Shipment*>
    SortingBatch::returnProcessed(const std::string& code, const logistics::domain::ShipmentGroupBook& groups)
    {
        if (m_processed.isEmpty() || m_processed.top()->code() != code)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::NotFound,
                                                  "sorting.not_last_processed",
                                                  "El envío no es el último procesado del lote"};
        }
        ds::DynamicArray<logistics::domain::Shipment*> returned;
        returned.pushBack(m_processed.pop());
        const logistics::domain::ShipmentGroup* pGroup = groups.groupOf(code);
        while (pGroup != nullptr && !m_processed.isEmpty() && pGroup->includes(m_processed.top()->code()))
        {
            returned.pushBack(m_processed.pop());
        }
        return returned;
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

    const std::string& SortingBatch::zone() const noexcept
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

    const logistics::domain::Shipment& SortingBatch::shipmentAt(const std::size_t index) const
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
