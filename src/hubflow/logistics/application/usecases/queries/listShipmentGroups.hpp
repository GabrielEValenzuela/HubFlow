#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENT_GROUPS_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENT_GROUPS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentGroupsUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de ListShipmentGroupsUseCase.
class ListShipmentGroups final : public ListShipmentGroupsUseCase
{
public:
    explicit ListShipmentGroups(const LogisticsModel& model);

private:
    const domain::ShipmentGroupBook& execute(const shared::application::Principal& principal) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENT_GROUPS_HPP
