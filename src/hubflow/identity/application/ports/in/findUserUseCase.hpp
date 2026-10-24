#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_FIND_USER_USE_CASE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_FIND_USER_USE_CASE_HPP

#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstdint>

namespace hubflow::identity::application
{

    /**
     * @brief Devuelve los datos de un usuario por su identificador.
     *
     * @par Quién puede ejecutarlo
     * ADMIN para cualquier usuario; el resto de los roles sólo puede consultarse a sí mismo.
     *
     * @par Errores
     * - Forbidden: consulta a otro usuario sin ser ADMIN.
     * - NotFound: el usuario no existe.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class FindUserUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~FindUserUseCase() = default;

        virtual domain::UserState execute(const shared::application::Principal& principal,
                                          std::uint64_t userId) const = 0;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_FIND_USER_USE_CASE_HPP
