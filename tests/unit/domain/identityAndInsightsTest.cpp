#include "support/fixtures.hpp"
#include "support/inMemoryRepositories.hpp"

#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/insights/domain/services/codeSet.hpp"
#include "hubflow/insights/domain/services/stateComparison.hpp"
#include "hubflow/logistics/domain/services/ticketDispenser.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using namespace hubflow::shared::domain;

namespace
{
std::vector<std::string> items(const insights::domain::CodeSet& set)
{
    std::vector<std::string> values;
    set.forEach(
        [&values](const std::string& value)
        {
            values.push_back(value);
        });
    return values;
}
} // namespace

TEST(User, RegistersValidatesAndAuthenticates)
{
    const test::IdleCouriers                idleCouriers;
    const hubflow::test::PlainHasher        plain;
    const identity::domain::PasswordHasher& hasher = plain;
    identity::domain::User*                 pUser  = identity::domain::User::registerNew(4,
                                                                        "Ana@HubFlow.local",
                                                                        "Ana Torres",
                                                                        identity::domain::Role::Operador,
                                                                        "secreta123",
                                                                        hasher,
                                                                        hubflow::test::actor());

    EXPECT_EQ(pUser->email(), "ana@hubflow.local");
    EXPECT_TRUE(pUser->authenticates("secreta123", hasher));
    EXPECT_FALSE(pUser->authenticates("otra-clave", hasher));
    pUser->activate(false, idleCouriers, hubflow::test::actor());
    EXPECT_FALSE(pUser->authenticates("secreta123", hasher));
    EXPECT_EQ(pUser->pendingEvents(), 2U);
    delete pUser;

    EXPECT_THROW(
        identity::domain::User::registerNew(
            5, "mal-email", "Nombre", identity::domain::Role::Pum, "secreta123", hasher, hubflow::test::actor()),
        business_error);
    EXPECT_THROW(identity::domain::User::registerNew(
                     5, "a@b.co", "Nombre", identity::domain::Role::Pum, "corta", hasher, hubflow::test::actor()),
                 business_error);
}

TEST(User, CannotDeactivateItself)
{
    identity::domain::User* pUser = identity::domain::User::restore(
        identity::domain::UserState {1, "a@b.co", "Admin", identity::domain::Role::Admin, "", true});
    const test::IdleCouriers idleCouriers;
    EXPECT_THROW(pUser->activate(false, idleCouriers, hubflow::test::actor()), business_error);
    delete pUser;
}

TEST(AccessPolicy, GrantsComeFromData)
{
    using identity::domain::AccessPolicy;
    using identity::domain::Permission;
    using identity::domain::Role;
    AccessPolicy empty;
    EXPECT_FALSE(empty.allows(Role::Admin, Permission::ManageUsers));

    AccessPolicy policy;
    hubflow::test::grantDefaultPermissions(policy);
    EXPECT_TRUE(policy.allows(Role::Admin, Permission::ManageUsers));
    EXPECT_TRUE(policy.allows(Role::Admin, Permission::ManageCatalogs));
    EXPECT_FALSE(policy.allows(Role::Operador, Permission::ManageUsers));
    EXPECT_FALSE(policy.allows(Role::Operador, Permission::ManageCatalogs));
    EXPECT_TRUE(policy.allows(Role::Operador, Permission::AssignRoutes));
    EXPECT_TRUE(policy.allows(Role::Pum, Permission::ResolveStops));
    EXPECT_FALSE(policy.allows(Role::Pum, Permission::OperateShipments));
    EXPECT_THROW(policy.require(Role::Pum, Permission::ViewAudit), business_error);

    policy.grant(Role::Pum, Permission::ViewAudit);
    policy.grant(Role::Pum, Permission::ViewAudit);
    EXPECT_NO_THROW(policy.require(Role::Pum, Permission::ViewAudit));
    EXPECT_EQ(policy.grantCount(), 15U);
}

TEST(CodeSets, BuildsByZoneAndServiceAndCombines)
{
    logistics::domain::ShipmentCatalog catalog;
    logistics::domain::TicketDispenser tickets;
    for (const auto& draft : hubflow::test::initialDataset())
    {
        catalog.adopt(logistics::domain::Shipment::receive(draft, tickets.next(), hubflow::test::actor()));
    }
    const insights::domain::ZoneCriterion    north {"NORTE"};
    const insights::domain::ServiceCriterion express {"EXPRESS"};
    const insights::domain::CodeSet a = static_cast<const insights::domain::SetCriterion&>(north).collect(catalog);
    const insights::domain::CodeSet b = static_cast<const insights::domain::SetCriterion&>(express).collect(catalog);

    const insights::domain::UnionOperation        unite;
    const insights::domain::IntersectionOperation intersect;
    const insights::domain::DifferenceOperation   difference;
    EXPECT_EQ(items(a), (std::vector<std::string> {"PKG-1002", "PKG-1005", "PKG-1008"}));
    EXPECT_EQ(items(b), (std::vector<std::string> {"PKG-1002", "PKG-1006"}));
    EXPECT_EQ(items(static_cast<const insights::domain::SetOperation&>(intersect).apply(a, b)),
              (std::vector<std::string> {"PKG-1002"}));
    EXPECT_EQ(items(static_cast<const insights::domain::SetOperation&>(unite).apply(a, b)),
              (std::vector<std::string> {"PKG-1002", "PKG-1005", "PKG-1006", "PKG-1008"}));
    EXPECT_EQ(items(static_cast<const insights::domain::SetOperation&>(difference).apply(a, b)),
              (std::vector<std::string> {"PKG-1005", "PKG-1008"}));
}

TEST(StateComparison, UsesBitwiseOperations)
{
    logistics::domain::TicketDispenser tickets;
    logistics::domain::Shipment*       pFirst = logistics::domain::Shipment::receive(
        hubflow::test::draft("PKG-0001", "EXPRESS"), tickets.next(), hubflow::test::actor());
    logistics::domain::Shipment* pSecond = logistics::domain::Shipment::receive(
        hubflow::test::draft("PKG-0002", "EXPRESS"), tickets.next(), hubflow::test::actor());
    pFirst->dispatch("salida", hubflow::test::actor());
    pFirst->reschedule("ausente", tickets.next(), hubflow::test::actor());
    pSecond->classify("clasificado", hubflow::test::actor());

    const auto comparison = insights::domain::compareStates(*pFirst, *pSecond);
    EXPECT_EQ(comparison.common.raw(), 0b00001U);
    EXPECT_EQ(comparison.onlyFirst.raw(), 0b01100U);
    EXPECT_EQ(comparison.onlySecond.raw(), 0b00010U);
    EXPECT_EQ(comparison.either.count(), 4U);
    delete pFirst;
    delete pSecond;
}
