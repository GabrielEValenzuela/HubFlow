#ifndef HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_USER_CONTROLLER_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_USER_CONTROLLER_HPP

#include "hubflow/identity/application/ports/in/findUserUseCase.hpp"
#include "hubflow/identity/application/ports/in/listCouriersUseCase.hpp"
#include "hubflow/identity/application/ports/in/listUsersUseCase.hpp"
#include "hubflow/identity/application/ports/in/registerUserUseCase.hpp"
#include "hubflow/identity/application/ports/in/updateUserUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::identity::adapters
{

/// @brief Casos de uso de usuarios que expone el controlador.
struct UserUseCases
{
    application::RegisterUserUseCase&       registerUser;
    application::UpdateUserUseCase&         updateUser;
    const application::ListUsersUseCase&    listUsers;
    const application::ListCouriersUseCase& listCouriers;
    const application::FindUserUseCase&     findUser;
};

/**
 * @brief Administración de usuarios y consulta de repartidores.
 *
 * - @c GET /users, @c GET /users/:id: consultas (administrador).
 * - @c POST /users: alta, acepta @c Idempotency-Key; repetir el mismo email responde 409.
 * - @c PATCH /users/:id: modificación; repetirla deja el mismo estado.
 * - @c GET /couriers: repartidores activos disponibles para asignar rutas.
 */
class UserController final : public platform::Controller
{
public:
    explicit UserController(UserUseCases useCases);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    platform::HttpResponse update(const platform::RequestContext& context) const;

    UserUseCases m_useCases;
};

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_IN_HTTP_USER_CONTROLLER_HPP
