#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_ZONE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_ZONE_HPP

#include <cstddef>

namespace hubflow::logistics::domain
{

/// @brief Zona de entrega.
enum class Zone
{
    Centro,
    Norte,
    Sur
};

/// @brief Todas las zonas, en orden de presentación.
inline constexpr Zone ALL_ZONES[] {Zone::Centro, Zone::Norte, Zone::Sur};

inline constexpr std::size_t ZONE_COUNT {3};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_ZONE_HPP
