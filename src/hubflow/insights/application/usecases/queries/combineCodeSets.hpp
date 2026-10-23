#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COMBINE_CODE_SETS_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COMBINE_CODE_SETS_HPP

#include "hubflow/insights/application/ports/in/combineCodeSetsUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

/// @brief Implementación de CombineCodeSetsUseCase.
class CombineCodeSets final : public CombineCodeSetsUseCase
{
public:
    explicit CombineCodeSets(const logistics::application::LogisticsModel& model);

private:
    SetCombination execute(const shared::application::Principal& principal,
                           SetOperationKind                      operation,
                           const SetSelector&                    first,
                           const SetSelector&                    second) const override;

    const logistics::application::LogisticsModel& m_model;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COMBINE_CODE_SETS_HPP
