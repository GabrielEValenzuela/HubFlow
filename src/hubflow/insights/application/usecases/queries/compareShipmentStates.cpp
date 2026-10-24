#include "hubflow/insights/application/usecases/queries/compareShipmentStates.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"

namespace hubflow::insights::application
{

    CompareShipmentStates::CompareShipmentStates(const logistics::application::LogisticsModel& model)
        : m_model {model}
    {
    }

    domain::StateComparison CompareShipmentStates::execute(const shared::application::Principal& principal,
                                                           const std::string& firstCode,
                                                           const std::string& secondCode) const
    {
        principal.require(identity::domain::Permission::ViewIndexes);
        return domain::compareStates(m_model.catalog.require(firstCode), m_model.catalog.require(secondCode));
    }

} // namespace hubflow::insights::application
