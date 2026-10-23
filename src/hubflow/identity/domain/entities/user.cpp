#include "hubflow/identity/domain/entities/user.hpp"

#include "hubflow/identity/domain/events/userEvents.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <utility>

namespace hubflow::identity::domain
{
namespace
{
constexpr std::size_t MIN_NAME_LENGTH {2};
constexpr std::size_t MAX_NAME_LENGTH {80};
constexpr std::size_t MIN_PASSWORD_LENGTH {8};
constexpr std::size_t MAX_PASSWORD_LENGTH {72};
} // namespace

User::User(UserState state)
    : m_state {std::move(state)}
{
}

User* User::registerNew(const std::uint64_t                 id,
                        const std::string&                  email,
                        const std::string&                  name,
                        const Role                          role,
                        const std::string&                  plainPassword,
                        const PasswordHasher&               hasher,
                        const shared::domain::ActorContext& context)
{
    User* pUser = new User {
        UserState {id, Email {email}.value(), validName(name), role, hasher.hash(validPassword(plainPassword)), true}};
    pUser->raise(new UserRegistered {pUser->m_state, context});
    return pUser;
}

User* User::restore(const UserState& state)
{
    return new User {state};
}

bool User::authenticates(const std::string& plainPassword, const PasswordHasher& hasher) const
{
    return m_state.active && hasher.verify(plainPassword, m_state.passwordHash);
}

void User::rename(const std::string& name, const shared::domain::ActorContext& context)
{
    m_state.name = validName(name);
    announceChange(context);
}

void User::assignRole(const Role role, const shared::domain::ActorContext& context)
{
    m_state.role = role;
    announceChange(context);
}

void User::activate(const bool active, const shared::domain::ActorContext& context)
{
    if (!active && context.userId == m_state.id)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Conflict, "user.self_deactivation", "No podés desactivar tu propio usuario"};
    }
    m_state.active = active;
    announceChange(context);
}

void User::changePassword(const std::string&                  plainPassword,
                          const PasswordHasher&               hasher,
                          const shared::domain::ActorContext& context)
{
    m_state.passwordHash = hasher.hash(validPassword(plainPassword));
    announceChange(context);
}

std::string User::validName(const std::string& name)
{
    if (name.size() < MIN_NAME_LENGTH || name.size() > MAX_NAME_LENGTH)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation, "user.name_invalid", "El nombre debe tener entre 2 y 80 caracteres"};
    }
    return name;
}

std::string User::validPassword(const std::string& plainPassword)
{
    if (plainPassword.size() < MIN_PASSWORD_LENGTH || plainPassword.size() > MAX_PASSWORD_LENGTH)
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "user.password_invalid",
                                              "La contraseña debe tener entre 8 y 72 caracteres"};
    }
    return plainPassword;
}

void User::announceChange(const shared::domain::ActorContext& context)
{
    raise(new UserProfileChanged {m_state, context});
}

std::uint64_t User::id() const noexcept
{
    return m_state.id;
}

const std::string& User::email() const noexcept
{
    return m_state.email;
}

const std::string& User::name() const noexcept
{
    return m_state.name;
}

Role User::role() const noexcept
{
    return m_state.role;
}

bool User::active() const noexcept
{
    return m_state.active;
}

const UserState& User::state() const noexcept
{
    return m_state;
}

} // namespace hubflow::identity::domain
