#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_RESOLVE_ROUTE_STOP_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_RESOLVE_ROUTE_STOP_USE_CASE_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace hubflow::routing::application
{

    /// @brief Resultado informado por el PUM para una parada.
    struct ResolveStopRequest
    {
        std::uint64_t routeId {0};
        std::size_t order {0};
        domain::StopOutcome outcome {domain::StopOutcome::Delivered};
        std::string note;
        bool confirmAllParcels {false};
    };

    /**
     * @brief El PUM informa el resultado de una parada de su ruta (entregado o reprogramado); se actualiza el envío y
     * la hoja de ruta.
     *
     * La parada se puede dar por entregada sólo si todos los bultos de sus paquetes fueron confirmados; con
     * @c confirmAllParcels el repartidor entrega todo en un solo paso y los bultos que faltan se confirman junto con la
     * parada. Si algunos bultos se entregaron y otros no, la parada se informa como reprogramada: los bultos entregados
     * quedan registrados y los demás vuelven a la planta con el paquete. Una parada con paquetes que viajan juntos se
     * resuelve completa.
     *
     * @par Quién puede ejecutarlo
     * ADMIN, OPERADOR y PUM; un PUM sólo puede resolver paradas de sus propias rutas.
     *
     * @par Errores
     * - Forbidden: el rol no puede resolver paradas o la ruta pertenece a otro PUM.
     * - NotFound: la ruta o la parada no existen.
     * - Validation: el resultado no es ENTREGADO ni REPROGRAMADO.
     * - Conflict: la parada ya tiene registrado un resultado distinto, o se pidió dar la parada por entregada con
     * bultos sin confirmar.
     *
     * @par Idempotencia
     * Sí. Informar el mismo resultado que ya está registrado no cambia nada, no emite eventos y responde igual que la
     * primera vez. Informar uno distinto sobre una parada ya resuelta es un Conflict.
     */
    class ResolveRouteStopUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ResolveRouteStopUseCase() = default;

        virtual const domain::DeliveryRoute& execute(const shared::application::Principal& principal,
                                                     const ResolveStopRequest& request) = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_RESOLVE_ROUTE_STOP_USE_CASE_HPP
