#include "hubflow/insights/application/usecases/queries/lookupInBPlusTree.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::insights::application
{

LookupInBPlusTree::LookupInBPlusTree(const logistics::application::LogisticsModel& model)
    : m_model {model}
{
}

logistics::domain::LookupResult LookupInBPlusTree::execute(const shared::application::Principal& principal,
                                                           const std::string&                    code) const
{
    principal.require(identity::domain::Permission::ViewIndexes);
    const auto lookup = m_model.catalog.bPlusTree().find(code);
    return logistics::domain::LookupResult {lookup.pValue, lookup.visited};
}

} // namespace hubflow::insights::application
