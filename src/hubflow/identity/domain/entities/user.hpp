#ifndef HUBFLOW_IDENTITY_DOMAIN_ENTITIES_USER_HPP
#define HUBFLOW_IDENTITY_DOMAIN_ENTITIES_USER_HPP

#include "hubflow/identity/domain/services/courierWorkload.hpp"
#include "hubflow/identity/domain/services/passwordHasher.hpp"
#include "hubflow/identity/domain/valueobjects/email.hpp"
#include "hubflow/identity/domain/valueobjects/role.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <cstdint>
#include <string>

namespace hubflow::identity::domain
{

/// @brief Estado persistible de un usuario.
struct UserState
{
    std::uint64_t id {0};
    std::string   email;
    std::string   name;
    Role          role {Role::Operador};
    std::string   passwordHash;
    bool          active {true};
};

/**
 * @brief Agregado raíz Usuario.
 *
 * Valida sus datos, decide si unas credenciales lo autentican y registra UserRegistered /
 * UserProfileChanged cuando cambia.
 */
class User final : public shared::domain::AggregateRoot
{
public:
    /// @brief Fábrica de alta. Ownership del resultado al llamador.
    static User* registerNew(std::uint64_t                       id,
                             const std::string&                  email,
                             const std::string&                  name,
                             Role                                role,
                             const std::string&                  plainPassword,
                             const PasswordHasher&               hasher,
                             const shared::domain::ActorContext& context);

    /// @brief Reconstituye un usuario persistido. Ownership al llamador.
    static User* restore(const UserState& state);

    /// @brief true si está activo y la contraseña coincide.
    bool authenticates(const std::string& plainPassword, const PasswordHasher& hasher) const;

    void rename(const std::string& name, const shared::domain::ActorContext& context);

    /**
     * @brief Cambia el rol.
     * @throws business_error (Conflict) si es un chofer con un reparto en curso y el nuevo rol deja de ser chofer.
     */
    void assignRole(Role role, const CourierWorkload& workload, const shared::domain::ActorContext& context);

    /**
     * @brief Habilita o desactiva al usuario.
     * @throws business_error (Conflict) si se desactiva a sí mismo, o si es un chofer con un reparto en curso: primero
     * hay que pasar su ruta a otro chofer.
     */
    void activate(bool active, const CourierWorkload& workload, const shared::domain::ActorContext& context);
    void changePassword(const std::string&                  plainPassword,
                        const PasswordHasher&               hasher,
                        const shared::domain::ActorContext& context);

    std::uint64_t      id() const noexcept;
    const std::string& email() const noexcept;
    const std::string& name() const noexcept;
    Role               role() const noexcept;
    bool               active() const noexcept;
    const UserState&   state() const noexcept;

private:
    explicit User(UserState state);

    static std::string validName(const std::string& name);
    void               ensureNoRouteInProgress(const CourierWorkload& workload) const;
    static std::string validPassword(const std::string& plainPassword);

    void announceChange(const shared::domain::ActorContext& context);

    UserState m_state;
};

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_ENTITIES_USER_HPP
