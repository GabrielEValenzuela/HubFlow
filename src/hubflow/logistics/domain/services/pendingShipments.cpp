#include "hubflow/logistics/domain/services/pendingShipments.hpp"

namespace hubflow::logistics::domain
{

PendingShipments::PendingShipments(const PriorityPolicy& policy) noexcept
    : m_policy {policy}
{
}

PendingShipments::~PendingShipments()
{
    while (m_pHead != nullptr)
    {
        PendingNode* pNext = m_pHead->pNext;
        delete m_pHead;
        m_pHead = pNext;
    }
}

bool PendingShipments::goesBefore(const Shipment* pCandidate, const Shipment* pExisting) const
{
    const unsigned candidate {m_policy.priorityOf(*pCandidate)};
    const unsigned existing {m_policy.priorityOf(*pExisting)};
    const bool     higherPriority {candidate > existing};
    const bool     samePriorityEarlier {candidate == existing && pCandidate->ticket() < pExisting->ticket()};
    return higherPriority || samePriorityEarlier;
}

void PendingShipments::insert(Shipment* pShipment)
{
    PendingNode* pNode = new PendingNode {pShipment, nullptr};
    if (m_pHead == nullptr || goesBefore(pShipment, m_pHead->pShipment))
    {
        pNode->pNext = m_pHead;
        m_pHead      = pNode;
    }
    else
    {
        PendingNode* pPrevious = m_pHead;
        while (pPrevious->pNext != nullptr && !goesBefore(pShipment, pPrevious->pNext->pShipment))
        {
            pPrevious = pPrevious->pNext;
        }
        pNode->pNext     = pPrevious->pNext;
        pPrevious->pNext = pNode;
    }
    ++m_size;
}

Shipment* PendingShipments::popFront() noexcept
{
    Shipment* pShipment = nullptr;
    if (m_pHead != nullptr)
    {
        PendingNode* pFirst = m_pHead;
        pShipment           = pFirst->pShipment;
        m_pHead             = pFirst->pNext;
        delete pFirst;
        --m_size;
    }
    return pShipment;
}

bool PendingShipments::remove(const Shipment* pShipment) noexcept
{
    PendingNode** ppLink = &m_pHead;
    while (*ppLink != nullptr && (*ppLink)->pShipment != pShipment)
    {
        ppLink = &(*ppLink)->pNext;
    }
    const bool found {*ppLink != nullptr};
    if (found)
    {
        PendingNode* pDoomed = *ppLink;
        *ppLink              = pDoomed->pNext;
        delete pDoomed;
        --m_size;
    }
    return found;
}

bool PendingShipments::contains(const Shipment* pShipment) const noexcept
{
    const PendingNode* pNode = m_pHead;
    while (pNode != nullptr && pNode->pShipment != pShipment)
    {
        pNode = pNode->pNext;
    }
    return pNode != nullptr;
}

const PendingNode* PendingShipments::head() const noexcept
{
    return m_pHead;
}

std::size_t PendingShipments::size() const noexcept
{
    return m_size;
}

bool PendingShipments::isEmpty() const noexcept
{
    return m_pHead == nullptr;
}

} // namespace hubflow::logistics::domain
