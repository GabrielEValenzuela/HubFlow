#ifndef HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_SHIPMENT_REPOSITORY_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_SHIPMENT_REPOSITORY_HPP

#include "hubflow/logistics/application/ports/out/shipmentRepository.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"

namespace hubflow::logistics::adapters
{

/// @brief Envíos, bultos y movimientos en las tablas @c shipments, @c parcels y @c movements.
class MySqlShipmentRepository final : public application::ShipmentRepository
{
public:
    explicit MySqlShipmentRepository(platform::MySqlConnection& connection);

private:
    void add(const domain::ShipmentState& state) override;
    void update(const domain::ShipmentState& state) override;
    void appendMovement(const std::string& code, const domain::Movement& movement) override;
    void loadAll(application::ShipmentRestorer& restorer) const override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_OUT_MYSQL_MY_SQL_SHIPMENT_REPOSITORY_HPP
