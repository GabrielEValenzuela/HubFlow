#ifndef HUBFLOW_ROUTING_ADAPTERS_OUT_LOGISTICS_LOGISTICS_STOP_SHIPMENTS_HPP
#define HUBFLOW_ROUTING_ADAPTERS_OUT_LOGISTICS_LOGISTICS_STOP_SHIPMENTS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/routing/application/ports/out/stopShipmentDirectory.hpp"

namespace hubflow::routing::adapters
{

    /// @brief Resuelve los datos de entrega de cada parada consultando el catálogo del contexto de logística.
    class LogisticsStopShipments final : public application::StopShipmentDirectory
    {
    public:
        explicit LogisticsStopShipments(const logistics::application::LogisticsModel& model);

    private:
        std::optional<application::StopShipmentDetails> detailsOf(const std::string& shipmentCode) const override;

        const logistics::application::LogisticsModel& m_model;
    };

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_OUT_LOGISTICS_LOGISTICS_STOP_SHIPMENTS_HPP
