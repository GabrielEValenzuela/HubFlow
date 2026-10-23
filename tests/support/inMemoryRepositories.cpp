#include "support/inMemoryRepositories.hpp"

#include "support/fixtures.hpp"

namespace hubflow::test
{

FixedClock::FixedClock(const std::int64_t now)
    : m_now {now}
{
}

void FixedClock::advance(const std::int64_t seconds)
{
    m_now += seconds;
}

std::int64_t FixedClock::now() const
{
    return m_now;
}

std::string PlainHasher::hash(const std::string& plainPassword) const
{
    return "plain:" + plainPassword;
}

bool PlainHasher::verify(const std::string& plainPassword, const std::string& storedHash) const
{
    return storedHash == "plain:" + plainPassword;
}

std::string PlainTokenIssuer::issue(const identity::application::TokenClaims& claims) const
{
    return std::to_string(claims.userId) + "|" + std::to_string(claims.expiresAt);
}

std::optional<identity::application::TokenClaims> PlainTokenIssuer::verify(const std::string& token,
                                                                           const std::int64_t now) const
{
    std::optional<identity::application::TokenClaims> claims;
    const auto                                        separator = token.find('|');
    if (separator != std::string::npos)
    {
        const std::int64_t expiresAt {std::stoll(token.substr(separator + 1))};
        if (expiresAt > now)
        {
            claims = identity::application::TokenClaims {std::stoull(token.substr(0, separator)), "", {}, expiresAt};
        }
    }
    return claims;
}

void InMemoryShipmentRepository::add(const logistics::domain::ShipmentState& state)
{
    states[state.draft.code] = state;
}

void InMemoryShipmentRepository::update(const logistics::domain::ShipmentState& state)
{
    states[state.draft.code] = state;
}

void InMemoryShipmentRepository::appendMovement(const std::string& code, const logistics::domain::Movement& movement)
{
    movements.emplace_back(code, movement);
}

void InMemoryShipmentRepository::loadAll(logistics::application::ShipmentRestorer& restorer) const
{
    for (const auto& [code, state] : states)
    {
        restorer.restoreShipment(state);
    }
    for (const auto& [code, movement] : movements)
    {
        restorer.restoreMovement(code, movement);
    }
}

void InMemoryRouteRepository::add(const routing::domain::RouteRecord& record)
{
    records[record.id] = record;
}

void InMemoryRouteRepository::updateStop(const std::uint64_t routeId, const routing::domain::RouteStop& stop)
{
    records.at(routeId).stops[stop.order - 1] = stop;
}

void InMemoryRouteRepository::loadAll(routing::application::RouteRestorer& restorer) const
{
    for (const auto& [id, record] : records)
    {
        restorer.restoreRoute(record);
    }
}

void FixtureRoadMapRepository::loadInto(routing::domain::RoadMap& map) const
{
    buildReferenceCity(map);
}

std::uint64_t InMemoryUserRepository::nextIdentity()
{
    return users.empty() ? 1 : users.rbegin()->first + 1;
}

void InMemoryUserRepository::add(const identity::domain::UserState& state)
{
    users[state.id] = state;
}

void InMemoryUserRepository::update(const identity::domain::UserState& state)
{
    users[state.id] = state;
}

std::optional<identity::domain::UserState> InMemoryUserRepository::byId(const std::uint64_t id) const
{
    const auto found = users.find(id);
    return found == users.end() ? std::nullopt : std::optional<identity::domain::UserState> {found->second};
}

std::optional<identity::domain::UserState> InMemoryUserRepository::byEmail(const std::string& email) const
{
    std::optional<identity::domain::UserState> result;
    for (const auto& [id, state] : users)
    {
        if (state.email == email)
        {
            result = state;
        }
    }
    return result;
}

void InMemoryUserRepository::forEach(identity::application::UserVisitor& visitor) const
{
    for (const auto& [id, state] : users)
    {
        visitor.visit(state);
    }
}

std::size_t InMemoryUserRepository::count() const
{
    return users.size();
}

void InMemoryAuditLog::append(const audit::application::AuditEntry& entry)
{
    entries.push_back(entry);
    entries.back().id = entries.size();
}

void InMemoryAuditLog::forEachRecent(const std::size_t                      limit,
                                     const std::size_t                      offset,
                                     audit::application::AuditEntryVisitor& visitor) const
{
    for (std::size_t i = offset; i < entries.size() && i < offset + limit; ++i)
    {
        visitor.visit(entries[entries.size() - 1 - i]);
    }
}

std::size_t InMemoryAuditLog::count() const
{
    return entries.size();
}

} // namespace hubflow::test
