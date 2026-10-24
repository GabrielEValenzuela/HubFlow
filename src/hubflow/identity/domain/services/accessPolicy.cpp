#include "hubflow/identity/domain/services/accessPolicy.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::domain
{

void AccessPolicy::grant(const Role role, const Permission permission)
{
    if (!allows(role, permission))
    {
        m_grants.pushBack(Grant {role, permission});
    }
}

bool AccessPolicy::allows(const Role role, const Permission permission) const
{
    bool found {false};
    for (std::size_t grantIndex = 0; grantIndex < m_grants.size() && !found; ++grantIndex)
    {
        found = m_grants[grantIndex].role == role && m_grants[grantIndex].permission == permission;
    }
    return found;
}

void AccessPolicy::require(const Role role, const Permission permission) const
{
    if (!allows(role, permission))
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Forbidden, "auth.forbidden", "No tenés permiso para esta operación"};
    }
}

std::size_t AccessPolicy::grantCount() const noexcept
{
    return m_grants.size();
}

} // namespace hubflow::identity::domain
