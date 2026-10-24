#include "hubflow/insights/application/usecases/queries/lookupInBst.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::insights::application
{

    LookupInBst::LookupInBst(const logistics::application::LogisticsModel& model)
        : m_model {model}
    {
    }

    BstLookup LookupInBst::execute(const shared::application::Principal& principal, const std::string& code) const
    {
        principal.require(identity::domain::Permission::ViewIndexes);
        BstLookup result;
        const auto lookup = m_model.catalog.bst().find(code,
                                                       [&result](const std::string& key, const ds::BstStep step)
                                                       { result.path.pushBack(BstVisit {key, step}); });
        result.pShipment = lookup.pValue;
        result.visited = lookup.visited;
        return result;
    }

} // namespace hubflow::insights::application
