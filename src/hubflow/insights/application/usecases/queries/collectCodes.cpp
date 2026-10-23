#include "hubflow/insights/application/usecases/queries/collectCodes.hpp"

namespace hubflow::insights::application
{

domain::CodeSet collectCodes(const logistics::domain::ShipmentCatalog& catalog, const SetSelector& selector)
{
    const domain::ZoneCriterion    byZone {selector.zone};
    const domain::ServiceCriterion byService {selector.service};
    const domain::SetCriterion&    criterion = selector.kind == SetSelector::Kind::Zone
                                                   ? static_cast<const domain::SetCriterion&>(byZone)
                                                   : static_cast<const domain::SetCriterion&>(byService);
    return criterion.collect(catalog);
}

} // namespace hubflow::insights::application
