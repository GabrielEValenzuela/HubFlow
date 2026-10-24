#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_RESOLVE_ROUTE_STOP_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_RESOLVE_ROUTE_STOP_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/routing/application/ports/in/resolveRouteStopUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::routing::application
{

    /// @brief Implementación de ResolveRouteStopUseCase.
    class ResolveRouteStop final : public ResolveRouteStopUseCase
    {
    public:
        ResolveRouteStop(logistics::application::LogisticsModel& logistics,
                         RoutingModel& routing,
                         shared::application::EventBus& bus,
                         const shared::application::Clock& clock);

    private:
        const domain::DeliveryRoute& execute(const shared::application::Principal& principal,
                                             const ResolveStopRequest& request) override;

        logistics::application::LogisticsModel& m_logistics;
        RoutingModel& m_routing;
        shared::application::EventBus& m_bus;
        const shared::application::Clock& m_clock;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_RESOLVE_ROUTE_STOP_HPP
