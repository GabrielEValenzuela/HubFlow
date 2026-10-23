#include "hubflow/logistics/application/usecases/commands/classifyShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

ClassifyShipment::ClassifyShipment(LogisticsModel&                   model,
                                   shared::application::EventBus&    bus,
                                   const shared::application::Clock& clock)
    : m_model {model}
    , m_bus {bus}
    , m_clock {clock}
{
}

const domain::Shipment& ClassifyShipment::execute(const shared::application::Principal& principal,
                                                  const std::string&                    code,
                                                  const std::string&                    note)
{
    principal.require(identity::domain::Permission::OperateShipments);
    domain::Shipment& shipment = m_model.catalog.require(code);
    shipment.classify(note, principal.contextAt(m_clock));
    shipment.publishEvents(m_bus);
    return shipment;
}

} // namespace hubflow::logistics::application
