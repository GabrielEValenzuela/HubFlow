#include "support/applicationHarness.hpp"
#include "support/fixtures.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using shared::domain::business_error;

TEST(IdentityFlow, SignInAuthenticateAndPermissions)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    const auto               admin = app.bootstrapUser(identity::application::RegisterUserRequest {
        "admin@hubflow.local", "Admin", identity::domain::Role::Admin, "admin-secreto"});

    const auto session   = app.signIn.execute("ADMIN@hubflow.local", "admin-secreto");
    const auto principal = app.authenticate.execute(session.token);
    EXPECT_EQ(principal.userId, admin.id);
    EXPECT_EQ(principal.role, identity::domain::Role::Admin);
    EXPECT_THROW(app.signIn.execute("admin@hubflow.local", "incorrecta"), business_error);
    EXPECT_THROW(app.authenticate.execute("basura"), business_error);

    const auto operador =
        app.registerUser.execute(principal,
                                 identity::application::RegisterUserRequest {
                                     "op@hubflow.local", "Operador", identity::domain::Role::Operador, "operador123"});
    EXPECT_THROW(app.registerUser.execute(
                     shared::application::Principal {operador.id, "Operador", operador.role, &app.policy},
                     identity::application::RegisterUserRequest {"x@hubflow.local", "X", {}, "operador123"}),
                 business_error);

    identity::application::UpdateUserRequest deactivate;
    deactivate.active = false;
    app.updateUser.execute(principal, operador.id, deactivate);
    EXPECT_THROW(app.signIn.execute("op@hubflow.local", "operador123"), business_error);
    EXPECT_EQ(storage.users.users.size(), 2U);
    EXPECT_EQ(storage.audit.entries.back().type, "user.changed");

    app.clock.advance(identity::application::SignIn::SESSION_SECONDS + 1);
    EXPECT_THROW(app.authenticate.execute(session.token), business_error);
}

TEST(InsightsFlow, CombinesSetsAndResolvesShipmentsThroughBPlus)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    insights::application::SetSelector north;
    north.kind = insights::application::SetSelector::Kind::Zone;
    north.zone = "NORTE";
    insights::application::SetSelector express;
    express.kind    = insights::application::SetSelector::Kind::Service;
    express.service = "EXPRESS";

    const auto intersection = app.combineCodeSets.execute(
        app.operador(), insights::application::SetOperationKind::Intersection, north, express);
    ASSERT_EQ(intersection.shipments.size(), 1U);
    EXPECT_EQ(intersection.shipments[0]->recipient(), "Bruno Díaz");
    EXPECT_EQ(
        app.combineCodeSets.execute(app.operador(), insights::application::SetOperationKind::Union, north, express)
            .result.size(),
        4U);

    const auto range = app.rangeInBPlusTree.execute(app.operador(), "PKG-1003", "PKG-1006");
    ASSERT_EQ(range.shipments.size(), 4U);
    EXPECT_EQ(range.shipments[0]->code(), "PKG-1003");
    EXPECT_EQ(range.shipments[3]->code(), "PKG-1006");

    const auto lookup = app.lookupInBst.execute(app.operador(), "PKG-1005");
    EXPECT_EQ(lookup.visited, 5U);
    EXPECT_EQ(lookup.path.size(), 5U);
    EXPECT_THROW(app.lookupInBst.execute(app.pum(3), "PKG-1005"), business_error);
}

TEST(InsightsFlow, NewShipmentIsReachableThroughEveryStructure)
{
    test::Storage            storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    auto extra          = test::draft("PKG-1009", "EXPRESS", "NORTE", 2.0);
    extra.recipient     = "Inés Ríos";
    extra.deliveryPoint = "N4";
    app.registerShipment.execute(app.operador(), extra);
    EXPECT_NE(app.lookupInBPlusTree.execute(app.operador(), "PKG-1009").pShipment, nullptr);
    insights::application::SetSelector express;
    express.kind    = insights::application::SetSelector::Kind::Service;
    express.service = "EXPRESS";
    EXPECT_TRUE(app.getCodeSet.execute(app.operador(), express).contains("PKG-1009"));
    const auto& shipment = app.getShipment.execute(app.operador(), "PKG-1009");
    EXPECT_TRUE(shipment.stateHistory().test(0));
    EXPECT_EQ(shipment.stateHistory().count(), 1U);
}
