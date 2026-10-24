#ifndef HUBFLOW_IDENTITY_DOMAIN_SERVICES_ACCESS_POLICY_HPP
#define HUBFLOW_IDENTITY_DOMAIN_SERVICES_ACCESS_POLICY_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/identity/domain/valueobjects/role.hpp"

namespace hubflow::identity::domain
{

    /// @brief Un permiso concedido a un rol.
    struct Grant
    {
        Role role;
        Permission permission;
    };

    /**
     * @brief Política de acceso por rol, alimentada con datos.
     *
     * Qué puede hacer cada rol no está escrito en el programa: se carga al iniciar desde el almacenamiento (por
     * ejemplo, ADMIN todas las acciones, OPERADOR las de la planta y PUM sólo resolver paradas). Sin ninguna concesión
     * cargada nadie puede hacer nada.
     */
    class AccessPolicy final
    {
    public:
        AccessPolicy() = default;

        AccessPolicy(const AccessPolicy&) = delete;
        AccessPolicy& operator=(const AccessPolicy&) = delete;

        /// @brief Concede @p permission a @p role. Conceder dos veces lo mismo no cambia nada.
        void grant(Role role, Permission permission);

        bool allows(Role role, Permission permission) const;

        /// @throws business_error (Forbidden) si @p role no tiene @p permission.
        void require(Role role, Permission permission) const;

        std::size_t grantCount() const noexcept;

    private:
        ds::DynamicArray<Grant> m_grants;
    };

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_SERVICES_ACCESS_POLICY_HPP
