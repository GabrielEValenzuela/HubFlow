#include "hubflow/logistics/adapters/out/mysql/mySqlShipmentRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

#include <map>
#include <string>
#include <vector>

namespace hubflow::logistics::adapters
{
namespace
{
using domain::Movement;
using domain::ShipmentState;
using platform::SqlColumns;
using shared::application::Vocabulary;

constexpr const char* INSERT_SHIPMENT {R"SQL(
    INSERT INTO shipments (code, recipient, zone, weight_kg, service, delivery_point, status, attempts, state_bits, queue_ticket)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
)SQL"};

constexpr const char* INSERT_FEATURE_LINK {R"SQL(
    INSERT INTO shipment_feature_links (shipment_code, feature_code) VALUES (?, ?)
)SQL"};

constexpr const char* SELECT_FEATURE_LINKS {R"SQL(
    SELECT shipment_code, feature_code FROM shipment_feature_links ORDER BY shipment_code, feature_code
)SQL"};

constexpr const char* UPDATE_SHIPMENT {R"SQL(
    UPDATE shipments SET status = ?, attempts = ?, state_bits = ?, queue_ticket = ? WHERE code = ?
)SQL"};

constexpr const char* INSERT_MOVEMENT {R"SQL(
    INSERT INTO movements (shipment_code, sequence, status, note, actor_name, occurred_at) VALUES (?, ?, ?, ?, ?, ?)
)SQL"};

constexpr const char* SELECT_SHIPMENTS {R"SQL(
    SELECT code, recipient, zone, weight_kg, service, delivery_point, status, attempts, state_bits, queue_ticket
    FROM shipments
)SQL"};

constexpr const char* SELECT_MOVEMENTS {R"SQL(
    SELECT shipment_code, sequence, status, note, actor_name, occurred_at FROM movements ORDER BY shipment_code, sequence
)SQL"};
} // namespace

MySqlShipmentRepository::MySqlShipmentRepository(platform::MySqlConnection& connection)
    : m_connection {connection}
{
}

void MySqlShipmentRepository::add(const ShipmentState& state)
{
    m_connection.execute(INSERT_SHIPMENT,
                         {state.draft.code,
                          state.draft.recipient,
                          state.draft.zone,
                          state.draft.weightKg,
                          state.draft.service,
                          state.draft.deliveryPoint,
                          Vocabulary::name(state.status),
                          std::uint64_t {state.attempts},
                          std::uint64_t {state.stateBits},
                          state.ticket});
    for (std::size_t i = 0; i < state.draft.features.size(); ++i)
    {
        m_connection.execute(INSERT_FEATURE_LINK, {state.draft.code, state.draft.features[i]});
    }
}

void MySqlShipmentRepository::update(const ShipmentState& state)
{
    m_connection.execute(UPDATE_SHIPMENT,
                         {Vocabulary::name(state.status),
                          std::uint64_t {state.attempts},
                          std::uint64_t {state.stateBits},
                          state.ticket,
                          state.draft.code});
}

void MySqlShipmentRepository::appendMovement(const std::string& code, const Movement& movement)
{
    m_connection.execute(INSERT_MOVEMENT,
                         {code,
                          std::uint64_t {movement.sequence},
                          Vocabulary::name(movement.status),
                          movement.note,
                          movement.actorName,
                          std::int64_t {movement.occurredAt}});
}

void MySqlShipmentRepository::loadAll(application::ShipmentRestorer& restorer) const
{
    std::map<std::string, std::vector<std::string>> featuresByShipment;
    for (const auto& row : m_connection.query(SELECT_FEATURE_LINKS))
    {
        featuresByShipment[SqlColumns::text(row, 0)].push_back(SqlColumns::text(row, 1));
    }
    for (const auto& row : m_connection.query(SELECT_SHIPMENTS))
    {
        ShipmentState state;
        state.draft.code          = SqlColumns::text(row, 0);
        state.draft.recipient     = SqlColumns::text(row, 1);
        state.draft.zone          = SqlColumns::text(row, 2);
        state.draft.weightKg      = SqlColumns::decimal(row, 3);
        state.draft.service       = SqlColumns::text(row, 4);
        state.draft.deliveryPoint = SqlColumns::text(row, 5);
        state.status              = SqlColumns::term(Vocabulary::shipmentStatus(SqlColumns::text(row, 6)), "status");
        state.attempts            = static_cast<std::uint32_t>(SqlColumns::unsignedNumber(row, 7));
        state.stateBits           = static_cast<std::uint32_t>(SqlColumns::unsignedNumber(row, 8));
        state.ticket              = SqlColumns::unsignedNumber(row, 9);
        for (const auto& feature : featuresByShipment[state.draft.code])
        {
            state.draft.features.pushBack(feature);
        }
        restorer.restoreShipment(state);
    }
    for (const auto& row : m_connection.query(SELECT_MOVEMENTS))
    {
        restorer.restoreMovement(
            SqlColumns::text(row, 0),
            Movement {static_cast<std::size_t>(SqlColumns::unsignedNumber(row, 1)),
                      SqlColumns::term(Vocabulary::shipmentStatus(SqlColumns::text(row, 2)), "status"),
                      SqlColumns::text(row, 3),
                      SqlColumns::text(row, 4),
                      SqlColumns::signedNumber(row, 5)});
    }
}

} // namespace hubflow::logistics::adapters
