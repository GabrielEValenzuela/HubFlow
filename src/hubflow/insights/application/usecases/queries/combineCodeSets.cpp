#include "hubflow/insights/application/usecases/queries/combineCodeSets.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/insights/application/usecases/queries/collectCodes.hpp"

namespace hubflow::insights::application
{
namespace
{
const domain::UnionOperation        gs_union;
const domain::IntersectionOperation gs_intersection;
const domain::DifferenceOperation   gs_difference;

const domain::SetOperation& operationFor(const SetOperationKind kind)
{
    const domain::SetOperation* pOperation = &gs_union;
    if (kind == SetOperationKind::Intersection)
    {
        pOperation = &gs_intersection;
    }
    else if (kind == SetOperationKind::Difference)
    {
        pOperation = &gs_difference;
    }
    return *pOperation;
}
} // namespace

CombineCodeSets::CombineCodeSets(const logistics::application::LogisticsModel& model)
    : m_model {model}
{
}

SetCombination CombineCodeSets::execute(const shared::application::Principal& principal,
                                        SetOperationKind                      operation,
                                        const SetSelector&                    first,
                                        const SetSelector&                    second) const
{
    principal.require(identity::domain::Permission::ViewIndexes);
    SetCombination combination {collectCodes(m_model.catalog, first), collectCodes(m_model.catalog, second), {}, {}};
    combination.result = operationFor(operation).apply(combination.first, combination.second);
    combination.result.forEach(
        [this, &combination](const std::string& code)
        {
            combination.shipments.pushBack(&m_model.catalog.require(code));
        });
    return combination;
}

} // namespace hubflow::insights::application
