#include "hubflow/routing/adapters/out/logistics/logisticsZoneDirectory.hpp"

namespace hubflow::routing::adapters
{

LogisticsZoneDirectory::LogisticsZoneDirectory(const logistics::application::LogisticsModel& model) noexcept
    : m_model {model}
{
}

bool LogisticsZoneDirectory::isActive(const std::string& zoneCode) const
{
    const logistics::domain::Zone* pZone = m_model.zones.find(zoneCode);
    return pZone != nullptr && pZone->isActive();
}

} // namespace hubflow::routing::adapters
