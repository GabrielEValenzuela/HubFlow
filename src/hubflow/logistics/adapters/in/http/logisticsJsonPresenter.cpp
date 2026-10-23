#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"

#include "hubflow/shared/application/vocabulary.hpp"

#include <cmath>

namespace hubflow::logistics::adapters
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

Json stateNames(const domain::StateHistory& history)
{
    Json states = Json::array();
    for (const auto status : domain::ALL_STATUSES)
    {
        if (history.test(domain::bitOf(status)))
        {
            states.push_back(Vocabulary::name(status));
        }
    }
    return states;
}

Json featureCodes(const domain::Shipment& shipment)
{
    Json codes = Json::array();
    for (std::size_t i = 0; i < shipment.features().size(); ++i)
    {
        codes.push_back(shipment.features()[i]);
    }
    return codes;
}

std::string binary(const domain::StateHistory& history)
{
    std::string text;
    for (std::size_t position = domain::STATUS_COUNT; position > 0; --position)
    {
        text.push_back(history.test(position - 1) ? '1' : '0');
    }
    return text;
}
} // namespace

Json LogisticsJsonPresenter::shipmentSummary(const domain::Shipment& shipment)
{
    return Json {{"code", shipment.code()},
                 {"recipient", shipment.recipient()},
                 {"zone", shipment.zone()},
                 {"weightKg", rounded(shipment.weightKg())},
                 {"service", shipment.service()},
                 {"features", featureCodes(shipment)},
                 {"status", Vocabulary::name(shipment.status())},
                 {"attempts", shipment.attempts()},
                 {"deliveryPoint", shipment.deliveryPoint()}};
}

Json LogisticsJsonPresenter::shipment(const domain::Shipment& shipment)
{
    Json body             = shipmentSummary(shipment);
    body["stateHistory"]  = stateHistory(shipment.stateHistory());
    body["movementCount"] = shipment.history().size();
    body["lastMovement"] =
        shipment.history().latest() == nullptr ? Json(nullptr) : movement(*shipment.history().latest());
    return body;
}

Json LogisticsJsonPresenter::movement(const domain::Movement& movement)
{
    return Json {{"sequence", movement.sequence},
                 {"status", Vocabulary::name(movement.status)},
                 {"note", movement.note},
                 {"actor", movement.actorName},
                 {"occurredAt", movement.occurredAt}};
}

Json LogisticsJsonPresenter::history(const domain::Shipment& shipment, const bool newestFirst)
{
    Json       items   = Json::array();
    const auto collect = [&items](const domain::Movement& entry)
    {
        items.push_back(movement(entry));
    };
    if (newestFirst)
    {
        shipment.history().forEachReverse(collect);
    }
    else
    {
        shipment.history().forEachChronological(collect);
    }
    return Json {{"code", shipment.code()}, {"order", newestFirst ? "desc" : "asc"}, {"items", items}};
}

Json LogisticsJsonPresenter::stateHistory(const domain::StateHistory& history)
{
    return Json {{"bits", binary(history)},
                 {"value", history.raw()},
                 {"states", stateNames(history)},
                 {"count", history.count()},
                 {"any", history.any()},
                 {"everRescheduled", history.test(domain::bitOf(domain::ShipmentStatus::Reprogramado))}};
}

Json LogisticsJsonPresenter::zoneSummary(const std::string& zoneCode, const domain::ZoneSummary& summary)
{
    return Json {{"zone", zoneCode},
                 {"count", summary.count},
                 {"totalWeightKg", rounded(summary.totalWeightKg)},
                 {"urgentCount", summary.urgentCount}};
}

Json LogisticsJsonPresenter::metrics(const application::OperationsMetrics& metrics)
{
    Json byStatus = Json::object();
    for (const auto status : domain::ALL_STATUSES)
    {
        byStatus[Vocabulary::name(status)] = metrics.byStatus[domain::bitOf(status)];
    }
    Json zones = Json::array();
    for (std::size_t i = 0; i < metrics.pendingByZone.size(); ++i)
    {
        const auto& entry = metrics.pendingByZone[i];
        Json        item  = zoneSummary(entry.code, entry.summary);
        item["name"]      = entry.name;
        item["active"]    = entry.active;
        zones.push_back(item);
    }
    return Json {{"totalShipments", metrics.totalShipments},
                 {"byStatus", byStatus},
                 {"totalAttempts", metrics.totalAttempts},
                 {"pendingByZone", zones}};
}

Json LogisticsJsonPresenter::shipments(const ds::DynamicArray<const domain::Shipment*>& shipments)
{
    Json items = Json::array();
    for (std::size_t i = 0; i < shipments.size(); ++i)
    {
        items.push_back(shipmentSummary(*shipments[i]));
    }
    return items;
}

} // namespace hubflow::logistics::adapters
