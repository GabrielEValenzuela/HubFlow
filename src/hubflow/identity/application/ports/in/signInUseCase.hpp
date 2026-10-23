#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_SIGN_IN_USE_CASE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_SIGN_IN_USE_CASE_HPP

#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstdint>
#include <string>

namespace hubflow::identity::application
{

/// @brief Sesión abierta: token, vencimiento y usuario autenticado.
struct Session
{
    std::string       token;
    std::int64_t      expiresAt {0};
    domain::UserState user;
};

/**
 * @brief Abre una sesión con email y contraseña y entrega el token con el que el usuario opera después.
 *
 * @par Quién puede ejecutarlo
 * Cualquier persona que conozca sus credenciales. Es la única operación que no requiere un usuario autenticado.
 *
 * @par Errores
 * - Unauthorized: credenciales inválidas o usuario inactivo (el mensaje no revela cuál de los dos).
 *
 * @par Idempotencia
 * No. Cada ejecución emite un token nuevo con su propio vencimiento, así que repetirla no devuelve la misma respuesta.
 */
class SignInUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~SignInUseCase() = default;

    virtual Session execute(const std::string& email, const std::string& password) const = 0;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_SIGN_IN_USE_CASE_HPP
