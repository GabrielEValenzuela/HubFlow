#include "hubflow/sorting/domain/services/shipmentOrdering.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::sorting::domain
{
    namespace
    {
        const CodeAscendingOrdering gs_codeAscending;
        const WeightDescendingOrdering gs_weightDescending;
    } // namespace

    const ShipmentOrdering& ShipmentOrdering::of(const SortCriteria criteria)
    {
        if (criteria == SortCriteria::None)
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Validation, "sorting.criteria_invalid", "Criterio de orden inválido"};
        }
        return criteria == SortCriteria::CodeAscending ? static_cast<const ShipmentOrdering&>(gs_codeAscending)
                                                       : static_cast<const ShipmentOrdering&>(gs_weightDescending);
    }

    bool CodeAscendingOrdering::precedes(const logistics::domain::Shipment& candidate,
                                         const logistics::domain::Shipment& existing) const
    {
        return candidate.code() < existing.code();
    }

    SortCriteria CodeAscendingOrdering::criteria() const noexcept
    {
        return SortCriteria::CodeAscending;
    }

    bool WeightDescendingOrdering::precedes(const logistics::domain::Shipment& candidate,
                                            const logistics::domain::Shipment& existing) const
    {
        return candidate.weightKg() > existing.weightKg();
    }

    SortCriteria WeightDescendingOrdering::criteria() const noexcept
    {
        return SortCriteria::WeightDescending;
    }

} // namespace hubflow::sorting::domain
