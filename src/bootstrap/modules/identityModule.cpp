#include "bootstrap/modules/identityModule.hpp"

#include "bootstrap/datasetSeeder.hpp"

#include "hubflow/identity/adapters/in/http/sessionController.hpp"
#include "hubflow/identity/adapters/in/http/userController.hpp"
#include "hubflow/identity/adapters/out/mysql/mySqlRolePermissionRepository.hpp"
#include "hubflow/identity/adapters/out/mysql/mySqlUserRepository.hpp"
#include "hubflow/identity/adapters/out/routing/routingCourierWorkload.hpp"
#include "hubflow/identity/application/hydration/accessPolicyHydrator.hpp"
#include "hubflow/identity/application/subscribers/userPersistenceSubscriber.hpp"
#include "hubflow/identity/application/usecases/commands/registerUser.hpp"
#include "hubflow/identity/application/usecases/commands/signIn.hpp"
#include "hubflow/identity/application/usecases/commands/updateUser.hpp"
#include "hubflow/identity/application/usecases/queries/authenticate.hpp"
#include "hubflow/identity/application/usecases/queries/findUser.hpp"
#include "hubflow/identity/application/usecases/queries/listCouriers.hpp"
#include "hubflow/identity/application/usecases/queries/listUsers.hpp"
#include "hubflow/platform/config/settings.hpp"
#include "hubflow/platform/http/apiRouter.hpp"

#include <vector>

namespace hubflow::bootstrap
{

    struct IdentityModule::Parts
    {
        Parts(const SharedServices& shared, const routing::application::RoutingModel& routing)
            : users {shared.connection}
            , rolePermissions {shared.connection}
            , courierWorkload {routing}
            , userPersistence {users}
            , signIn {users, shared.hasher, shared.tokens, shared.clock}
            , authenticate {users, shared.tokens, policy, shared.clock}
            , registerUser {users, shared.hasher, shared.bus, shared.clock}
            , updateUser {users, shared.hasher, courierWorkload, shared.bus, shared.clock}
            , listUsers {users}
            , listCouriers {users}
            , findUser {users}
            , seeder {registerUser, users, shared.clock, shared.settings.seedPassword}
        {
            subscribers.add(SubscriptionPhase::Persistence, userPersistence);
        }

        identity::adapters::MySqlUserRepository users;
        identity::adapters::MySqlRolePermissionRepository rolePermissions;
        identity::adapters::RoutingCourierWorkload courierWorkload;
        identity::domain::AccessPolicy policy;
        identity::application::UserPersistenceSubscriber userPersistence;
        identity::application::SignIn signIn;
        identity::application::Authenticate authenticate;
        identity::application::RegisterUser registerUser;
        identity::application::UpdateUser updateUser;
        identity::application::ListUsers listUsers;
        identity::application::ListCouriers listCouriers;
        identity::application::FindUser findUser;
        UserSeeder seeder;
        PhasedSubscribers subscribers;
        std::vector<std::unique_ptr<platform::Controller>> controllers;
    };

    IdentityModule::IdentityModule(const SharedServices& shared, const routing::application::RoutingModel& routing)
        : m_spParts {std::make_unique<Parts>(shared, routing)}
    {
    }

    IdentityModule::~IdentityModule() = default;

    const identity::application::UserRepository& IdentityModule::users() const noexcept
    {
        return m_spParts->users;
    }

    const identity::domain::AccessPolicy& IdentityModule::policy() const noexcept
    {
        return m_spParts->policy;
    }

    const identity::application::AuthenticateUseCase& IdentityModule::authenticator() const noexcept
    {
        return m_spParts->authenticate;
    }

    void IdentityModule::subscribe(const SubscriptionPhase phase, shared::application::EventBus& bus)
    {
        m_spParts->subscribers.subscribe(phase, bus);
    }

    void IdentityModule::hydrate()
    {
        identity::application::AccessPolicyHydrator {m_spParts->policy}.hydrate(m_spParts->rolePermissions);
    }

    void IdentityModule::seed()
    {
        m_spParts->seeder.seedUsers();
    }

    void IdentityModule::registerRoutes(platform::ApiRouter& router)
    {
        Parts& parts = *m_spParts;
        parts.controllers.push_back(
            std::make_unique<identity::adapters::SessionController>(parts.signIn, parts.findUser));
        parts.controllers.push_back(
            std::make_unique<identity::adapters::UserController>(identity::adapters::UserUseCases {
                parts.registerUser, parts.updateUser, parts.listUsers, parts.listCouriers, parts.findUser}));
        for (auto& spController : parts.controllers)
        {
            spController->registerRoutes(router);
        }
    }

} // namespace hubflow::bootstrap
