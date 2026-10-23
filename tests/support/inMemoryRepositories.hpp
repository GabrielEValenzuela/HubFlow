#ifndef HUBFLOW_TESTS_SUPPORT_IN_MEMORY_REPOSITORIES_HPP
#define HUBFLOW_TESTS_SUPPORT_IN_MEMORY_REPOSITORIES_HPP

#include "hubflow/audit/application/ports/out/auditLog.hpp"
#include "hubflow/identity/application/ports/out/rolePermissionRepository.hpp"
#include "hubflow/identity/application/ports/out/tokenIssuer.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/logistics/application/ports/out/catalogRepository.hpp"
#include "hubflow/logistics/application/ports/out/shipmentGroupRepository.hpp"
#include "hubflow/logistics/application/ports/out/shipmentRepository.hpp"
#include "hubflow/routing/application/ports/out/roadMapRepository.hpp"
#include "hubflow/routing/application/ports/out/routeRepository.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"
#include "hubflow/shared/application/ports/out/idempotencyStore.hpp"
#include "hubflow/tracking/application/ports/out/customerNoticeLog.hpp"
#include "hubflow/tracking/application/ports/out/trackingTokenIssuer.hpp"

#include <map>
#include <string>
#include <vector>

namespace hubflow::test
{

/// @brief Reloj controlable por el test.
class FixedClock final : public shared::application::Clock
{
public:
    explicit FixedClock(std::int64_t now = 1700000000);

    void advance(std::int64_t seconds);

private:
    std::int64_t now() const override;

    std::int64_t m_now;
};

/// @brief Hasher reversible para tests (nunca usar en producción).
class PlainHasher final : public identity::domain::PasswordHasher
{
private:
    std::string hash(const std::string& plainPassword) const override;
    bool        verify(const std::string& plainPassword, const std::string& storedHash) const override;
};

/// @brief Token de texto plano "id|expiresAt" para tests.
class PlainTokenIssuer final : public identity::application::TokenIssuer
{
private:
    std::string issue(const identity::application::TokenClaims& claims) const override;
    std::optional<identity::application::TokenClaims> verify(const std::string& token, std::int64_t now) const override;
};

/// @brief Repositorio de envíos en memoria.
class InMemoryShipmentRepository final : public logistics::application::ShipmentRepository
{
public:
    std::map<std::string, logistics::domain::ShipmentState>          states;
    std::vector<std::pair<std::string, logistics::domain::Movement>> movements;

private:
    void add(const logistics::domain::ShipmentState& state) override;
    void update(const logistics::domain::ShipmentState& state) override;
    void appendMovement(const std::string& code, const logistics::domain::Movement& movement) override;
    void loadAll(logistics::application::ShipmentRestorer& restorer) const override;
};

/// @brief Repositorio de grupos de envíos en memoria.
class InMemoryShipmentGroupRepository final : public logistics::application::ShipmentGroupRepository
{
public:
    std::vector<logistics::domain::ShipmentGroupState> groups;

private:
    void add(const logistics::domain::ShipmentGroupState& state) override;
    void remove(const std::string& id) override;
    void loadAll(logistics::application::ShipmentGroupRestorer& restorer) const override;
};

/// @brief Repositorio de rutas en memoria.
class InMemoryRouteRepository final : public routing::application::RouteRepository
{
public:
    std::map<std::uint64_t, routing::domain::RouteRecord> records;

private:
    void add(const routing::domain::RouteRecord& record) override;
    void updateStop(std::uint64_t routeId, const routing::domain::RouteStop& stop) override;
    void updateCourier(std::uint64_t routeId, const routing::domain::Courier& courier) override;
    void loadAll(routing::application::RouteRestorer& restorer) const override;
};

/// @brief Mapa de prueba: arranca con la ciudad de referencia y recuerda los puntos que se le agregan o cambian.
class FixtureRoadMapRepository final : public routing::application::RoadMapRepository
{
public:
    struct AddedNode
    {
        routing::domain::RoadNode                   node;
        ds::DynamicArray<routing::domain::RoadLink> links;
    };

    std::vector<AddedNode>                           added;
    std::map<std::string, routing::domain::RoadNode> changed;

private:
    void loadInto(routing::domain::RoadMap& map) const override;
    void addNode(const routing::domain::RoadNode&                   node,
                 const ds::DynamicArray<routing::domain::RoadLink>& links) override;
    void updateNode(const routing::domain::RoadNode& node) override;
};

/// @brief Zonas, servicios y características en memoria, arrancan con los datos de ejemplo del TP.
class InMemoryCatalogRepository final : public logistics::application::CatalogRepository
{
public:
    InMemoryCatalogRepository();

    std::vector<logistics::domain::ZoneState>            zones;
    std::vector<logistics::domain::ServiceLevelState>    services;
    std::vector<logistics::domain::ShipmentFeatureState> features;

private:
    void saveZone(const logistics::domain::ZoneState& state) override;
    void saveServiceLevel(const logistics::domain::ServiceLevelState& state) override;
    void saveFeature(const logistics::domain::ShipmentFeatureState& state) override;
    void loadAll(logistics::application::CatalogRestorer& restorer) const override;
};

/// @brief Permisos por rol en memoria, arrancan con la configuración por defecto.
class InMemoryRolePermissionRepository final : public identity::application::RolePermissionRepository
{
public:
    InMemoryRolePermissionRepository();

    std::vector<identity::domain::Grant> grants;

private:
    void loadAll(identity::application::GrantRestorer& restorer) const override;
};

/// @brief Repositorio de usuarios en memoria.
class InMemoryUserRepository final : public identity::application::UserRepository
{
public:
    std::map<std::uint64_t, identity::domain::UserState> users;

private:
    std::uint64_t                              nextIdentity() override;
    void                                       add(const identity::domain::UserState& state) override;
    void                                       update(const identity::domain::UserState& state) override;
    std::optional<identity::domain::UserState> byId(std::uint64_t id) const override;
    std::optional<identity::domain::UserState> byEmail(const std::string& email) const override;
    void                                       forEach(identity::application::UserVisitor& visitor) const override;
    std::size_t                                count() const override;
};

/// @brief Auditoría en memoria.
class InMemoryAuditLog final : public audit::application::AuditLog
{
public:
    std::vector<audit::application::AuditEntry> entries;

private:
    void append(const audit::application::AuditEntry& entry) override;
    void
    forEachRecent(std::size_t limit, std::size_t offset, audit::application::AuditEntryVisitor& visitor) const override;
    std::size_t count() const override;
};

/// @brief Avisos a clientes en memoria.
class InMemoryCustomerNoticeLog final : public tracking::application::CustomerNoticeLog
{
public:
    std::vector<tracking::application::CustomerNotice> notices;

private:
    void        append(const tracking::application::CustomerNotice& notice) override;
    void        forEachRecent(std::size_t                                   limit,
                              std::size_t                                   offset,
                              tracking::application::CustomerNoticeVisitor& visitor) const override;
    std::size_t count() const override;
};

/// @brief Constancia de seguimiento de texto plano "código|vencimiento" para tests (sin firma, nunca en producción).
class PlainTrackingTokenIssuer final : public tracking::application::TrackingTokenIssuer
{
private:
    std::string issue(const tracking::application::TrackingGrant& grant) const override;
    std::optional<tracking::application::TrackingGrant> verify(const std::string& token,
                                                               std::int64_t       now) const override;
};

/// @brief Memoria de claves de idempotencia en memoria.
class InMemoryIdempotencyStore final : public shared::application::IdempotencyStore
{
public:
    std::map<std::pair<std::uint64_t, std::string>, shared::application::StoredResponse> responses;

private:
    std::optional<shared::application::StoredResponse> find(std::uint64_t      userId,
                                                            const std::string& key) const override;
    void
    save(std::uint64_t userId, const std::string& key, const shared::application::StoredResponse& response) override;
};

} // namespace hubflow::test

#endif // HUBFLOW_TESTS_SUPPORT_IN_MEMORY_REPOSITORIES_HPP
