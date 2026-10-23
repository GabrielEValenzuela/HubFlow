#ifndef HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_GET_CODE_SET_USE_CASE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_GET_CODE_SET_USE_CASE_HPP

#include "hubflow/insights/domain/services/codeSet.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::insights::application
{

/// @brief Criterio de pertenencia pedido por el operador: una zona o un nivel de servicio, por su código.
struct SetSelector
{
    enum class Kind
    {
        Zone,
        Service
    };

    Kind        kind {Kind::Zone};
    std::string zone;
    std::string service;
};

/**
 * @brief Reúne los códigos de los envíos que pertenecen a una zona o a un nivel de servicio, sin repetidos.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede consultar los índices.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class GetCodeSetUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~GetCodeSetUseCase() = default;

    virtual domain::CodeSet execute(const shared::application::Principal& principal,
                                    const SetSelector&                    selector) const = 0;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_GET_CODE_SET_USE_CASE_HPP
