#include "hubflow/identity/application/usecases/queries/listCouriers.hpp"

#include "hubflow/identity/application/usecases/queries/userCollector.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::identity::application
{

ListCouriers::ListCouriers(const UserRepository& users)
    : m_users {users}
{
}

ds::DynamicArray<domain::UserState> ListCouriers::execute(const shared::application::Principal& principal) const
{
    principal.require(identity::domain::Permission::AssignRoutes);
    UserCollector collector {true};
    m_users.forEach(collector);
    return collector.users();
}

} // namespace hubflow::identity::application
