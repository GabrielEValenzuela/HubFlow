#include "hubflow/logistics/adapters/out/mysql/mySqlShipmentGroupRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"

#include <map>
#include <string>
#include <vector>

namespace hubflow::logistics::adapters
{
    namespace
    {
        using platform::SqlColumns;

        constexpr const char* INSERT_GROUP {R"SQL(
    INSERT INTO shipment_groups (id, delivery_point) VALUES (?, ?)
)SQL"};

        constexpr const char* INSERT_MEMBER {R"SQL(
    INSERT INTO shipment_group_members (group_id, shipment_code, position) VALUES (?, ?, ?)
)SQL"};

        constexpr const char* DELETE_GROUP {R"SQL(
    DELETE FROM shipment_groups WHERE id = ?
)SQL"};

        constexpr const char* SELECT_GROUPS {R"SQL(
    SELECT id, delivery_point FROM shipment_groups ORDER BY created_at, id
)SQL"};

        constexpr const char* SELECT_MEMBERS {R"SQL(
    SELECT group_id, shipment_code FROM shipment_group_members ORDER BY group_id, position
)SQL"};
    } // namespace

    MySqlShipmentGroupRepository::MySqlShipmentGroupRepository(platform::MySqlConnection& connection)
        : m_connection {connection}
    {
    }

    void MySqlShipmentGroupRepository::add(const domain::ShipmentGroupState& state)
    {
        m_connection.execute(INSERT_GROUP, {state.id, state.deliveryPoint});
        for (std::size_t codeIndex = 0; codeIndex < state.codes.size(); ++codeIndex)
        {
            m_connection.execute(INSERT_MEMBER, {state.id, state.codes[codeIndex], std::uint64_t {codeIndex}});
        }
    }

    void MySqlShipmentGroupRepository::remove(const std::string& id)
    {
        m_connection.execute(DELETE_GROUP, {id});
    }

    void MySqlShipmentGroupRepository::loadAll(application::ShipmentGroupRestorer& restorer) const
    {
        std::map<std::string, std::vector<std::string>> membersByGroup;
        for (const auto& row : m_connection.query(SELECT_MEMBERS))
        {
            membersByGroup[SqlColumns::text(row, 0)].push_back(SqlColumns::text(row, 1));
        }
        for (const auto& row : m_connection.query(SELECT_GROUPS))
        {
            domain::ShipmentGroupState state;
            state.id = SqlColumns::text(row, 0);
            state.deliveryPoint = SqlColumns::text(row, 1);
            for (const auto& code : membersByGroup[state.id])
            {
                state.codes.pushBack(code);
            }
            restorer.restoreGroup(state);
        }
    }

} // namespace hubflow::logistics::adapters
