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
                if (history.isSet(domain::bitOf(status)))
                {
                    states.push_back(Vocabulary::name(status));
                }
            }
            return states;
        }

        Json featureCodes(const domain::Shipment& shipment)
        {
            Json codes = Json::array();
            for (std::size_t featureIndex = 0; featureIndex < shipment.features().size(); ++featureIndex)
            {
                codes.push_back(shipment.features()[featureIndex]);
            }
            return codes;
        }

        std::string binary(const domain::StateHistory& history)
        {
            std::string text;
            for (std::size_t position = domain::STATUS_COUNT; position > 0; --position)
            {
                text.push_back(history.isSet(position - 1) ? '1' : '0');
            }
            return text;
        }
    } // namespace

    Json LogisticsJsonPresenter::shipmentSummary(const domain::Shipment& shipment)
    {
        return Json {{"id", shipment.id()},
                     {"code", shipment.code()},
                     {"recipient", shipment.recipient()},
                     {"recipientPhone", shipment.contact().phone().digits()},
                     {"recipientPostalCode", shipment.contact().postalCode().value()},
                     {"zone", shipment.zone()},
                     {"weightKg", rounded(shipment.weightKg())},
                     {"realWeightKg", rounded(shipment.realWeightKg())},
                     {"billableWeightKg", rounded(shipment.billableWeightKg())},
                     {"parcelCount", shipment.parcelCount()},
                     {"pendingParcels", shipment.pendingParcelCount()},
                     {"parcels", parcels(shipment)},
                     {"service", shipment.service()},
                     {"features", featureCodes(shipment)},
                     {"status", Vocabulary::name(shipment.status())},
                     {"attempts", shipment.attempts()},
                     {"deliveryPoint", shipment.deliveryPoint()}};
    }

    Json LogisticsJsonPresenter::parcels(const domain::Shipment& shipment)
    {
        Json items = Json::array();
        for (std::size_t parcelIndex = 0; parcelIndex < shipment.parcelCount(); ++parcelIndex)
        {
            const auto& parcel = shipment.parcel(parcelIndex);
            items.push_back(Json {{"ordinal", parcel.ordinal()},
                                  {"label", parcel.label()},
                                  {"weightKg", rounded(parcel.weightKg())},
                                  {"lengthCm", parcel.dimensions().lengthCm()},
                                  {"widthCm", parcel.dimensions().widthCm()},
                                  {"heightCm", parcel.dimensions().heightCm()},
                                  {"volumetricWeightKg", rounded(parcel.volumetricWeightKg())},
                                  {"billableWeightKg", rounded(parcel.billableWeightKg())},
                                  {"delivered", parcel.isDelivered()}});
        }
        return items;
    }

    Json LogisticsJsonPresenter::group(const domain::ShipmentGroup& group)
    {
        Json codes = Json::array();
        for (std::size_t codeIndex = 0; codeIndex < group.codes().size(); ++codeIndex)
        {
            codes.push_back(group.codes()[codeIndex]);
        }
        return Json {{"id", group.id()}, {"deliveryPoint", group.deliveryPoint()}, {"codes", codes}};
    }

    Json LogisticsJsonPresenter::groupOf(const domain::Shipment& shipment, const domain::ShipmentGroup* pGroup)
    {
        Json result = nullptr;
        if (pGroup != nullptr)
        {
            result = group(*pGroup);
            result["travelsWith"] = Json::array();
            for (std::size_t codeIndex = 0; codeIndex < pGroup->codes().size(); ++codeIndex)
            {
                if (pGroup->codes()[codeIndex] != shipment.code())
                {
                    result["travelsWith"].push_back(pGroup->codes()[codeIndex]);
                }
            }
        }
        return result;
    }

    Json LogisticsJsonPresenter::shipment(const domain::Shipment& shipment)
    {
        Json body = shipmentSummary(shipment);
        body["stateHistory"] = stateHistory(shipment.stateHistory());
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
        Json items = Json::array();
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
                     {"value", history.toWord()},
                     {"states", stateNames(history)},
                     {"count", history.activeCount()},
                     {"any", history.isAnySet()},
                     {"everRescheduled", history.isSet(domain::bitOf(domain::ShipmentStatus::Reprogramado))}};
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
            Json item = zoneSummary(entry.code, entry.summary);
            item["name"] = entry.name;
            item["active"] = entry.active;
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
        for (std::size_t shipmentIndex = 0; shipmentIndex < shipments.size(); ++shipmentIndex)
        {
            items.push_back(shipmentSummary(*shipments[shipmentIndex]));
        }
        return items;
    }

} // namespace hubflow::logistics::adapters
