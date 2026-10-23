#ifndef HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_LOOKUP_IN_BPLUS_TREE_USE_CASE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_LOOKUP_IN_BPLUS_TREE_USE_CASE_HPP

#include "hubflow/logistics/domain/services/shipmentLookup.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::insights::application
{

/**
 * @brief Busca un envío en el árbol B+ y devuelve cuántos nodos visitó.
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
class LookupInBPlusTreeUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~LookupInBPlusTreeUseCase() = default;

    virtual logistics::domain::LookupResult execute(const shared::application::Principal& principal,
                                                    const std::string&                    code) const = 0;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_LOOKUP_IN_BPLUS_TREE_USE_CASE_HPP
