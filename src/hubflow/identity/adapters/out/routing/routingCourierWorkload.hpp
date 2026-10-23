#ifndef HUBFLOW_IDENTITY_ADAPTERS_OUT_ROUTING_ROUTING_COURIER_WORKLOAD_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_OUT_ROUTING_ROUTING_COURIER_WORKLOAD_HPP

#include "hubflow/identity/domain/services/courierWorkload.hpp"
#include "hubflow/routing/application/routingModel.hpp"

namespace hubflow::identity::adapters
{

/// @brief Responde si un chofer tiene trabajo pendiente mirando las hojas de ruta del contexto de ruteo.
class RoutingCourierWorkload final : public domain::CourierWorkload
{
public:
    explicit RoutingCourierWorkload(const routing::application::RoutingModel& routing) noexcept;

private:
    bool hasRouteInProgress(std::uint64_t courierId) const override;

    const routing::application::RoutingModel& m_routing;
};

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_OUT_ROUTING_ROUTING_COURIER_WORKLOAD_HPP
