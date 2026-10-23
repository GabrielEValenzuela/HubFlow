#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_CATALOG_PRIORITY_POLICY_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_CATALOG_PRIORITY_POLICY_HPP

#include "hubflow/logistics/domain/services/priorityPolicy.hpp"
#include "hubflow/logistics/domain/services/referenceCatalog.hpp"

namespace hubflow::logistics::domain
{

/**
 * @brief Criterio por defecto: la prioridad que el catálogo asigna al nivel de servicio del envío.
 *
 * Un servicio que el catálogo no conoce tiene prioridad 0, es decir, sale último. Como los niveles de servicio son
 * datos, agregar uno con mayor prioridad lo hace salir primero sin cambiar ninguna otra pieza.
 */
class CatalogPriorityPolicy final : public PriorityPolicy
{
public:
    explicit CatalogPriorityPolicy(const ServiceLevelCatalog& services) noexcept;

private:
    unsigned priorityOf(const Shipment& shipment) const override;
    unsigned highest() const override;

    const ServiceLevelCatalog& m_services;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_CATALOG_PRIORITY_POLICY_HPP
