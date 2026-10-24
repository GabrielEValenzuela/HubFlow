#ifndef HUBFLOW_SORTING_DOMAIN_SERVICES_SHIPMENT_ORDERING_HPP
#define HUBFLOW_SORTING_DOMAIN_SERVICES_SHIPMENT_ORDERING_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/sorting/domain/valueobjects/sortCriteria.hpp"

namespace hubflow::sorting::domain
{

/// @brief Criterio de orden de un lote (Strategy).
class ShipmentOrdering
{
public:
    virtual ~ShipmentOrdering() = default;

    /// @brief true si @p candidate debe quedar antes que @p existing.
    virtual bool precedes(const logistics::domain::Shipment& candidate,
                          const logistics::domain::Shipment& existing) const = 0;

    /// @brief Criterio que representa la estrategia.
    virtual SortCriteria criteria() const noexcept = 0;

    /**
     * @brief Estrategia correspondiente a @p criteria.
     * @throws business_error (Validation) si @p criteria es None.
     */
    static const ShipmentOrdering& of(SortCriteria criteria);
};

/// @brief Código de seguimiento ascendente.
class CodeAscendingOrdering final : public ShipmentOrdering
{
private:
    bool         precedes(const logistics::domain::Shipment& candidate,
                          const logistics::domain::Shipment& existing) const override;
    SortCriteria criteria() const noexcept override;
};

/// @brief Peso descendente.
class WeightDescendingOrdering final : public ShipmentOrdering
{
private:
    bool         precedes(const logistics::domain::Shipment& candidate,
                          const logistics::domain::Shipment& existing) const override;
    SortCriteria criteria() const noexcept override;
};

} // namespace hubflow::sorting::domain

#endif // HUBFLOW_SORTING_DOMAIN_SERVICES_SHIPMENT_ORDERING_HPP
