#include "hubflow/tracking/adapters/out/routing/routingDeliveryProgress.hpp"

#include "hubflow/routing/domain/services/travelTimePolicy.hpp"

#include <vector>

namespace hubflow::tracking::adapters
{
    namespace
    {
        using routing::domain::DeliveryRoute;
        using routing::domain::StopOutcome;

        std::optional<std::size_t> pendingStopOf(const DeliveryRoute& route, const std::string& code)
        {
            std::optional<std::size_t> index;
            for (std::size_t stopIndex = 0; stopIndex < route.stops().size() && !index.has_value(); ++stopIndex)
            {
                const auto& stop = route.stops()[stopIndex];
                if (stop.outcome == StopOutcome::Pending && stop.includes(code))
                {
                    index = stopIndex;
                }
            }
            return index;
        }

        std::optional<application::DeliveryProgress> progressOnRoute(const DeliveryRoute& route,
                                                                     const std::size_t target)
        {
            std::vector<double> legs;
            route.forEachSegment([&legs](const routing::domain::SearchTrace& trace)
                                 { legs.push_back(trace.metrics.costTotal); });
            std::size_t firstPending {0};
            while (firstPending < route.stops().size() && route.stops()[firstPending].outcome != StopOutcome::Pending)
            {
                ++firstPending;
            }
            double km {0.0};
            for (std::size_t i = firstPending; i <= target && i < legs.size(); ++i)
            {
                km += legs[i];
            }
            const std::size_t stopsToServe {target - firstPending + 1};
            return application::DeliveryProgress {routing::domain::TravelTimePolicy::estimateMinutes(km, stopsToServe),
                                                  stopsToServe - 1};
        }
    } // namespace

    RoutingDeliveryProgress::RoutingDeliveryProgress(const routing::application::RoutingModel& routing)
        : m_routing {routing}
    {
    }

    std::optional<application::DeliveryProgress> RoutingDeliveryProgress::progressOf(const std::string& code) const
    {
        std::optional<application::DeliveryProgress> progress;
        m_routing.routes.forEach(
            [&progress, &code](const DeliveryRoute& route)
            {
                const auto target = pendingStopOf(route, code);
                if (target.has_value())
                {
                    progress = progressOnRoute(route, *target);
                }
            });
        return progress;
    }

} // namespace hubflow::tracking::adapters
