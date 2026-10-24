#ifndef HUBFLOW_TRACKING_DOMAIN_VALUEOBJECTS_MILESTONE_HPP
#define HUBFLOW_TRACKING_DOMAIN_VALUEOBJECTS_MILESTONE_HPP

#include <cstdint>

namespace hubflow::tracking::domain
{

    /**
     * @brief Hito del recorrido de un paquete tal como lo entiende el cliente.
     *
     * La planta distingue más estados y matices que los que le importan a quien espera el paquete; cada movimiento
     * interno se traduce a uno de estos hitos.
     */
    enum class Milestone : std::uint8_t
    {
        Received,
        Prepared,
        OnTheWay,
        RetryScheduled,
        Delivered
    };

    /// @brief Un hito alcanzado y el instante (segundos desde epoch) en que ocurrió.
    struct MilestoneEvent
    {
        Milestone milestone {Milestone::Received};
        std::int64_t occurredAt {0};
    };

} // namespace hubflow::tracking::domain

#endif // HUBFLOW_TRACKING_DOMAIN_VALUEOBJECTS_MILESTONE_HPP
