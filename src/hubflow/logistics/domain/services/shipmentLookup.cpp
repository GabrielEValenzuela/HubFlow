#include "hubflow/logistics/domain/services/shipmentLookup.hpp"

namespace hubflow::logistics::domain
{

    LookupResult LinearShipmentLookup::find(const ShipmentCatalog& catalog, const std::string& code) const
    {
        LookupResult result;
        for (const auto* pNode = catalog.allShipments().head(); pNode != nullptr && result.pShipment == nullptr;
             pNode = pNode->pNext)
        {
            ++result.visited;
            if (pNode->value->code() == code)
            {
                result.pShipment = pNode->value;
            }
        }
        return result;
    }

    LookupResult BstShipmentLookup::find(const ShipmentCatalog& catalog, const std::string& code) const
    {
        const auto lookup = catalog.bst().find(code);
        return LookupResult {lookup.pValue, lookup.visited};
    }

    LookupResult BPlusShipmentLookup::find(const ShipmentCatalog& catalog, const std::string& code) const
    {
        const auto lookup = catalog.bPlusTree().find(code);
        return LookupResult {lookup.pValue, lookup.visited};
    }

} // namespace hubflow::logistics::domain
