#include "hubflow/logistics/application/usecases/queries/findHeaviestShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    FindHeaviestShipment::FindHeaviestShipment(const LogisticsModel& model)
        : m_model {model}
    {
    }

    const domain::Shipment* FindHeaviestShipment::execute(const shared::application::Principal& principal,
                                                          const std::string& zoneCode) const
    {
        principal.require(identity::domain::Permission::OperateShipments);
        m_model.zones.require(zoneCode);
        return domain::heaviestInZone(m_model.pending.head(), zoneCode);
    }

} // namespace hubflow::logistics::application
