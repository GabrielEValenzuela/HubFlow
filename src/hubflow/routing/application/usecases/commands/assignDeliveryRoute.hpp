#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_ASSIGN_DELIVERY_ROUTE_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_ASSIGN_DELIVERY_ROUTE_HPP

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/logistics/domain/services/shipmentGroupBook.hpp"
#include "hubflow/routing/application/ports/in/assignDeliveryRouteUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::routing::application
{

/// @brief Implementación de AssignDeliveryRouteUseCase.
class AssignDeliveryRoute final : public AssignDeliveryRouteUseCase
{
public:
    AssignDeliveryRoute(RoutingModel&                                routing,
                        sorting::domain::SortingDesk&                desk,
                        const logistics::domain::ShipmentGroupBook&  groups,
                        const identity::application::UserRepository& users,
                        shared::application::EventBus&               bus,
                        const shared::application::Clock&            clock);

private:
    const domain::DeliveryRoute& execute(const shared::application::Principal& principal,
                                         const AssignRouteRequest&             request) override;

    RoutingModel&                                m_routing;
    sorting::domain::SortingDesk&                m_desk;
    const logistics::domain::ShipmentGroupBook&  m_groups;
    const identity::application::UserRepository& m_users;
    shared::application::EventBus&               m_bus;
    const shared::application::Clock&            m_clock;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_ASSIGN_DELIVERY_ROUTE_HPP
