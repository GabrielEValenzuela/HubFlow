#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_RANGE_IN_BPLUS_TREE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_RANGE_IN_BPLUS_TREE_HPP

#include "hubflow/insights/application/ports/in/rangeInBPlusTreeUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

    /// @brief Implementación de RangeInBPlusTreeUseCase.
    class RangeInBPlusTree final : public RangeInBPlusTreeUseCase
    {
    public:
        explicit RangeInBPlusTree(const logistics::application::LogisticsModel& model);

    private:
        RangeResult execute(const shared::application::Principal& principal,
                            const std::string& from,
                            const std::string& to) const override;

        const logistics::application::LogisticsModel& m_model;
    };

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_RANGE_IN_BPLUS_TREE_HPP
