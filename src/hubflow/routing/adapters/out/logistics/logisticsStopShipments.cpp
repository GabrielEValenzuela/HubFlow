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
            shipment.recipient(), shipment.service(), shipment.zone(), shipment.weightKg()};
    }
    return details;
}

} // namespace hubflow::routing::adapters
