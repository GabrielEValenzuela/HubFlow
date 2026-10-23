#include "hubflow/logistics/application/subscribers/shipmentGroupPersistenceSubscriber.hpp"

namespace hubflow::logistics::application
{

ShipmentGroupPersistenceSubscriber::ShipmentGroupPersistenceSubscriber(ShipmentGroupRepository& repository)
    : m_repository {repository}
{
}

void ShipmentGroupPersistenceSubscriber::on(const domain::ShipmentGroupFormed& event)
{
    m_repository.add(event.state());
}

void ShipmentGroupPersistenceSubscriber::on(const domain::ShipmentGroupDissolved& event)
{
    m_repository.remove(event.state().id);
}

} // namespace hubflow::logistics::application
