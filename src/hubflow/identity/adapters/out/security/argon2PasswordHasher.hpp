#ifndef HUBFLOW_IDENTITY_ADAPTERS_OUT_SECURITY_ARGON2_PASSWORD_HASHER_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_OUT_SECURITY_ARGON2_PASSWORD_HASHER_HPP

#include "hubflow/identity/domain/services/passwordHasher.hpp"

namespace hubflow::identity::adapters
{

/// @brief Hash de contraseñas con Argon2id (libsodium, parámetros interactivos).
class Argon2PasswordHasher final : public domain::PasswordHasher
{
public:
    /// @throws std::runtime_error si libsodium no puede inicializarse.
    Argon2PasswordHasher();

private:
    std::string hash(const std::string& plainPassword) const override;
    bool        verify(const std::string& plainPassword, const std::string& storedHash) const override;
};

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_OUT_SECURITY_ARGON2_PASSWORD_HASHER_HPP
