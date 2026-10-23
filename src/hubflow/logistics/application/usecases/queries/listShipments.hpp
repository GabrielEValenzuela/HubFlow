#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENTS_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENTS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentsUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de ListShipmentsUseCase.
class ListShipments final : public ListShipmentsUseCase
{
public:
    explicit ListShipments(const LogisticsModel& model);

private:
    const domain::ShipmentCatalog& execute(const shared::application::Principal& principal) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENTS_HPP
