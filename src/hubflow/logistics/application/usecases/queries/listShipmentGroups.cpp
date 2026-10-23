#include "hubflow/logistics/application/usecases/queries/listShipmentGroups.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

ListShipmentGroups::ListShipmentGroups(const LogisticsModel& model)
    : m_model {model}
{
}

const domain::ShipmentGroupBook& ListShipmentGroups::execute(const shared::application::Principal& principal) const
{
    principal.require(identity::domain::Permission::OperateShipments);
    return m_model.groups;
}

} // namespace hubflow::logistics::application
