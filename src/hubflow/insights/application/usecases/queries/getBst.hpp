#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_BST_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_BST_HPP

#include "hubflow/insights/application/ports/in/getBstUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

    /// @brief Implementación de GetBstUseCase.
    class GetBst final : public GetBstUseCase
    {
    public:
        explicit GetBst(const logistics::application::LogisticsModel& model);

    private:
        const logistics::domain::ShipmentBst& execute(const shared::application::Principal& principal) const override;

        const logistics::application::LogisticsModel& m_model;
    };

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_BST_HPP
