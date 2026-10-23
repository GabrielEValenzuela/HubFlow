#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_CODE_SET_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_CODE_SET_HPP

#include "hubflow/insights/application/ports/in/getCodeSetUseCase.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"

namespace hubflow::insights::application
{

/// @brief Implementación de GetCodeSetUseCase.
class GetCodeSet final : public GetCodeSetUseCase
{
public:
    explicit GetCodeSet(const logistics::application::LogisticsModel& model);

private:
    domain::CodeSet execute(const shared::application::Principal& principal,
                            const SetSelector&                    selector) const override;

    const logistics::application::LogisticsModel& m_model;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_GET_CODE_SET_HPP
