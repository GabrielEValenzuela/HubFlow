#include "hubflow/identity/adapters/out/security/argon2PasswordHasher.hpp"

#include <sodium.h>
#include <stdexcept>

namespace hubflow::identity::adapters
{

Argon2PasswordHasher::Argon2PasswordHasher()
{
    if (sodium_init() < 0)
    {
        throw std::runtime_error("No se pudo inicializar libsodium");
    }
}

std::string Argon2PasswordHasher::hash(const std::string& plainPassword) const
{
    char encoded[crypto_pwhash_STRBYTES] {};
    if (crypto_pwhash_str(encoded,
                          plainPassword.c_str(),
                          plainPassword.size(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        throw std::runtime_error("Memoria insuficiente para calcular el hash");
    }
    return std::string {encoded};
}

bool Argon2PasswordHasher::verify(const std::string& plainPassword, const std::string& storedHash) const
{
    return !storedHash.empty() &&
           crypto_pwhash_str_verify(storedHash.c_str(), plainPassword.c_str(), plainPassword.size()) == 0;
}

} // namespace hubflow::identity::adapters
