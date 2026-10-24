#ifndef HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_RANGE_IN_BPLUS_TREE_USE_CASE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_RANGE_IN_BPLUS_TREE_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::insights::application
{

    /// @brief Resultado de una búsqueda por rango en el árbol B+.
    struct RangeResult
    {
        ds::DynamicArray<const logistics::domain::Shipment*> shipments;
        std::size_t visited {0};
    };

    /**
     * @brief Lista los envíos cuyo código está entre dos valores (inclusive), recorriendo las hojas enlazadas del árbol
     * B+.
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
    class RangeInBPlusTreeUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~RangeInBPlusTreeUseCase() = default;

        virtual RangeResult execute(const shared::application::Principal& principal,
                                    const std::string& from,
                                    const std::string& to) const = 0;
    };

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_RANGE_IN_BPLUS_TREE_USE_CASE_HPP
