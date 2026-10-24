#ifndef HUBFLOW_IDENTITY_APPLICATION_HYDRATION_ACCESS_POLICY_HYDRATOR_HPP
#define HUBFLOW_IDENTITY_APPLICATION_HYDRATION_ACCESS_POLICY_HYDRATOR_HPP

#include "hubflow/identity/application/ports/out/rolePermissionRepository.hpp"
#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::identity::application
{

    /// @brief Carga en la política de acceso los permisos de cada rol guardados en el almacenamiento, al iniciar.
    class AccessPolicyHydrator final : private GrantRestorer
    {
    public:
        explicit AccessPolicyHydrator(domain::AccessPolicy& policy);

        void hydrate(const RolePermissionRepository& repository);

    private:
        void restoreGrant(domain::Role role, domain::Permission permission) override;

        domain::AccessPolicy& m_policy;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_HYDRATION_ACCESS_POLICY_HYDRATOR_HPP
