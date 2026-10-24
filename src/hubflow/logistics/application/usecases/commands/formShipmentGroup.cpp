#include "hubflow/logistics/application/usecases/commands/formShipmentGroup.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    FormShipmentGroup::FormShipmentGroup(LogisticsModel& model,
                                         shared::application::IdentityGenerator& identities,
                                         shared::application::EventBus& bus,
                                         const shared::application::Clock& clock)
        : m_model {model}
        , m_identities {identities}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::ShipmentGroup& FormShipmentGroup::execute(const shared::application::Principal& principal,
                                                            const ds::DynamicArray<std::string>& codes)
    {
        principal.require(identity::domain::Permission::OperateShipments);
        domain::ShipmentGroup& group =
            m_model.groups.form(m_identities.next(), codes, m_model.catalog, principal.contextAt(m_clock));
        group.publishEvents(m_bus);
        return group;
    }

} // namespace hubflow::logistics::application
