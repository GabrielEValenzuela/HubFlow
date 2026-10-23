#ifndef HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_ROLE_HPP
#define HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_ROLE_HPP

namespace hubflow::identity::domain
{

/// @brief Rol del usuario en la operación.
enum class Role
{
    Admin,
    Operador,
    Pum
};

/// @brief Acciones del negocio sujetas a autorización. Qué rol puede cada una es un dato (ver AccessPolicy).
enum class Permission
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
