#ifndef HUBFLOW_IDENTITY_DOMAIN_SERVICES_PASSWORD_HASHER_HPP
#define HUBFLOW_IDENTITY_DOMAIN_SERVICES_PASSWORD_HASHER_HPP

#include <string>

namespace hubflow::identity::domain
{

/**
 * @brief Servicio de hashing de contraseñas (su algoritmo concreto lo aporta un adaptador).
 *
 * Vive en el dominio porque el usuario necesita verificar y cambiar contraseñas sin conocer el algoritmo.
 */
class PasswordHasher
{
public:
    virtual ~PasswordHasher() = default;

    /// @brief Hash con sal aleatoria, apto para almacenar.
    virtual std::string hash(const std::string& plainPassword) const = 0;

    /// @brief true si @p plainPassword corresponde a @p storedHash.
    virtual bool verify(const std::string& plainPassword, const std::string& storedHash) const = 0;
};

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_SERVICES_PASSWORD_HASHER_HPP
