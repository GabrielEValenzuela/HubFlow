#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_LOOKUP_IN_BPLUS_TREE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_LOOKUP_IN_BPLUS_TREE_HPP

#include "hubflow/insights/application/ports/in/lookupInBPlusTreeUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

    /// @brief Implementación de LookupInBPlusTreeUseCase.
    class LookupInBPlusTree final : public LookupInBPlusTreeUseCase
    {
    public:
        explicit LookupInBPlusTree(const logistics::application::LogisticsModel& model);

    private:
        logistics::domain::LookupResult execute(const shared::application::Principal& principal,
                                                const std::string& code) const override;

        const logistics::application::LogisticsModel& m_model;
    };

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_LOOKUP_IN_BPLUS_TREE_HPP
