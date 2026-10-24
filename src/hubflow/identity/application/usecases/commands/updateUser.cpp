#include "hubflow/identity/application/usecases/commands/updateUser.hpp"

#include "hubflow/identity/application/usecases/userHolder.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::application
{

    UpdateUser::UpdateUser(const UserRepository& users,
                           const domain::PasswordHasher& hasher,
                           const domain::CourierWorkload& workload,
                           shared::application::EventBus& bus,
                           const shared::application::Clock& clock)
        : m_users {users}
        , m_hasher {hasher}
        , m_workload {workload}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    domain::UserState UpdateUser::execute(const shared::application::Principal& principal,
                                          std::uint64_t userId,
                                          const UpdateUserRequest& request)
    {
        principal.require(identity::domain::Permission::ManageUsers);
        const auto state = m_users.byId(userId);
        if (!state.has_value())
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::NotFound, "user.not_found", "Usuario inexistente"};
        }
        const auto context = principal.contextAt(m_clock);
        const UserHolder holder {domain::User::restore(*state)};
        domain::User& user = holder.user();
        if (request.name.has_value())
        {
            user.rename(*request.name, context);
        }
        if (request.role.has_value())
        {
            user.assignRole(*request.role, m_workload, context);
        }
        if (request.active.has_value())
        {
            user.activate(*request.active, m_workload, context);
        }
        if (request.password.has_value())
        {
            user.changePassword(*request.password, m_hasher, context);
        }
        user.publishEvents(m_bus);
        return user.state();
    }

} // namespace hubflow::identity::application
