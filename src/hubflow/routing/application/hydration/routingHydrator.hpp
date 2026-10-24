#ifndef HUBFLOW_ROUTING_APPLICATION_HYDRATION_ROUTING_HYDRATOR_HPP
#define HUBFLOW_ROUTING_APPLICATION_HYDRATION_ROUTING_HYDRATOR_HPP

#include "hubflow/routing/application/ports/out/roadMapRepository.hpp"
#include "hubflow/routing/application/ports/out/routeRepository.hpp"
#include "hubflow/routing/application/routingModel.hpp"

namespace hubflow::routing::application
{

    /**
     * @brief Carga el mapa y reconstituye las hojas de ruta persistidas.
     *
     * Las trazas de búsqueda no se almacenan: el PUM es determinista, así que se recalculan.
     */
    class RoutingHydrator final : private RouteRestorer
    {
    public:
        explicit RoutingHydrator(RoutingModel& model);

        void hydrate(const RoadMapRepository& maps, const RouteRepository& routes);

    private:
        void restoreRoute(const domain::RouteRecord& record) override;

        RoutingModel& m_model;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_HYDRATION_ROUTING_HYDRATOR_HPP
