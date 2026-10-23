#ifndef HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_SHIPMENT_GROUP_REPOSITORY_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_SHIPMENT_GROUP_REPOSITORY_HPP

#include "hubflow/logistics/application/ports/out/shipmentGroupRepository.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"

namespace hubflow::logistics::adapters
{

/// @brief Grupos de envíos que viajan juntos en las tablas @c shipment_groups y @c shipment_group_members.
class MySqlShipmentGroupRepository final : public application::ShipmentGroupRepository
{
public:
    explicit MySqlShipmentGroupRepository(platform::MySqlConnection& connection);

private:
    void add(const domain::ShipmentGroupState& state) override;
    void remove(const std::string& id) override;
    void loadAll(application::ShipmentGroupRestorer& restorer) const override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_SHIPMENT_GROUP_REPOSITORY_HPP
