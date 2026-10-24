#include "hubflow/routing/application/usecases/queries/getCourierRouteSheet.hpp"

namespace hubflow::routing::application
{

    GetCourierRouteSheet::GetCourierRouteSheet(const GetRouteUseCase& getRoute, const StopShipmentDirectory& directory)
        : m_getRoute {getRoute}
        , m_directory {directory}
    {
    }

    CourierRouteSheet GetCourierRouteSheet::execute(const shared::application::Principal& principal,
                                                    const std::uint64_t routeId) const
    {
        const domain::DeliveryRoute& route = m_getRoute.execute(principal, routeId);
        CourierRouteSheet sheet {route, {}};
        for (std::size_t stopIndex = 0; stopIndex < route.stops().size(); ++stopIndex)
        {
            std::vector<std::optional<StopShipmentDetails>> details;
            const auto codes = route.stops()[stopIndex].shipmentCodes();
            details.reserve(codes.size());
            for (std::size_t codeIndex = 0; codeIndex < codes.size(); ++codeIndex)
            {
                details.push_back(m_directory.detailsOf(codes[codeIndex]));
            }
            sheet.stopDetails.push_back(details);
        }
        return sheet;
    }

} // namespace hubflow::routing::application
