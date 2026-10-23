#ifndef HUBFLOW_INSIGHTS_DOMAIN_SERVICES_STATE_COMPARISON_HPP
#define HUBFLOW_INSIGHTS_DOMAIN_SERVICES_STATE_COMPARISON_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"

namespace hubflow::insights::domain
{

/// @brief Comparación de los estados históricos de dos envíos mediante operaciones bit a bit.
struct StateComparison
{
    logistics::domain::StateHistory common;
    logistics::domain::StateHistory onlyFirst;
    logistics::domain::StateHistory onlySecond;
    logistics::domain::StateHistory either;
    logistics::domain::StateHistory exclusive;
};

/**
 * @brief Compara los estados de dos envíos: common = A & B, either = A | B, exclusive = A ^ B,
 * onlyFirst = (A ^ B) & A, onlySecond = (A ^ B) & B. Todas O(1).
 */
StateComparison compareStates(const logistics::domain::Shipment& first,
                              const logistics::domain::Shipment& second) noexcept;

} // namespace hubflow::insights::domain

#endif // HUBFLOW_INSIGHTS_DOMAIN_SERVICES_STATE_COMPARISON_HPP
