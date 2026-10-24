#include "hubflow/routing/application/usecases/queries/previewRoute.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/routing/domain/services/pumAgent.hpp"
#include "hubflow/routing/domain/services/search/searchStrategy.hpp"

namespace hubflow::routing::application
{

PreviewRoute::PreviewRoute(const RoutingModel& model)
    : m_model {model}
{
}

domain::SearchTrace PreviewRoute::execute(const shared::application::Principal& principal,
                                          const std::string&                    originId,
                                          const std::string&                    destinationId,
                                          domain::SearchAlgorithm               algorithm) const
{
    principal.require(identity::domain::Permission::ResolveStops);
    const domain::PumAgent agent {m_model.map, domain::SearchStrategy::of(algorithm)};
    return agent.travel(m_model.map.require(originId), m_model.map.require(destinationId));
}

} // namespace hubflow::routing::application
