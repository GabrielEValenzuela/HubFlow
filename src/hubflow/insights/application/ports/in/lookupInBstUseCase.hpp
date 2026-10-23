#ifndef HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_LOOKUP_IN_BST_USE_CASE_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_LOOKUP_IN_BST_USE_CASE_HPP

#include "datastructures/binarySearchTree.hpp"
#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::insights::application
{

/// @brief Un paso del recorrido de una búsqueda en el árbol binario.
struct BstVisit
{
    std::string code;
    ds::BstStep step {ds::BstStep::Found};
};

/// @brief Resultado de buscar en el árbol binario: envío (nullptr si no está), nodos visitados y camino.
struct BstLookup
{
    const logistics::domain::Shipment* pShipment {nullptr};
    std::size_t                        visited {0};
    ds::DynamicArray<BstVisit>         path;
};

/**
 * @brief Busca un envío en el árbol binario de búsqueda y devuelve el camino recorrido, nodo por nodo.
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
class LookupInBstUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~LookupInBstUseCase() = default;

    virtual BstLookup execute(const shared::application::Principal& principal, const std::string& code) const = 0;
};

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_PORTS_IN_LOOKUP_IN_BST_USE_CASE_HPP
