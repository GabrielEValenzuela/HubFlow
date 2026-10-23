#include "hubflow/identity/application/usecases/queries/listUsers.hpp"

#include "hubflow/identity/application/usecases/queries/userCollector.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::identity::application
{

ListUsers::ListUsers(const UserRepository& users)
    : m_users {users}
{
}

ds::DynamicArray<domain::UserState> ListUsers::execute(const shared::application::Principal& principal) const
{
    principal.require(identity::domain::Permission::ManageUsers);
    UserCollector collector {false};
    m_users.forEach(collector);
    return collector.users();
}

} // namespace hubflow::identity::application
