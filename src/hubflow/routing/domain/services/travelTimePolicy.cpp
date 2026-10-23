#include "hubflow/routing/domain/services/travelTimePolicy.hpp"

#include <cmath>

namespace hubflow::routing::domain
{
namespace
{
constexpr double MINUTES_PER_HOUR {60.0};
} // namespace

std::uint32_t TravelTimePolicy::estimateMinutes(const double km, const std::size_t stops) noexcept
{
    const double driving = km / AVERAGE_KMH * MINUTES_PER_HOUR;
    const double serving = static_cast<double>(stops) * MINUTES_PER_STOP;
    return static_cast<std::uint32_t>(std::lround(driving + serving));
}

const char* TravelTimePolicy::assumptions() noexcept
{
    return "Estimación: 25 km/h promedio urbano + 4 min por parada.";
}

} // namespace hubflow::routing::domain
