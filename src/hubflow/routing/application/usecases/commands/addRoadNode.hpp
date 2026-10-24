#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_ADD_ROAD_NODE_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_ADD_ROAD_NODE_HPP

#include "hubflow/routing/application/ports/in/addRoadNodeUseCase.hpp"
#include "hubflow/routing/application/ports/out/zoneDirectory.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::routing::application
{

    /// @brief Implementación de AddRoadNodeUseCase.
    class AddRoadNode final : public AddRoadNodeUseCase
    {
    public:
        AddRoadNode(RoutingModel& routing,
                    const ZoneDirectory& zones,
                    shared::application::EventBus& bus,
                    const shared::application::Clock& clock);

    private:
        const domain::RoadNode& execute(const shared::application::Principal& principal,
                                        const AddRoadNodeRequest& request) override;

        RoutingModel& m_routing;
        const ZoneDirectory& m_zones;
        shared::application::EventBus& m_bus;
        const shared::application::Clock& m_clock;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_ADD_ROAD_NODE_HPP
