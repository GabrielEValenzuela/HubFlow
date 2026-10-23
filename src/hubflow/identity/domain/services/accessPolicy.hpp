#ifndef HUBFLOW_IDENTITY_DOMAIN_SERVICES_ACCESS_POLICY_HPP
#define HUBFLOW_IDENTITY_DOMAIN_SERVICES_ACCESS_POLICY_HPP

#include "hubflow/identity/domain/valueobjects/role.hpp"

namespace hubflow::identity::domain
{

/**
 * @brief Política de acceso por rol.
 *
 * - ADMIN: todas las acciones.
 * - OPERADOR: envíos, mesa, asignación de rutas, resolución de paradas e índices.
 * - PUM: sólo resolver las paradas de sus propias rutas.
 */
class AccessPolicy final
{
public:
    static bool allows(Role role, Permission permission) noexcept;

    /// @throws business_error (Forbidden) si @p role no tiene @p permission.
    static void require(Role role, Permission permission);
};

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_SERVICES_ACCESS_POLICY_HPP
