#include "hubflow/insights/application/usecases/queries/getCodeSet.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/insights/application/usecases/queries/collectCodes.hpp"

namespace hubflow::insights::application
{

    GetCodeSet::GetCodeSet(const logistics::application::LogisticsModel& model)
        : m_model {model}
    {
    }

    domain::CodeSet GetCodeSet::execute(const shared::application::Principal& principal,
                                        const SetSelector& selector) const
    {
        principal.require(identity::domain::Permission::ViewIndexes);
        return collectCodes(m_model.catalog, selector);
    }

} // namespace hubflow::insights::application
