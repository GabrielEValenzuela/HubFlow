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

/// @brief Acciones del negocio sujetas a autorización.
enum class Permission
{
    ManageUsers,
    OperateShipments,
    OperateSortingDesk,
    AssignRoutes,
    ResolveStops,
    ViewIndexes,
    ViewAudit
};

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_ROLE_HPP
