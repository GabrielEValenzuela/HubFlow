#ifndef HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_SHIPMENT_REPOSITORY_HPP
#define HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_SHIPMENT_REPOSITORY_HPP

#include "domain/logistics/repositories/shipmentRepository.hpp"
#include "infrastructure/persistence/mysql/mySqlConnection.hpp"

namespace hubflow::infrastructure
{

/// @brief Envíos y movimientos en las tablas @c shipments y @c movements.
class MySqlShipmentRepository final : public domain::logistics::ShipmentRepository
{
public:
    explicit MySqlShipmentRepository(MySqlConnection& connection);

private:
    void add(const domain::logistics::ShipmentState& state) override;
    void update(const domain::logistics::ShipmentState& state) override;
    void appendMovement(const std::string& code, const domain::logistics::Movement& movement) override;
    void loadAll(domain::logistics::ShipmentRestorer& restorer) const override;

    MySqlConnection& m_connection;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_SHIPMENT_REPOSITORY_HPP
