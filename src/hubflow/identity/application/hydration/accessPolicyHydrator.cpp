#include "hubflow/identity/application/hydration/accessPolicyHydrator.hpp"

namespace hubflow::identity::application
{

AccessPolicyHydrator::AccessPolicyHydrator(domain::AccessPolicy& policy)
    : m_policy {policy}
{
}

void AccessPolicyHydrator::hydrate(const RolePermissionRepository& repository)
{
    repository.loadAll(*this);
}

void AccessPolicyHydrator::restoreGrant(const domain::Role role, const domain::Permission permission)
{
    m_policy.grant(role, permission);
}

} // namespace hubflow::identity::application
