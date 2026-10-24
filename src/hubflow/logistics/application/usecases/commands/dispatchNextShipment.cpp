#include "hubflow/logistics/application/usecases/commands/dispatchNextShipment.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::application
{
    namespace
    {
        constexpr const char* DEFAULT_DISPATCH_NOTE {"Despachado desde el centro de distribución"};
    } // namespace

    DispatchNextShipment::DispatchNextShipment(LogisticsModel& model,
                                               shared::application::EventBus& bus,
                                               const shared::application::Clock& clock)
        : m_model {model}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    const domain::Shipment& DispatchNextShipment::execute(const shared::application::Principal& principal,
                                                          const std::string& note)
    {
        principal.require(identity::domain::Permission::OperateShipments);
        if (m_model.pending.isEmpty())
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::Conflict, "pending.empty", "No hay envíos pendientes"};
        }
        domain::Shipment& shipment = *m_model.pending.head()->pShipment;
        const auto context = principal.contextAt(m_clock);
        const std::string reason {note.empty() ? DEFAULT_DISPATCH_NOTE : note};
        const domain::ShipmentGroup* pGroup = m_model.groups.groupOf(shipment.code());
        shipment.dispatch(reason, context);
        shipment.publishEvents(m_bus);
        for (std::size_t i = 0; pGroup != nullptr && i < pGroup->codes().size(); ++i)
        {
            domain::Shipment& companion = m_model.catalog.require(pGroup->codes()[i]);
            if (companion.isAwaitingDispatch())
            {
                companion.dispatch(reason, context);
                companion.publishEvents(m_bus);
            }
        }
        return shipment;
    }

} // namespace hubflow::logistics::application
