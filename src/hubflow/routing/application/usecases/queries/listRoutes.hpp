#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_LIST_ROUTES_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_LIST_ROUTES_HPP

#include "hubflow/routing/application/ports/in/listRoutesUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"

namespace hubflow::routing::application
{

/// @brief Implementación de ListRoutesUseCase.
class ListRoutes final : public ListRoutesUseCase
{
public:
    explicit ListRoutes(const RoutingModel& model);

private:
    ds::DynamicArray<const domain::DeliveryRoute*>
    execute(const shared::application::Principal& principal) const override;

    const RoutingModel& m_model;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_LIST_ROUTES_HPP
