#include "hubflow/routing/application/usecases/queries/getCourierRouteSheet.hpp"

namespace hubflow::routing::application
{

GetCourierRouteSheet::GetCourierRouteSheet(const GetRouteUseCase& getRoute, const StopShipmentDirectory& directory)
    : m_getRoute {getRoute}
    , m_directory {directory}
{
}

CourierRouteSheet GetCourierRouteSheet::execute(const shared::application::Principal& principal,
                                                const std::uint64_t                   routeId) const
{
    const domain::DeliveryRoute& route = m_getRoute.execute(principal, routeId);
    CourierRouteSheet            sheet {route, {}};
    for (std::size_t i = 0; i < route.stops().size(); ++i)
    {
        sheet.stopDetails.push_back(m_directory.detailsOf(route.stops()[i].shipmentCode));
    }
    return sheet;
}

} // namespace hubflow::routing::application
