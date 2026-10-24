#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_PREVIEW_ROUTE_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_PREVIEW_ROUTE_HPP

#include "hubflow/routing/application/ports/in/previewRouteUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"

namespace hubflow::routing::application
{

/// @brief Implementación de PreviewRouteUseCase.
class PreviewRoute final : public PreviewRouteUseCase
{
public:
    explicit PreviewRoute(const RoutingModel& model);

private:
    domain::SearchTrace execute(const shared::application::Principal& principal,
                                const std::string&                    originId,
                                const std::string&                    destinationId,
                                domain::SearchAlgorithm               algorithm) const override;

    const RoutingModel& m_model;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_PREVIEW_ROUTE_HPP
