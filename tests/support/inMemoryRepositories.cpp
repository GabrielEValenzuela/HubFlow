#include "support/inMemoryRepositories.hpp"
#include <algorithm>

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
        const auto separator = token.find('|');
        if (separator != std::string::npos)
        {
            const std::int64_t expiresAt {std::stoll(token.substr(separator + 1))};
            if (expiresAt > now)
            {
                claims =
                    identity::application::TokenClaims {std::stoull(token.substr(0, separator)), "", {}, expiresAt};
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

    void InMemoryShipmentRepository::appendMovement(const std::string& code,
                                                    const logistics::domain::Movement& movement)
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

    void InMemoryShipmentGroupRepository::add(const logistics::domain::ShipmentGroupState& state)
    {
        groups.push_back(state);
    }

    void InMemoryShipmentGroupRepository::remove(const std::string& id)
    {
        for (auto groupIterator = groups.begin(); groupIterator != groups.end(); ++groupIterator)
        {
            if (groupIterator->id == id)
            {
                groups.erase(groupIterator);
                break;
            }
        }
    }

    void InMemoryShipmentGroupRepository::loadAll(logistics::application::ShipmentGroupRestorer& restorer) const
    {
        for (const auto& state : groups)
        {
            restorer.restoreGroup(state);
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

    void InMemoryRouteRepository::updateCourier(const std::uint64_t routeId, const routing::domain::Courier& courier)
    {
        records.at(routeId).courier = courier;
    }

    void FixtureRoadMapRepository::loadInto(routing::domain::RoadMap& map) const
    {
        buildReferenceCity(map, changed);
        for (const auto& entry : added)
        {
            const auto found = changed.find(entry.node.id);
            map.addNode(found == changed.end() ? entry.node : found->second);
            for (std::size_t linkIndex = 0; linkIndex < entry.links.size(); ++linkIndex)
            {
                map.connect(entry.node.id, entry.links[linkIndex].destinationId, entry.links[linkIndex].costKm);
            }
        }
    }

    void FixtureRoadMapRepository::addNode(const routing::domain::RoadNode& node,
                                           const ds::DynamicArray<routing::domain::RoadLink>& links)
    {
        added.push_back(AddedNode {node, links});
    }

    void FixtureRoadMapRepository::updateNode(const routing::domain::RoadNode& node)
    {
        changed[node.id] = node;
    }

    InMemoryCatalogRepository::InMemoryCatalogRepository()
        : zones {{"CENTRO", "Centro", true}, {"NORTE", "Norte", true}, {"SUR", "Sur", true}}
        , services {{"EXPRESS", "Express", 3, true},
                    {"PRIORITARIO", "Prioritario", 2, true},
                    {"ESTANDAR", "Estándar", 1, true}}
        , features {
              {"FRAGIL", "Frágil", true}, {"REFRIGERADO", "Refrigerado", true}, {"VOLUMINOSO", "Voluminoso", true}}
    {
    }

    void InMemoryCatalogRepository::saveZone(const logistics::domain::ZoneState& state)
    {
        const auto found = std::find_if(
            zones.begin(), zones.end(), [&state](const auto& stored) { return stored.code == state.code; });
        if (found != zones.end())
        {
            *found = state;
            return;
        }
        zones.push_back(state);
    }

    void InMemoryCatalogRepository::saveServiceLevel(const logistics::domain::ServiceLevelState& state)
    {
        const auto found = std::find_if(
            services.begin(), services.end(), [&state](const auto& stored) { return stored.code == state.code; });
        if (found != services.end())
        {
            *found = state;
            return;
        }
        services.push_back(state);
    }

    void InMemoryCatalogRepository::saveFeature(const logistics::domain::ShipmentFeatureState& state)
    {
        const auto found = std::find_if(
            features.begin(), features.end(), [&state](const auto& stored) { return stored.code == state.code; });
        if (found != features.end())
        {
            *found = state;
            return;
        }
        features.push_back(state);
    }

    void InMemoryCatalogRepository::loadAll(logistics::application::CatalogRestorer& restorer) const
    {
        for (const auto& state : zones)
        {
            restorer.restoreZone(state);
        }
        for (const auto& state : services)
        {
            restorer.restoreServiceLevel(state);
        }
        for (const auto& state : features)
        {
            restorer.restoreFeature(state);
        }
    }

    InMemoryRolePermissionRepository::InMemoryRolePermissionRepository()
    {
        identity::domain::AccessPolicy defaults;
        grantDefaultPermissions(defaults);
        for (const auto role :
             {identity::domain::Role::Admin, identity::domain::Role::Operador, identity::domain::Role::Pum})
        {
            for (int permission = 0; permission <= static_cast<int>(identity::domain::Permission::ManageCatalogs);
                 ++permission)
            {
                if (defaults.allows(role, static_cast<identity::domain::Permission>(permission)))
                {
                    grants.push_back({role, static_cast<identity::domain::Permission>(permission)});
                }
            }
        }
    }

    void InMemoryRolePermissionRepository::loadAll(identity::application::GrantRestorer& restorer) const
    {
        for (const auto& grant : grants)
        {
            restorer.restoreGrant(grant.role, grant.permission);
        }
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

    void InMemoryAuditLog::forEachRecent(const std::size_t limit,
                                         const std::size_t offset,
                                         audit::application::AuditEntryVisitor& visitor) const
    {
        for (std::size_t entryIndex = offset; entryIndex < entries.size() && entryIndex < offset + limit; ++entryIndex)
        {
            visitor.visit(entries[entries.size() - 1 - entryIndex]);
        }
    }

    std::size_t InMemoryAuditLog::count() const
    {
        return entries.size();
    }

    void InMemoryCustomerNoticeLog::append(const tracking::application::CustomerNotice& notice)
    {
        notices.push_back(notice);
    }

    void InMemoryCustomerNoticeLog::forEachRecent(const std::size_t limit,
                                                  const std::size_t offset,
                                                  tracking::application::CustomerNoticeVisitor& visitor) const
    {
        for (std::size_t i = offset; i < notices.size() && i < offset + limit; ++i)
        {
            visitor.visit(
                tracking::application::StoredCustomerNotice {notices.size() - i, notices[notices.size() - 1 - i]});
        }
    }

    std::size_t InMemoryCustomerNoticeLog::count() const
    {
        return notices.size();
    }

    std::string PlainTrackingTokenIssuer::issue(const tracking::application::TrackingGrant& grant) const
    {
        return grant.code + "|" + std::to_string(grant.expiresAt);
    }

    std::optional<tracking::application::TrackingGrant> PlainTrackingTokenIssuer::verify(const std::string& token,
                                                                                         const std::int64_t now) const
    {
        std::optional<tracking::application::TrackingGrant> grant;
        const auto separator = token.find('|');
        if (separator != std::string::npos &&
            token.find_first_not_of("0123456789", separator + 1) == std::string::npos && separator + 1 < token.size())
        {
            const std::int64_t expiresAt {std::stoll(token.substr(separator + 1))};
            if (expiresAt > now)
            {
                grant = tracking::application::TrackingGrant {token.substr(0, separator), expiresAt};
            }
        }
        return grant;
    }

    std::optional<shared::application::StoredResponse> InMemoryIdempotencyStore::find(const std::uint64_t userId,
                                                                                      const std::string& key) const
    {
        const auto found = responses.find({userId, key});
        return found == responses.end() ? std::nullopt
                                        : std::optional<shared::application::StoredResponse> {found->second};
    }

    void InMemoryIdempotencyStore::save(const std::uint64_t userId,
                                        const std::string& key,
                                        const shared::application::StoredResponse& response)
    {
        responses.emplace(std::make_pair(userId, key), response);
    }

} // namespace hubflow::test
