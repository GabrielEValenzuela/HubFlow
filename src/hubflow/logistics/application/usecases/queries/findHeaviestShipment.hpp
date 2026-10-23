#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_HEAVIEST_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_HEAVIEST_SHIPMENT_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/findHeaviestShipmentUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de FindHeaviestShipmentUseCase.
class FindHeaviestShipment final : public FindHeaviestShipmentUseCase
{
public:
    explicit FindHeaviestShipment(const LogisticsModel& model);

private:
    const domain::Shipment* execute(const shared::application::Principal& principal,
                                    const std::string&                    zoneCode) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_HEAVIEST_SHIPMENT_HPP
