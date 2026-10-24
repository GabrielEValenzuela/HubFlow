#ifndef HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_COMBINE_CODE_SETS_USE_CASE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_COMBINE_CODE_SETS_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/insights/application/ports/in/getCodeSetUseCase.hpp"
#include "hubflow/insights/domain/services/codeSet.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::insights::application
{

    /// @brief Operación entre conjuntos de códigos.
    enum class SetOperationKind
    {
        Union,
        Intersection,
        Difference
    };

    /// @brief Resultado de combinar dos conjuntos: ambos operandos, el resultado y sus envíos completos.
    struct SetCombination
    {
        domain::CodeSet first;
        domain::CodeSet second;
        domain::CodeSet result;
        ds::DynamicArray<const logistics::domain::Shipment*> shipments;
    };

    /**
     * @brief Combina dos conjuntos de códigos (unión, intersección o diferencia) y devuelve también los envíos
     * completos del resultado.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede consultar los índices.
     * - NotFound: algún código del resultado no corresponde a un envío conocido.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class CombineCodeSetsUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~CombineCodeSetsUseCase() = default;

        virtual SetCombination execute(const shared::application::Principal& principal,
                                       SetOperationKind operation,
                                       const SetSelector& first,
                                       const SetSelector& second) const = 0;
    };

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_COMBINE_CODE_SETS_USE_CASE_HPP
