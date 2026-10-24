#include "hubflow/logistics/application/usecases/queries/listPendingShipments.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::logistics::application
{

    ListPendingShipments::ListPendingShipments(const LogisticsModel& model)
        : m_model {model}
    {
    }

    const domain::PendingShipments& ListPendingShipments::execute(const shared::application::Principal& principal) const
    {
        principal.require(identity::domain::Permission::OperateShipments);
        return m_model.pending;
    }

} // namespace hubflow::logistics::application
