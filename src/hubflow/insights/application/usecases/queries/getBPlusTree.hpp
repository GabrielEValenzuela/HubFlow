#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_BPLUS_TREE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_BPLUS_TREE_HPP

#include "hubflow/insights/application/ports/in/getBPlusTreeUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

/// @brief Implementación de GetBPlusTreeUseCase.
class GetBPlusTree final : public GetBPlusTreeUseCase
{
public:
    explicit GetBPlusTree(const logistics::application::LogisticsModel& model);

private:
    const logistics::domain::ShipmentBPlusTree& execute(const shared::application::Principal& principal) const override;

    const logistics::application::LogisticsModel& m_model;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_BPLUS_TREE_HPP
