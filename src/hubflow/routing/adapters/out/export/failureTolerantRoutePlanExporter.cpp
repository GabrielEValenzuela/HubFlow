#include "hubflow/routing/adapters/out/export/failureTolerantRoutePlanExporter.hpp"

#include <exception>
#include <iostream>

namespace hubflow::routing::adapters
{

    FailureTolerantRoutePlanExporter::FailureTolerantRoutePlanExporter(application::RoutePlanExporter& inner)
        : m_inner {inner}
    {
    }

    void FailureTolerantRoutePlanExporter::exportPlan(const domain::RoadMap& map, const domain::DeliveryRoute& route)
    {
        try
        {
            m_inner.exportPlan(map, route);
        }
        catch (const std::exception& error)
        {
            std::cerr << "[hubflow] no se pudo exportar el plan de la ruta " << route.id() << ": " << error.what()
                      << '\n';
        }
    }

} // namespace hubflow::routing::adapters
