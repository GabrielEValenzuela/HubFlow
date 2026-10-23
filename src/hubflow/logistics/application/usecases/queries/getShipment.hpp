#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_SHIPMENT_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/getShipmentUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de GetShipmentUseCase.
class GetShipment final : public GetShipmentUseCase
{
public:
    explicit GetShipment(const LogisticsModel& model);

private:
    const domain::Shipment& execute(const shared::application::Principal& principal,
                                    const std::string&                    code) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_SHIPMENT_HPP
