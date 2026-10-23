#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_INSIGHT_CONTROLLER_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_INSIGHT_CONTROLLER_HPP

#include "application/insights/insightQueries.hpp"
#include "infrastructure/http/controllers/controller.hpp"

namespace hubflow::infrastructure
{

/**
 * @brief Índices y consultas avanzadas (RF10–RF14).
 *
 * - @c GET /indexes/bst, @c GET /indexes/bst/entries/:code
 * - @c GET /indexes/bplus, @c GET /indexes/bplus/entries/:code, @c GET /indexes/bplus/entries?from=&to=
 * - @c GET /state-history-comparisons?first=&second=
 * - @c GET /shipment-sets?zone=|service=
 * - @c GET /shipment-set-operations?operation=&first=zone:NORTE&second=service:EXPRESS
 */
class InsightController final : public Controller
{
public:
    explicit InsightController(const application::InsightQueries& queries);

private:
    void registerRoutes(ApiRouter& router) override;

    HttpResponse setOperation(const RequestContext& context) const;

    const application::InsightQueries& m_queries;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_INSIGHT_CONTROLLER_HPP
