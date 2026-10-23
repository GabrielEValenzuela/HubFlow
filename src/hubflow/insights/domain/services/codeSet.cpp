#include "hubflow/insights/domain/services/codeSet.hpp"

namespace hubflow::insights::domain
{

CodeSet SetCriterion::collect(const logistics::domain::ShipmentCatalog& catalog) const
{
    CodeSet result;
    catalog.forEach(
        [this, &result](const logistics::domain::Shipment& shipment)
        {
            if (matches(shipment))
            {
                result.insert(shipment.code());
            }
        });
    return result;
}

ZoneCriterion::ZoneCriterion(const logistics::domain::Zone zone) noexcept
    : m_zone {zone}
{
}

bool ZoneCriterion::matches(const logistics::domain::Shipment& shipment) const
{
    return shipment.zone() == m_zone;
}

ServiceCriterion::ServiceCriterion(const logistics::domain::ServiceLevel service) noexcept
    : m_service {service}
{
}

bool ServiceCriterion::matches(const logistics::domain::Shipment& shipment) const
{
    return shipment.service() == m_service;
}

CodeSet UnionOperation::apply(const CodeSet& a, const CodeSet& b) const
{
    return a.unite(b);
}

CodeSet IntersectionOperation::apply(const CodeSet& a, const CodeSet& b) const
{
    return a.intersect(b);
}

CodeSet DifferenceOperation::apply(const CodeSet& a, const CodeSet& b) const
{
    return a.difference(b);
}

} // namespace hubflow::insights::domain
