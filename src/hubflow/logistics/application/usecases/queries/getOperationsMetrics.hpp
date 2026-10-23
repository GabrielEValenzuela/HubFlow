#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_OPERATIONS_METRICS_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_OPERATIONS_METRICS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/getOperationsMetricsUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de GetOperationsMetricsUseCase.
class GetOperationsMetrics final : public GetOperationsMetricsUseCase
{
public:
    explicit GetOperationsMetrics(const LogisticsModel& model);

private:
    OperationsMetrics execute(const shared::application::Principal& principal) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_GET_OPERATIONS_METRICS_HPP
