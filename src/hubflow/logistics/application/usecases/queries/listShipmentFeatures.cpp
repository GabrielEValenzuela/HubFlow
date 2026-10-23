#include "hubflow/logistics/application/usecases/queries/listShipmentFeatures.hpp"

namespace hubflow::logistics::application
{

ListShipmentFeatures::ListShipmentFeatures(const LogisticsModel& model)
    : m_model {model}
{
}

const domain::FeatureCatalog& ListShipmentFeatures::execute(const shared::application::Principal&) const
{
    return m_model.features;
}

} // namespace hubflow::logistics::application
