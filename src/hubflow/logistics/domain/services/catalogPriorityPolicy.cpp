#include "hubflow/logistics/domain/services/catalogPriorityPolicy.hpp"

namespace hubflow::logistics::domain
{

    CatalogPriorityPolicy::CatalogPriorityPolicy(const ServiceLevelCatalog& services) noexcept
        : m_services {services}
    {
    }

    unsigned CatalogPriorityPolicy::priorityOf(const Shipment& shipment) const
    {
        const ServiceLevel* pService = m_services.find(shipment.service());
        return pService == nullptr ? 0 : pService->priority();
    }

    unsigned CatalogPriorityPolicy::highestPriority() const
    {
        unsigned top {0};
        m_services.forEach([&top](const ServiceLevel& service)
                           { top = service.priority() > top ? service.priority() : top; });
        return top;
    }

} // namespace hubflow::logistics::domain
