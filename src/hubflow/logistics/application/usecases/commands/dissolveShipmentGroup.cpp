#include "hubflow/logistics/application/usecases/commands/dissolveShipmentGroup.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

DissolveShipmentGroup::DissolveShipmentGroup(LogisticsModel&                   model,
                                             shared::application::EventBus&    bus,
                                             const shared::application::Clock& clock)
    : m_model {model}
    , m_bus {bus}
    , m_clock {clock}
{
}

void DissolveShipmentGroup::execute(const shared::application::Principal& principal, const std::string& groupId)
{
    principal.require(identity::domain::Permission::OperateShipments);
    domain::ShipmentGroup& group = m_model.groups.require(groupId);
    m_model.groups.ensureDissolvable(group, m_model.catalog);
    group.dissolve(principal.contextAt(m_clock));
    group.publishEvents(m_bus);
    m_model.groups.discard(groupId);
}

} // namespace hubflow::logistics::application
