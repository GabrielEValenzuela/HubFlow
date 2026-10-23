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
        std::vector<std::optional<StopShipmentDetails>> details;
        const auto                                      codes = route.stops()[i].shipmentCodes();
        for (std::size_t j = 0; j < codes.size(); ++j)
        {
            details.push_back(m_directory.detailsOf(codes[j]));
        }
        sheet.stopDetails.push_back(details);
    }
    return sheet;
}

} // namespace hubflow::routing::application
