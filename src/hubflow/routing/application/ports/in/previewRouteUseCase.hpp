#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_IN_PREVIEW_ROUTE_USE_CASE_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_IN_PREVIEW_ROUTE_USE_CASE_HPP

#include "hubflow/routing/domain/services/search/searchTrace.hpp"
#include "hubflow/routing/domain/valueobjects/searchAlgorithm.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::routing::application
{

/**
 * @brief Simula el recorrido del PUM entre dos puntos del mapa con el algoritmo elegido, sin asignar ninguna ruta.
 *
 * @par Quién puede ejecutarlo
 * ADMIN, OPERADOR y PUM.
 *
 * @par Errores
 * - Forbidden: el rol no puede resolver paradas.
 * - Validation: origen o destino inexistentes.
 *
 * @par Idempotencia
 * Sí. Es una simulación de sólo lectura: el mismo pedido devuelve la misma traza.
 */
class PreviewRouteUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~PreviewRouteUseCase() = default;

    virtual domain::SearchTrace execute(const shared::application::Principal& principal,
                                        const std::string&                    originId,
                                        const std::string&                    destinationId,
                                        domain::SearchAlgorithm               algorithm) const = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_IN_PREVIEW_ROUTE_USE_CASE_HPP
