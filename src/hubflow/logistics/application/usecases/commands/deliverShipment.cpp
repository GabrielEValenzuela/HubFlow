#include "hubflow/logistics/application/usecases/commands/deliverShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

DeliverShipment::DeliverShipment(LogisticsModel&                   model,
                                 shared::application::EventBus&    bus,
                                 const shared::application::Clock& clock)
    : m_model {model}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::Shipment& DeliverShipment::execute(const shared::application::Principal& principal,
                                                 const std::string&                    code,
                                                 const std::string&                    note,
                                                 const bool                            confirmAllParcels)
{
    principal.require(identity::domain::Permission::ResolveStops);
    domain::Shipment& shipment = m_model.catalog.require(code);
    const auto        context  = principal.contextAt(m_clock);
    if (confirmAllParcels && shipment.status() == domain::ShipmentStatus::EnReparto)
    {
        shipment.confirmAllParcels(context);
    }
    shipment.deliver(note, context);
    shipment.publishEvents(m_bus);
    return shipment;
}

} // namespace hubflow::logistics::application
