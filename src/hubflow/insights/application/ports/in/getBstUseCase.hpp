#ifndef HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_GET_BST_USE_CASE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_GET_BST_USE_CASE_HPP

#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::insights::application
{

    /**
     * @brief Devuelve el árbol binario de búsqueda por código para inspeccionar su estructura.
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
    class GetBstUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~GetBstUseCase() = default;

        virtual const logistics::domain::ShipmentBst&
        execute(const shared::application::Principal& principal) const = 0;
    };

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_GET_BST_USE_CASE_HPP
