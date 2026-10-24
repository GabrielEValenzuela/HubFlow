#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_ROAD_MAP_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_ROAD_MAP_USE_CASE_HPP

#include "hubflow/routing/domain/entities/roadMap.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::routing::application
{

    /**
     * @brief Devuelve el mapa de la ciudad (centro de distribución, puntos de entrega y calles).
     *
     * @par Quién puede ejecutarlo
     * Cualquier usuario autenticado.
     *
     * @par Errores
     * - Ninguno propio de la operación.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class GetRoadMapUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~GetRoadMapUseCase() = default;

        virtual const domain::RoadMap& execute() const = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_GET_ROAD_MAP_USE_CASE_HPP
