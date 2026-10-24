#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_COURIER_ROUTE_SHEET_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_COURIER_ROUTE_SHEET_USE_CASE_HPP

#include "hubflow/routing/application/ports/out/stopShipmentDirectory.hpp"
#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace hubflow::routing::application
{

    /// @brief Hoja de ruta lista para el repartidor: la ruta y, por cada parada, los datos de cada paquete que entrega
    /// (con sus bultos).
    struct CourierRouteSheet
    {
        const domain::DeliveryRoute& route;
        std::vector<std::vector<std::optional<StopShipmentDetails>>> stopDetails;
    };

    /**
     * @brief Devuelve la hoja de ruta con el destinatario, el servicio, el peso, la zona y los bultos de cada paquete
     * de cada parada.
     *
     * Es la única vía por la que un PUM conoce datos de los paquetes, y sólo de los de su propia ruta.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR para cualquier ruta; un PUM sólo las propias.
     *
     * @par Errores
     * - Forbidden: el rol no puede resolver paradas o la ruta pertenece a otro PUM.
     * - NotFound: la ruta no existe.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class GetCourierRouteSheetUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~GetCourierRouteSheetUseCase() = default;

        virtual CourierRouteSheet execute(const shared::application::Principal& principal,
                                          std::uint64_t routeId) const = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_COURIER_ROUTE_SHEET_USE_CASE_HPP
