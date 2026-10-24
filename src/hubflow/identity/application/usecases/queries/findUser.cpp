#include "hubflow/identity/application/usecases/queries/findUser.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::application
{

    FindUser::FindUser(const UserRepository& users)
        : m_users {users}
    {
    }

    domain::UserState FindUser::execute(const shared::application::Principal& principal, std::uint64_t userId) const
    {
        if (principal.userId != userId)
        {
            principal.require(identity::domain::Permission::ManageUsers);
        }
        const auto state = m_users.byId(userId);
        if (!state.has_value())
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::NotFound, "user.not_found", "Usuario inexistente"};
        }
        return *state;
    }

} // namespace hubflow::identity::application
