#include "hubflow/identity/domain/services/accessPolicy.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::domain
{

bool AccessPolicy::allows(const Role role, const Permission permission) noexcept
{
    const bool operatorAllowed {role == Role::Operador && permission != Permission::ManageUsers &&
                                permission != Permission::ViewAudit};
    const bool courierAllowed {role == Role::Pum && permission == Permission::ResolveStops};
    return role == Role::Admin || operatorAllowed || courierAllowed;
}

void AccessPolicy::require(const Role role, const Permission permission)
{
    if (!allows(role, permission))
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Forbidden, "auth.forbidden", "No tenés permiso para esta operación"};
    }
}

} // namespace hubflow::identity::domain
