#include "hubflow/insights/domain/services/stateComparison.hpp"

namespace hubflow::insights::domain
{

    StateComparison compareStates(const logistics::domain::Shipment& first,
                                  const logistics::domain::Shipment& second) noexcept
    {
        const logistics::domain::StateHistory& firstHistory = first.stateHistory();
        const logistics::domain::StateHistory& secondHistory = second.stateHistory();
        const logistics::domain::StateHistory exclusive {firstHistory ^ secondHistory};
        return StateComparison {firstHistory & secondHistory,
                                exclusive & firstHistory,
                                exclusive & secondHistory,
                                firstHistory | secondHistory,
                                exclusive};
    }

} // namespace hubflow::insights::domain
