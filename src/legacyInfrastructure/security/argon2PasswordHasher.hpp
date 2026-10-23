#ifndef HUBFLOW_INFRASTRUCTURE_SECURITY_ARGON2_PASSWORD_HASHER_HPP
#define HUBFLOW_INFRASTRUCTURE_SECURITY_ARGON2_PASSWORD_HASHER_HPP

#include "domain/identity/services/passwordHasher.hpp"

namespace hubflow::infrastructure
{

/// @brief Hash de contraseñas con Argon2id (libsodium, parámetros interactivos).
class Argon2PasswordHasher final : public domain::identity::PasswordHasher
{
public:
    /// @throws std::runtime_error si libsodium no puede inicializarse.
    Argon2PasswordHasher();

private:
    std::string hash(const std::string& plainPassword) const override;
    bool        verify(const std::string& plainPassword, const std::string& storedHash) const override;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_SECURITY_ARGON2_PASSWORD_HASHER_HPP
