#include "hubflow/logistics/application/usecases/queries/findShipmentGroupOfShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    FindShipmentGroupOfShipment::FindShipmentGroupOfShipment(const LogisticsModel& model)
        : m_model {model}
    {
    }

    const domain::ShipmentGroup* FindShipmentGroupOfShipment::execute(const shared::application::Principal& principal,
                                                                      const std::string& shipmentCode) const
    {
        principal.require(identity::domain::Permission::OperateShipments);
        return m_model.groups.groupOf(m_model.catalog.require(shipmentCode).code());
    }

} // namespace hubflow::logistics::application
