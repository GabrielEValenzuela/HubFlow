#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_ROLE_PERMISSION_REPOSITORY_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_ROLE_PERMISSION_REPOSITORY_HPP

#include "hubflow/identity/domain/valueobjects/role.hpp"

namespace hubflow::identity::application
{

/// @brief Receptor de las concesiones de permisos durante la hidratación.
class GrantRestorer
{
public:
    virtual ~GrantRestorer() = default;

    virtual void restoreGrant(domain::Role role, domain::Permission permission) = 0;
};

/// @brief Fuente de la tabla que dice qué puede hacer cada rol.
class RolePermissionRepository
{
public:
    virtual ~RolePermissionRepository() = default;

    /// @brief Entrega todas las concesiones al @p restorer.
    virtual void loadAll(GrantRestorer& restorer) const = 0;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_ROLE_PERMISSION_REPOSITORY_HPP
