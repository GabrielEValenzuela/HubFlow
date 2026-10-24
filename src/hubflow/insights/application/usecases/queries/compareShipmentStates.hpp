#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COMPARE_SHIPMENT_STATES_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COMPARE_SHIPMENT_STATES_HPP

#include "hubflow/insights/application/ports/in/compareShipmentStatesUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

    /// @brief Implementación de CompareShipmentStatesUseCase.
    class CompareShipmentStates final : public CompareShipmentStatesUseCase
    {
    public:
        explicit CompareShipmentStates(const logistics::application::LogisticsModel& model);

    private:
        domain::StateComparison execute(const shared::application::Principal& principal,
                                        const std::string& firstCode,
                                        const std::string& secondCode) const override;

        const logistics::application::LogisticsModel& m_model;
    };

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COMPARE_SHIPMENT_STATES_HPP
