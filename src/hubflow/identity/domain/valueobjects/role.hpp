#ifndef HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_ROLE_HPP
#define HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_ROLE_HPP

#include <cstdint>

namespace hubflow::identity::domain
{

    /// @brief Rol del usuario en la operación.
    enum class Role : std::uint8_t
    {
        Admin,
        Operador,
        Pum
    };

    /// @brief Acciones del negocio sujetas a autorización. Qué rol puede cada una es un dato (ver AccessPolicy).
    enum class Permission : std::uint8_t
    {
        ManageUsers,
        OperateShipments,
        OperateSortingDesk,
        AssignRoutes,
        ResolveStops,
        ViewIndexes,
        ViewAudit,
        ManageCatalogs
    };

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_ROLE_HPP
