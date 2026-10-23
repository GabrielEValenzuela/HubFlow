#include "hubflow/logistics/application/usecases/queries/getShipmentGroup.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

GetShipmentGroup::GetShipmentGroup(const LogisticsModel& model)
    : m_model {model}
{
}

const domain::ShipmentGroup& GetShipmentGroup::execute(const shared::application::Principal& principal,
                                                       const std::string&                    groupId) const
{
    principal.require(identity::domain::Permission::OperateShipments);
    return m_model.groups.require(groupId);
}

} // namespace hubflow::logistics::application
