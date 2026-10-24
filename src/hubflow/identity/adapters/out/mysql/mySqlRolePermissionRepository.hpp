#ifndef HUBFLOW_IDENTITY_ADAPTERS_OUT_MYSQL_MY_SQL_ROLE_PERMISSION_REPOSITORY_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_OUT_MYSQL_MY_SQL_ROLE_PERMISSION_REPOSITORY_HPP

#include "hubflow/identity/application/ports/out/rolePermissionRepository.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"

namespace hubflow::identity::adapters
{

    /// @brief Qué puede hacer cada rol, guardado en la tabla @c role_permissions.
    class MySqlRolePermissionRepository final : public application::RolePermissionRepository
    {
    public:
        explicit MySqlRolePermissionRepository(platform::MySqlConnection& connection);

    private:
        void loadAll(application::GrantRestorer& restorer) const override;

        platform::MySqlConnection& m_connection;
    };

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_OUT_MYSQL_MY_SQL_ROLE_PERMISSION_REPOSITORY_HPP
