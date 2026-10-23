#include "hubflow/logistics/application/usecases/queries/listShipments.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

ListShipments::ListShipments(const LogisticsModel& model)
    : m_model {model}
{
}

const domain::ShipmentCatalog& ListShipments::execute(const shared::application::Principal& principal) const
{
    principal.require(identity::domain::Permission::OperateShipments);
    return m_model.catalog;
}

} // namespace hubflow::logistics::application
