#include "hubflow/routing/application/usecases/queries/getRoadMap.hpp"

namespace hubflow::routing::application
{

GetRoadMap::GetRoadMap(const RoutingModel& model)
    : m_model {model}
{
}

const domain::RoadMap& GetRoadMap::execute() const
{
    return m_model.map;
}

} // namespace hubflow::routing::application
