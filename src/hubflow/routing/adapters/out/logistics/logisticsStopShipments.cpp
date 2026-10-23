#include "hubflow/routing/adapters/out/logistics/logisticsStopShipments.hpp"

namespace hubflow::routing::adapters
{

LogisticsStopShipments::LogisticsStopShipments(const logistics::application::LogisticsModel& model)
    : m_model {model}
{
}

std::optional<application::StopShipmentDetails> LogisticsStopShipments::detailsOf(const std::string& shipmentCode) const
{
    std::optional<application::StopShipmentDetails> details;
    if (m_model.catalog.contains(shipmentCode))
    {
        const auto& shipment = m_model.catalog.require(shipmentCode);
        details              = application::StopShipmentDetails {
            shipment.code(), shipment.recipient(), shipment.service(), shipment.zone(), shipment.weightKg(), {}};
        for (std::size_t i = 0; i < shipment.parcelCount(); ++i)
        {
            const auto& parcel = shipment.parcel(i);
            details->parcels.push_back(application::StopParcelDetails {
                parcel.ordinal(), parcel.label(), parcel.billableWeightKg(), parcel.delivered()});
        }
    }
    return details;
}

} // namespace hubflow::routing::adapters
