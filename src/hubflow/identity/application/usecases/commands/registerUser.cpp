#include "hubflow/identity/application/usecases/commands/registerUser.hpp"

#include "hubflow/identity/application/usecases/userHolder.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/identity/domain/valueobjects/email.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::application
{

    RegisterUser::RegisterUser(UserRepository& users,
                               const domain::PasswordHasher& hasher,
                               shared::application::EventBus& bus,
                               const shared::application::Clock& clock)
        : m_users {users}
        , m_hasher {hasher}
        , m_bus {bus}
        , m_clock {clock}
    {
    }

    domain::UserState RegisterUser::execute(const shared::application::Principal& principal,
                                            const RegisterUserRequest& request)
    {
        principal.require(identity::domain::Permission::ManageUsers);
        return create(request, principal.contextAt(m_clock));
    }

    domain::UserState RegisterUser::bootstrap(const RegisterUserRequest& request,
                                              const shared::domain::ActorContext& context)
    {
        return create(request, context);
    }

    domain::UserState RegisterUser::create(const RegisterUserRequest& request,
                                           const shared::domain::ActorContext& context)
    {
        if (m_users.byEmail(domain::Email {request.email}.value()).has_value())
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Conflict, "user.duplicate_email", "El email ya está registrado"};
        }
        const UserHolder holder {domain::User::registerNew(
            m_users.nextIdentity(), request.email, request.name, request.role, request.password, m_hasher, context)};
        holder.user().publishEvents(m_bus);
        return holder.user().state();
    }

} // namespace hubflow::identity::application
