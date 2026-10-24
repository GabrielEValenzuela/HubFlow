#include "hubflow/insights/application/usecases/queries/getBPlusTree.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::insights::application
{

    GetBPlusTree::GetBPlusTree(const logistics::application::LogisticsModel& model)
        : m_model {model}
    {
    }

    const logistics::domain::ShipmentBPlusTree&
    GetBPlusTree::execute(const shared::application::Principal& principal) const
    {
        principal.require(identity::domain::Permission::ViewIndexes);
        return m_model.catalog.bPlusTree();
    }

} // namespace hubflow::insights::application
