#ifndef HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_IDENTITY_JSON_PRESENTER_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_IDENTITY_JSON_PRESENTER_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/identity/application/ports/in/signInUseCase.hpp"
#include "hubflow/platform/http/httpTypes.hpp"

namespace hubflow::identity::adapters
{

    /// @brief Representación externa de usuarios y sesiones. Nunca expone el hash de la contraseña.
    class IdentityJsonPresenter final
    {
    public:
        /// @brief Ficha pública de un usuario.
        static platform::Json user(const domain::UserState& user);

        /// @brief Token de sesión, vencimiento y usuario autenticado.
        static platform::Json session(const application::Session& session);

        /// @brief Lista de fichas de usuario.
        static platform::Json users(const ds::DynamicArray<domain::UserState>& userStates);
    };

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_IDENTITY_JSON_PRESENTER_HPP
