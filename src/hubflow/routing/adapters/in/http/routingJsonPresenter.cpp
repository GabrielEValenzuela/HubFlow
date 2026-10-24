#include "hubflow/routing/adapters/in/http/routingJsonPresenter.hpp"

#include "hubflow/routing/adapters/out/export/routePlanJson.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

#include <cmath>

namespace hubflow::routing::adapters
{
    namespace
    {
        using platform::Json;
        using shared::application::Vocabulary;

        constexpr double ROUNDING {100.0};

        double rounded(const double value)
        {
            return std::round(value * ROUNDING) / ROUNDING;
        }
    } // namespace

    Json RoutingJsonPresenter::roadMap(const domain::RoadMap& map)
    {
        return RoutePlanJson::roadMap(map);
    }

    Json RoutingJsonPresenter::node(const domain::RoadNode& node)
    {
        return Json {{"id", node.id},
                     {"label", node.label},
                     {"zone", node.zone},
                     {"isHub", node.isHub},
                     {"x", node.x},
                     {"y", node.y},
                     {"active", node.active}};
    }

    Json RoutingJsonPresenter::searchTrace(const domain::SearchTrace& trace, const domain::RoadMap& map)
    {
        return RoutePlanJson::searchTrace(map, trace);
    }

    Json RoutingJsonPresenter::routeSummary(const domain::DeliveryRoute& route)
    {
        std::size_t resolved {0};
        for (std::size_t stopIndex = 0; stopIndex < route.stops().size(); ++stopIndex)
        {
            resolved += route.stops()[stopIndex].outcome == domain::StopOutcome::Pending ? 0 : 1;
        }
        return Json {{"id", route.id()},
                     {"courier", Json {{"id", route.courier().id}, {"name", route.courier().name}}},
                     {"algorithm", Vocabulary::name(route.algorithm())},
                     {"status", Vocabulary::name(route.status())},
                     {"createdAt", route.createdAt()},
                     {"stopCount", route.stops().size()},
                     {"resolvedStops", resolved},
                     {"totalCostKm", rounded(route.totalCostKm())},
                     {"nodesExplored", route.totalNodesExplored()}};
    }

    Json RoutingJsonPresenter::route(const domain::DeliveryRoute& route, const domain::RoadMap& map)
    {
        Json stops = Json::array();
        for (std::size_t stopIndex = 0; stopIndex < route.stops().size(); ++stopIndex)
        {
            const auto& stop = route.stops()[stopIndex];
            Json codes = Json::array();
            for (std::size_t codeIndex = 0; codeIndex < stop.shipmentCodes().size(); ++codeIndex)
            {
                codes.push_back(stop.shipmentCodes()[codeIndex]);
            }
            stops.push_back(Json {{"order", stop.order},
                                  {"shipmentCode", stop.shipmentCode},
                                  {"shipmentCodes", codes},
                                  {"nodeId", stop.nodeId},
                                  {"outcome", Vocabulary::name(stop.outcome)}});
        }
        Json body = routeSummary(route);
        body["stops"] = stops;
        body["sequence"] = sequence(route, map);
        return body;
    }

    Json RoutingJsonPresenter::courierSheet(const application::CourierRouteSheet& sheet, const domain::RoadMap& map)
    {
        Json body = route(sheet.route, map);
        for (std::size_t stopIndex = 0; stopIndex < sheet.stopDetails.size(); ++stopIndex)
        {
            Json shipments = Json::array();
            for (const auto& details : sheet.stopDetails[stopIndex])
            {
                if (details.has_value())
                {
                    Json parcels = Json::array();
                    for (const auto& parcel : details->parcels)
                    {
                        parcels.push_back(Json {{"ordinal", parcel.ordinal},
                                                {"label", parcel.label},
                                                {"billableWeightKg", rounded(parcel.billableWeightKg)},
                                                {"delivered", parcel.delivered}});
                    }
                    shipments.push_back(Json {{"code", details->code},
                                              {"recipient", details->recipient},
                                              {"service", details->service},
                                              {"zone", details->zone},
                                              {"weightKg", rounded(details->weightKg)},
                                              {"parcels", parcels}});
                }
            }
            body["stops"][stopIndex]["shipments"] = shipments;
            if (!shipments.empty())
            {
                body["stops"][stopIndex]["recipient"] = shipments[0]["recipient"];
                body["stops"][stopIndex]["service"] = shipments[0]["service"];
                body["stops"][stopIndex]["zone"] = shipments[0]["zone"];
                body["stops"][stopIndex]["weightKg"] = shipments[0]["weightKg"];
            }
        }
        return body;
    }

    Json RoutingJsonPresenter::initialState(const domain::DeliveryRoute& route, const domain::RoadMap& map)
    {
        return RoutePlanJson::initialState(map, route);
    }

    Json RoutingJsonPresenter::sequence(const domain::DeliveryRoute& route, const domain::RoadMap& map)
    {
        return RoutePlanJson::sequence(map, route);
    }

} // namespace hubflow::routing::adapters
