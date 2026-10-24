#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_ROUTE_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_ROUTE_HPP

#include "hubflow/routing/application/ports/in/getRouteUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"

namespace hubflow::routing::application
{

    /// @brief Implementación de GetRouteUseCase.
    class GetRoute final : public GetRouteUseCase
    {
    public:
        explicit GetRoute(const RoutingModel& model);

    private:
        const domain::DeliveryRoute& execute(const shared::application::Principal& principal,
                                             std::uint64_t routeId) const override;

        const RoutingModel& m_model;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_ROUTE_HPP
