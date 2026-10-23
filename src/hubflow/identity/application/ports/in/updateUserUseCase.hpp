#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_UPDATE_USER_USE_CASE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_UPDATE_USER_USE_CASE_HPP

#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace hubflow::identity::application
{

/// @brief Cambios parciales sobre un usuario: sólo se aplican los campos presentes.
struct UpdateUserRequest
{
    std::optional<std::string>  name;
    std::optional<domain::Role> role;
    std::optional<bool>         active;
    std::optional<std::string>  password;
};

/**
 * @brief Modifica nombre, rol, estado (activo o no) o contraseña de un usuario; sólo se aplican los datos presentes.
 *
 * @par Quién puede ejecutarlo
 * Sólo ADMIN.
 *
 * @par Errores
 * - Forbidden: el solicitante no es ADMIN.
 * - NotFound: el usuario no existe.
 * - Conflict: un administrador no puede desactivarse a sí mismo.
 * - Validation: datos inválidos.
 *
 * @par Idempotencia
 * Sí. Describe el estado final deseado de los campos enviados: aplicar la misma modificación dos veces deja al usuario
 * exactamente igual.
 */
class UpdateUserUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~UpdateUserUseCase() = default;

    virtual domain::UserState execute(const shared::application::Principal& principal,
                                      std::uint64_t                         userId,
                                      const UpdateUserRequest&              request) = 0;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_UPDATE_USER_USE_CASE_HPP
