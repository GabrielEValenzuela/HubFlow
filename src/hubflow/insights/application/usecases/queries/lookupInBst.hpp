#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_LOOKUP_IN_BST_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_LOOKUP_IN_BST_HPP

#include "hubflow/insights/application/ports/in/lookupInBstUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

/// @brief Implementación de LookupInBstUseCase.
class LookupInBst final : public LookupInBstUseCase
{
public:
    explicit LookupInBst(const logistics::application::LogisticsModel& model);

private:
    BstLookup execute(const shared::application::Principal& principal, const std::string& code) const override;

    const logistics::application::LogisticsModel& m_model;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_LOOKUP_IN_BST_HPP
