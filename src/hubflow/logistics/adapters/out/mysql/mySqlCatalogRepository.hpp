#ifndef HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_CATALOG_REPOSITORY_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_CATALOG_REPOSITORY_HPP

#include "hubflow/logistics/application/ports/out/catalogRepository.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"

namespace hubflow::logistics::adapters
{

/// @brief Zonas, niveles de servicio y características en las tablas @c zones, @c service_levels y @c
/// shipment_features.
class MySqlCatalogRepository final : public application::CatalogRepository
{
public:
    explicit MySqlCatalogRepository(platform::MySqlConnection& connection);

private:
    void saveZone(const domain::ZoneState& state) override;
    void saveServiceLevel(const domain::ServiceLevelState& state) override;
    void saveFeature(const domain::ShipmentFeatureState& state) override;
    void loadAll(application::CatalogRestorer& restorer) const override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_CATALOG_REPOSITORY_HPP
