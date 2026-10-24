#include "hubflow/logistics/application/usecases/queries/findShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/logistics/domain/services/shipmentLookup.hpp"

namespace hubflow::logistics::application
{
    namespace
    {
        const domain::LinearShipmentLookup gs_linearLookup;
        const domain::BstShipmentLookup gs_bstLookup;
        const domain::BPlusShipmentLookup gs_bPlusLookup;

        const domain::ShipmentLookup& lookupFor(const LookupIndex index)
        {
            const domain::ShipmentLookup* pLookup = &gs_bPlusLookup;
            if (index == LookupIndex::LinkedList)
            {
                pLookup = &gs_linearLookup;
            }
            else if (index == LookupIndex::Bst)
            {
                pLookup = &gs_bstLookup;
            }
            return *pLookup;
        }
    } // namespace

    FindShipment::FindShipment(const LogisticsModel& model)
        : m_model {model}
    {
    }

    domain::LookupResult FindShipment::execute(const shared::application::Principal& principal,
                                               const std::string& code,
                                               LookupIndex index) const
    {
        principal.require(identity::domain::Permission::OperateShipments);
        return lookupFor(index).find(m_model.catalog, code);
    }

} // namespace hubflow::logistics::application
