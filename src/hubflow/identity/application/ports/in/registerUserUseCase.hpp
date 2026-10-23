#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_REGISTER_USER_USE_CASE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_REGISTER_USER_USE_CASE_HPP

#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::identity::application
{

/// @brief Datos de alta de un usuario.
struct RegisterUserRequest
{
    std::string  email;
    std::string  name;
    domain::Role role {domain::Role::Operador};
    std::string  password;
};

/**
 * @brief Da de alta un usuario del sistema (administrador, operador o repartidor PUM) con su contraseña inicial.
 *
 * @par Quién puede ejecutarlo
 * Sólo ADMIN.
 *
 * @par Errores
 * - Forbidden: el solicitante no es ADMIN.
 * - Conflict: ya existe un usuario con ese email.
 * - Validation: email, nombre o contraseña inválidos.
 *
 * @par Idempotencia
 * No. Crea un recurso nuevo. La clave natural es el email: repetir la petición falla con Conflict sin duplicar nada.
 * Con el encabezado Idempotency-Key el borde HTTP devuelve la respuesta original en lugar del error.
 */
class RegisterUserUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~RegisterUserUseCase() = default;

    virtual domain::UserState execute(const shared::application::Principal& principal,
                                      const RegisterUserRequest&            request) = 0;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_REGISTER_USER_USE_CASE_HPP
