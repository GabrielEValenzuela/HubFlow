#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_AUTHENTICATE_USE_CASE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_AUTHENTICATE_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::identity::application
{

/**
 * @brief Identifica al usuario detrás de un token de sesión para que el resto de las operaciones sepan quién las pide.
 *
 * @par Quién puede ejecutarlo
 * Lo invoca el borde HTTP antes de cada operación autenticada; no requiere rol.
 *
 * @par Errores
 * - Unauthorized: el token es inválido, expiró o el usuario fue desactivado.
 *
 * @par Idempotencia
 * Sí. Es una lectura: el mismo token produce siempre la misma identidad mientras no expire.
 */
class AuthenticateUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~AuthenticateUseCase() = default;

    virtual shared::application::Principal execute(const std::string& token) const = 0;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_AUTHENTICATE_USE_CASE_HPP
