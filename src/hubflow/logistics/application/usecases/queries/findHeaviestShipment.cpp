#include "hubflow/logistics/application/usecases/queries/findHeaviestShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

FindHeaviestShipment::FindHeaviestShipment(const LogisticsModel& model)
    : m_model {model}
{
}

const domain::Shipment* FindHeaviestShipment::execute(const shared::application::Principal& principal,
                                                      domain::Zone                          zone) const
{
    principal.require(identity::domain::Permission::OperateShipments);
    return domain::heaviestInZone(m_model.pending.head(), zone);
}

} // namespace hubflow::logistics::application
