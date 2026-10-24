#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_LIST_USERS_USE_CASE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_LIST_USERS_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::identity::application
{

    /**
     * @brief Lista todos los usuarios del sistema.
     *
     * @par Quién puede ejecutarlo
     * Sólo ADMIN.
     *
     * @par Errores
     * - Forbidden: el solicitante no es ADMIN.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class ListUsersUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ListUsersUseCase() = default;

        virtual ds::DynamicArray<domain::UserState> execute(const shared::application::Principal& principal) const = 0;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_LIST_USERS_USE_CASE_HPP
