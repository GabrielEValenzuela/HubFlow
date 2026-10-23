#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_ROAD_MAP_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_ROAD_MAP_HPP

#include "hubflow/routing/application/ports/in/getRoadMapUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"

namespace hubflow::routing::application
{

/// @brief Implementación de GetRoadMapUseCase.
class GetRoadMap final : public GetRoadMapUseCase
{
public:
    explicit GetRoadMap(const RoutingModel& model);

private:
    const domain::RoadMap& execute() const override;

    const RoutingModel& m_model;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_ROAD_MAP_HPP
