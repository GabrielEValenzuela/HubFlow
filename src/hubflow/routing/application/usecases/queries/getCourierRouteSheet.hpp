#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_COURIER_ROUTE_SHEET_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_COURIER_ROUTE_SHEET_HPP

#include "hubflow/routing/application/ports/in/getCourierRouteSheetUseCase.hpp"
#include "hubflow/routing/application/ports/in/getRouteUseCase.hpp"

namespace hubflow::routing::application
{

/// @brief Implementación de GetCourierRouteSheetUseCase: reutiliza el control de acceso de GetRouteUseCase.
class GetCourierRouteSheet final : public GetCourierRouteSheetUseCase
{
public:
    GetCourierRouteSheet(const GetRouteUseCase& getRoute, const StopShipmentDirectory& directory);

private:
    CourierRouteSheet execute(const shared::application::Principal& principal, std::uint64_t routeId) const override;

    const GetRouteUseCase&       m_getRoute;
    const StopShipmentDirectory& m_directory;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_QUERIES_GET_COURIER_ROUTE_SHEET_HPP
