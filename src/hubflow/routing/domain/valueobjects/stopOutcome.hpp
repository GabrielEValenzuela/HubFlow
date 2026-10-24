#ifndef HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_STOP_OUTCOME_HPP
#define HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_STOP_OUTCOME_HPP

#include <cstdint>

namespace hubflow::routing::domain
{

    /// @brief Resultado de una parada de la hoja de ruta.
    enum class StopOutcome : std::uint8_t
    {
        Pending,
        Delivered,
        Rescheduled
    };

    /// @brief Estado de una hoja de ruta.
    enum class RouteStatus : std::uint8_t
    {
        InProgress,
        Completed
    };

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_STOP_OUTCOME_HPP
