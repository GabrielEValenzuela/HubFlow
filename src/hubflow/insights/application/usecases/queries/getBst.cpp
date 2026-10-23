#include "hubflow/insights/application/usecases/queries/getBst.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::insights::application
{

GetBst::GetBst(const logistics::application::LogisticsModel& model)
    : m_model {model}
{
}

const logistics::domain::ShipmentBst& GetBst::execute(const shared::application::Principal& principal) const
{
    principal.require(identity::domain::Permission::ViewIndexes);
    return m_model.catalog.bst();
}

} // namespace hubflow::insights::application
