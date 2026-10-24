#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_ASSIGN_DELIVERY_ROUTE_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_ASSIGN_DELIVERY_ROUTE_USE_CASE_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"
#include "hubflow/routing/domain/valueobjects/searchAlgorithm.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstdint>

namespace hubflow::routing::application
{

    /// @brief Pedido de asignación de ruta a partir del lote activo de la mesa.
    struct AssignRouteRequest
    {
        std::uint64_t batchId {0};
        std::uint64_t courierId {0};
        domain::SearchAlgorithm algorithm {domain::SearchAlgorithm::AStar};
    };

    /**
     * @brief Carga el vehículo de un PUM con los envíos procesados del lote, planifica el recorrido completo y despacha
     * todos los envíos. Los paquetes que viajan juntos forman una única parada.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede asignar rutas.
     * - Validation: el repartidor no es un PUM activo.
     * - NotFound: el lote no es el activo.
     * - Conflict: el lote no tiene envíos procesados, alguno ya no está pendiente o falta cargar algún compañero de un
     *   grupo.
     * - InvalidState: algún tramo no tiene camino en el mapa (no se modifica nada).
     *
     * @par Idempotencia
     * No. Crea una hoja de ruta nueva, despacha los envíos y cierra el lote. Repetirla falla porque el lote ya no
     * existe; con Idempotency-Key el borde HTTP devuelve la respuesta original.
     */
    class AssignDeliveryRouteUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

        virtual ~AssignDeliveryRouteUseCase() = default;

        virtual const domain::DeliveryRoute& execute(const shared::application::Principal& principal,
                                                     const AssignRouteRequest& request) = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_ASSIGN_DELIVERY_ROUTE_USE_CASE_HPP
