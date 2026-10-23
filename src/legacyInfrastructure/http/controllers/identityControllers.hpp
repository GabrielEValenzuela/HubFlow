#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_IDENTITY_CONTROLLERS_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_IDENTITY_CONTROLLERS_HPP

#include "application/audit/auditQueries.hpp"
#include "application/identity/userCommands.hpp"
#include "application/identity/userQueries.hpp"
#include "infrastructure/http/controllers/controller.hpp"

namespace hubflow::infrastructure
{

/**
 * @brief Sesiones.
 *
 * - @c POST /sessions — inicia sesión (público).
 * - @c GET /sessions/current — usuario de la sesión.
 */
class SessionController final : public Controller
{
public:
    SessionController(const application::SignIn& signIn, const application::UserQueries& users);

private:
    void registerRoutes(ApiRouter& router) override;

    const application::SignIn&      m_signIn;
    const application::UserQueries& m_users;
};

/**
 * @brief Usuarios (ADMIN) y repartidores.
 *
 * - @c GET /users, @c POST /users, @c GET /users/:id, @c PATCH /users/:id
 * - @c GET /couriers — PUM activos para asignar rutas.
 */
class UserController final : public Controller
{
public:
    UserController(application::RegisterUser&       registerUser,
                   application::UpdateUser&         updateUser,
                   const application::UserQueries& users);

private:
    void registerRoutes(ApiRouter& router) override;

    application::RegisterUser&       m_registerUser;
    application::UpdateUser&         m_updateUser;
    const application::UserQueries& m_users;
};

/// @brief Auditoría: @c GET /audit-events (ADMIN).
class AuditController final : public Controller
{
public:
    explicit AuditController(const application::AuditQueries& audit);

private:
    void registerRoutes(ApiRouter& router) override;

    const application::AuditQueries& m_audit;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_IDENTITY_CONTROLLERS_HPP
