#include "hubflow/insights/domain/services/codeSet.hpp"

#include <utility>

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

    ZoneCriterion::ZoneCriterion(std::string zone) noexcept
        : m_zone {std::move(zone)}
    {
    }

    bool ZoneCriterion::matches(const logistics::domain::Shipment& shipment) const
    {
        return shipment.zone() == m_zone;
    }

    ServiceCriterion::ServiceCriterion(std::string service) noexcept
        : m_service {std::move(service)}
    {
    }

    bool ServiceCriterion::matches(const logistics::domain::Shipment& shipment) const
    {
        return shipment.service() == m_service;
    }

    CodeSet UnionOperation::apply(const CodeSet& first, const CodeSet& second) const
    {
        return first.unite(second);
    }

    CodeSet IntersectionOperation::apply(const CodeSet& first, const CodeSet& second) const
    {
        return first.intersect(second);
    }

    CodeSet DifferenceOperation::apply(const CodeSet& first, const CodeSet& second) const
    {
        return first.difference(second);
    }

} // namespace hubflow::insights::domain
