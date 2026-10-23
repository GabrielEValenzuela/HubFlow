#include "hubflow/insights/domain/services/stateComparison.hpp"

namespace hubflow::insights::domain
{

StateComparison compareStates(const logistics::domain::Shipment& first,
                              const logistics::domain::Shipment& second) noexcept
{
    const logistics::domain::StateHistory& a = first.stateHistory();
    const logistics::domain::StateHistory& b = second.stateHistory();
    const logistics::domain::StateHistory  exclusive {a ^ b};
    return StateComparison {a & b, exclusive & a, exclusive & b, a | b, exclusive};
}

} // namespace hubflow::insights::domain
