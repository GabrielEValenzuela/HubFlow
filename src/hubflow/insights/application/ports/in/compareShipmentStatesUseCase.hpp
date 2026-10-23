#ifndef HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_COMPARE_SHIPMENT_STATES_USE_CASE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_COMPARE_SHIPMENT_STATES_USE_CASE_HPP

#include "hubflow/insights/domain/services/stateComparison.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::insights::application
{

/**
 * @brief Compara los estados por los que pasaron dos envíos: los que comparten, los de al menos uno y los exclusivos de
 * cada uno.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede consultar los índices.
 * - NotFound: alguno de los envíos no existe.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class CompareShipmentStatesUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~CompareShipmentStatesUseCase() = default;

    virtual domain::StateComparison execute(const shared::application::Principal& principal,
                                            const std::string&                    firstCode,
                                            const std::string&                    secondCode) const = 0;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_COMPARE_SHIPMENT_STATES_USE_CASE_HPP
