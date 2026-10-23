#include "hubflow/logistics/domain/services/zoneReport.hpp"

namespace hubflow::logistics::domain
{

ZoneSummary summarizeZone(const PendingNode* pNode, const Zone zone)
{
    ZoneSummary summary;
    if (pNode != nullptr)
    {
        summary                  = summarizeZone(pNode->pNext, zone);
        const Shipment& shipment = *pNode->pShipment;
        if (shipment.zone() == zone)
        {
            summary.count += 1;
            summary.totalWeightKg += shipment.weightKg();
            summary.expressCount += shipment.service() == ServiceLevel::Express ? 1 : 0;
        }
    }
    return summary;
}

const Shipment* heaviestInZone(const PendingNode* pNode, const Zone zone)
{
    const Shipment* pHeaviest = nullptr;
    if (pNode != nullptr)
    {
        pHeaviest                 = heaviestInZone(pNode->pNext, zone);
        const Shipment* pCurrent  = pNode->pShipment;
        const bool      inZone    = pCurrent->zone() == zone;
        const bool      isHeavier = pHeaviest == nullptr || pCurrent->weightKg() >= pHeaviest->weightKg();
        if (inZone && isHeavier)
        {
            pHeaviest = pCurrent;
        }
    }
    return pHeaviest;
}

} // namespace hubflow::logistics::domain
