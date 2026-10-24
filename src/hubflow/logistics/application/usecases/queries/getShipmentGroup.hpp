#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_SHIPMENT_GROUP_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_SHIPMENT_GROUP_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/getShipmentGroupUseCase.hpp"

namespace hubflow::logistics::application
{

    /// @brief Implementación de GetShipmentGroupUseCase.
    class GetShipmentGroup final : public GetShipmentGroupUseCase
    {
    public:
        explicit GetShipmentGroup(const LogisticsModel& model);

    private:
        const domain::ShipmentGroup& execute(const shared::application::Principal& principal,
                                             const std::string& groupId) const override;

        const LogisticsModel& m_model;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_SHIPMENT_GROUP_HPP
