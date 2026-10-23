#ifndef HUBFLOW_TRACKING_DOMAIN_SERVICES_CUSTOMER_TIMELINE_HPP
#define HUBFLOW_TRACKING_DOMAIN_SERVICES_CUSTOMER_TIMELINE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/tracking/domain/valueobjects/milestone.hpp"

#include <string>

namespace hubflow::tracking::domain
{

/// @brief Un paso de la línea de tiempo: el hito, lo que se le dice al cliente y cuándo ocurrió.
struct TimelineEntry
{
    Milestone    milestone {Milestone::Received};
    std::string  text;
    std::int64_t occurredAt {0};
};

/**
 * @brief Línea de tiempo que ve el cliente: los hitos del paquete en orden cronológico, con las frases de
 * @c CustomerWording.
 *
 * Se arma a partir de hitos ya traducidos, por lo que no contiene notas internas, nombres de quien operó ni datos
 * personales. El último paso es la situación actual del paquete.
 */
class CustomerTimeline final
{
public:
    /// @brief Arma la línea de tiempo con los hitos en el orden recibido.
    static CustomerTimeline from(const ds::DynamicArray<MilestoneEvent>& events);

    std::size_t          size() const noexcept;
    bool                 empty() const noexcept;
    const TimelineEntry& entry(std::size_t index) const;

    /// @brief La situación actual: el último paso. @pre no está vacía.
    const TimelineEntry& current() const;

    /// @brief true si el último paso es la entrega.
    bool finished() const noexcept;

private:
    ds::DynamicArray<TimelineEntry> m_entries;
};

} // namespace hubflow::tracking::domain

#endif // HUBFLOW_TRACKING_DOMAIN_SERVICES_CUSTOMER_TIMELINE_HPP
