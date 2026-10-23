#ifndef HUBFLOW_ROUTING_ADAPTERS_OUT_LOGISTICS_LOGISTICS_ZONE_DIRECTORY_HPP
#define HUBFLOW_ROUTING_ADAPTERS_OUT_LOGISTICS_LOGISTICS_ZONE_DIRECTORY_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/routing/application/ports/out/zoneDirectory.hpp"

namespace hubflow::routing::adapters
{

/// @brief Resuelve si una zona existe y está habilitada consultando el catálogo de zonas del contexto de logística.
class LogisticsZoneDirectory final : public application::ZoneDirectory
{
public:
    explicit LogisticsZoneDirectory(const logistics::application::LogisticsModel& model) noexcept;

private:
    bool isActive(const std::string& zoneCode) const override;

    const logistics::application::LogisticsModel& m_model;
};

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_OUT_LOGISTICS_LOGISTICS_ZONE_DIRECTORY_HPP
