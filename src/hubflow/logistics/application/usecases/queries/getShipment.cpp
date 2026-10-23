#include "hubflow/logistics/application/usecases/queries/getShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

GetShipment::GetShipment(const LogisticsModel& model)
    : m_model {model}
{
}

const domain::Shipment& GetShipment::execute(const shared::application::Principal& principal,
                                             const std::string&                    code) const
{
    principal.require(identity::domain::Permission::OperateShipments);
    return m_model.catalog.require(code);
}

} // namespace hubflow::logistics::application
