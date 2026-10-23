#include "hubflow/logistics/application/subscribers/shipmentPersistenceSubscriber.hpp"

namespace hubflow::logistics::application
{

ShipmentPersistenceSubscriber::ShipmentPersistenceSubscriber(ShipmentRepository& repository)
    : m_repository {repository}
{
}

void ShipmentPersistenceSubscriber::on(const domain::ShipmentRegistered& event)
{
    m_repository.add(event.state());
    m_repository.appendMovement(event.code(), event.movement());
}

void ShipmentPersistenceSubscriber::on(const domain::ShipmentClassified& event)
{
    persistChange(event);
}

void ShipmentPersistenceSubscriber::on(const domain::ShipmentDispatched& event)
{
    persistChange(event);
}

void ShipmentPersistenceSubscriber::on(const domain::ShipmentRescheduled& event)
{
    persistChange(event);
}

void ShipmentPersistenceSubscriber::on(const domain::ShipmentDelivered& event)
{
    persistChange(event);
}

void ShipmentPersistenceSubscriber::persistChange(const domain::ShipmentEvent& event)
{
    m_repository.update(event.state());
    m_repository.appendMovement(event.code(), event.movement());
}

} // namespace hubflow::logistics::application
