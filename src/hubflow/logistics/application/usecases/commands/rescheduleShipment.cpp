#include "hubflow/logistics/application/usecases/commands/rescheduleShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

RescheduleShipment::RescheduleShipment(LogisticsModel&                   model,
                                       shared::application::EventBus&    bus,
                                       const shared::application::Clock& clock)
    : m_model {model}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::Shipment& RescheduleShipment::execute(const shared::application::Principal& principal,
                                                    const std::string&                    code,
                                                    const std::string&                    reason)
{
    principal.require(identity::domain::Permission::ResolveStops);
    domain::Shipment& shipment = m_model.catalog.require(code);
    shipment.reschedule(reason, m_model.tickets.next(), principal.contextAt(m_clock));
    shipment.publishEvents(m_bus);
    return shipment;
}

} // namespace hubflow::logistics::application
