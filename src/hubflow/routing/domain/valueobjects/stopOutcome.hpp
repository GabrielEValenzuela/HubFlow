#ifndef HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_STOP_OUTCOME_HPP
#define HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_STOP_OUTCOME_HPP

namespace hubflow::routing::domain
{

/// @brief Resultado de una parada de la hoja de ruta.
enum class StopOutcome
{
    Pending,
    Delivered,
    Rescheduled
};

/// @brief Estado de una hoja de ruta.
enum class RouteStatus
{
    InProgress,
    Completed
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_VALUEOBJECTS_STOP_OUTCOME_HPP
