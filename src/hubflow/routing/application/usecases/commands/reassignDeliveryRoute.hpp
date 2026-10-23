#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_REASSIGN_DELIVERY_ROUTE_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_REASSIGN_DELIVERY_ROUTE_HPP

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/routing/application/ports/in/reassignDeliveryRouteUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::routing::application
{

/// @brief Implementación de ReassignDeliveryRouteUseCase.
class ReassignDeliveryRoute final : public ReassignDeliveryRouteUseCase
{
public:
    ReassignDeliveryRoute(RoutingModel&                                routing,
                          const identity::application::UserRepository& users,
                          shared::application::EventBus&               bus,
                          const shared::application::Clock&            clock);

private:
    const domain::DeliveryRoute&
    execute(const shared::application::Principal& principal, std::uint64_t routeId, std::uint64_t courierId) override;

    RoutingModel&                                m_routing;
    const identity::application::UserRepository& m_users;
    shared::application::EventBus&               m_bus;
    const shared::application::Clock&            m_clock;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_REASSIGN_DELIVERY_ROUTE_HPP
