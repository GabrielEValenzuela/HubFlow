#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_LOOKUP_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_LOOKUP_HPP

#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"

namespace hubflow::logistics::domain
{

/// @brief Resultado de buscar un envío por código.
struct LookupResult
{
    const Shipment* pShipment {nullptr};
    std::size_t     visited {0};
};

/**
 * @brief Estrategia de búsqueda por código sobre el catálogo (Strategy).
 *
 * Permite comparar, para el mismo código, el costo de recorrer la lista enlazada, el árbol
 * binario de búsqueda y el árbol B+.
 */
class ShipmentLookup
{
public:
    virtual ~ShipmentLookup() = default;

    /// @brief Busca @p code informando la cantidad de nodos visitados.
    virtual LookupResult find(const ShipmentCatalog& catalog, const std::string& code) const = 0;
};

/// @brief Búsqueda lineal recorriendo la lista enlazada del catálogo. Mejor O(1), peor O(n).
class LinearShipmentLookup final : public ShipmentLookup
{
private:
    LookupResult find(const ShipmentCatalog& catalog, const std::string& code) const override;
};

/// @brief Búsqueda en el BST. Mejor O(1), esperado O(log n), peor O(n) si el árbol degenera.
class BstShipmentLookup final : public ShipmentLookup
{
private:
    LookupResult find(const ShipmentCatalog& catalog, const std::string& code) const override;
};

/// @brief Búsqueda en el B+ de orden 4. O(log n): el árbol siempre está balanceado.
class BPlusShipmentLookup final : public ShipmentLookup
{
private:
    LookupResult find(const ShipmentCatalog& catalog, const std::string& code) const override;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_LOOKUP_HPP
