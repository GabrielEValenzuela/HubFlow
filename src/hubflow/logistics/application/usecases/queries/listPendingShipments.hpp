#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_PENDING_SHIPMENTS_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_PENDING_SHIPMENTS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/listPendingShipmentsUseCase.hpp"

namespace hubflow::logistics::application
{

    /// @brief Implementación de ListPendingShipmentsUseCase.
    class ListPendingShipments final : public ListPendingShipmentsUseCase
    {
    public:
        explicit ListPendingShipments(const LogisticsModel& model);

    private:
        const domain::PendingShipments& execute(const shared::application::Principal& principal) const override;

        const LogisticsModel& m_model;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_PENDING_SHIPMENTS_HPP
