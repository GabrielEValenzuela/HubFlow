#include "hubflow/logistics/adapters/out/mysql/mySqlCatalogRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"

namespace hubflow::logistics::adapters
{
    namespace
    {
        using platform::SqlColumns;

        constexpr const char* SAVE_ZONE {R"SQL(
    INSERT INTO zones (code, name, active) VALUES (?, ?, ?)
    ON DUPLICATE KEY UPDATE name = VALUES(name), active = VALUES(active)
)SQL"};

        constexpr const char* SAVE_SERVICE {R"SQL(
    INSERT INTO service_levels (code, name, priority, active) VALUES (?, ?, ?, ?)
    ON DUPLICATE KEY UPDATE name = VALUES(name), priority = VALUES(priority), active = VALUES(active)
)SQL"};

        constexpr const char* SAVE_FEATURE {R"SQL(
    INSERT INTO shipment_features (code, name, active) VALUES (?, ?, ?)
    ON DUPLICATE KEY UPDATE name = VALUES(name), active = VALUES(active)
)SQL"};

        constexpr const char* SELECT_ZONES {"SELECT code, name, active FROM zones ORDER BY position"};

        constexpr const char* SELECT_SERVICES {
            "SELECT code, name, priority, active FROM service_levels ORDER BY position"};

        constexpr const char* SELECT_FEATURES {"SELECT code, name, active FROM shipment_features ORDER BY position"};
    } // namespace

    MySqlCatalogRepository::MySqlCatalogRepository(platform::MySqlConnection& connection)
        : m_connection {connection}
    {
    }

    void MySqlCatalogRepository::saveZone(const domain::ZoneState& state)
    {
        m_connection.execute(SAVE_ZONE, {state.code, state.name, std::int64_t {state.active ? 1 : 0}});
    }

    void MySqlCatalogRepository::saveServiceLevel(const domain::ServiceLevelState& state)
    {
        m_connection.execute(
            SAVE_SERVICE,
            {state.code, state.name, std::uint64_t {state.priority}, std::int64_t {state.active ? 1 : 0}});
    }

    void MySqlCatalogRepository::saveFeature(const domain::ShipmentFeatureState& state)
    {
        m_connection.execute(SAVE_FEATURE, {state.code, state.name, std::int64_t {state.active ? 1 : 0}});
    }

    void MySqlCatalogRepository::loadAll(application::CatalogRestorer& restorer) const
    {
        for (const auto& row : m_connection.query(SELECT_ZONES))
        {
            restorer.restoreZone(
                domain::ZoneState {SqlColumns::text(row, 0), SqlColumns::text(row, 1), SqlColumns::flag(row, 2)});
        }
        for (const auto& row : m_connection.query(SELECT_SERVICES))
        {
            restorer.restoreServiceLevel(
                domain::ServiceLevelState {SqlColumns::text(row, 0),
                                           SqlColumns::text(row, 1),
                                           static_cast<unsigned>(SqlColumns::unsignedNumber(row, 2)),
                                           SqlColumns::flag(row, 3)});
        }
        for (const auto& row : m_connection.query(SELECT_FEATURES))
        {
            restorer.restoreFeature(domain::ShipmentFeatureState {
                SqlColumns::text(row, 0), SqlColumns::text(row, 1), SqlColumns::flag(row, 2)});
        }
    }

} // namespace hubflow::logistics::adapters
