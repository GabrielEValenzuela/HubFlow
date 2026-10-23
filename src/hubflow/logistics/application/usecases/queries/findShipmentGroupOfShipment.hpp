#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_SHIPMENT_GROUP_OF_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_SHIPMENT_GROUP_OF_SHIPMENT_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/findShipmentGroupOfShipmentUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de FindShipmentGroupOfShipmentUseCase.
class FindShipmentGroupOfShipment final : public FindShipmentGroupOfShipmentUseCase
{
public:
    explicit FindShipmentGroupOfShipment(const LogisticsModel& model);

private:
    const domain::ShipmentGroup* execute(const shared::application::Principal& principal,
                                         const std::string&                    shipmentCode) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_SHIPMENT_GROUP_OF_SHIPMENT_HPP
