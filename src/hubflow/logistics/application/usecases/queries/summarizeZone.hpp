#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_SUMMARIZE_ZONE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_SUMMARIZE_ZONE_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/summarizeZoneUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de SummarizeZoneUseCase.
class SummarizeZone final : public SummarizeZoneUseCase
{
public:
    explicit SummarizeZone(const LogisticsModel& model);

private:
    domain::ZoneSummary execute(const shared::application::Principal& principal, domain::Zone zone) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_SUMMARIZE_ZONE_HPP
