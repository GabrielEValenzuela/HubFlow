#include "hubflow/identity/adapters/out/mysql/mySqlRolePermissionRepository.hpp"

#include "hubflow/platform/mysql/sqlColumns.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::identity::adapters
{
    namespace
    {
        using platform::SqlColumns;
        using shared::application::Vocabulary;

        constexpr const char* SELECT_GRANTS {"SELECT role, permission FROM role_permissions ORDER BY role, permission"};
    } // namespace

    MySqlRolePermissionRepository::MySqlRolePermissionRepository(platform::MySqlConnection& connection)
        : m_connection {connection}
    {
    }

    void MySqlRolePermissionRepository::loadAll(application::GrantRestorer& restorer) const
    {
        for (const auto& row : m_connection.query(SELECT_GRANTS))
        {
            restorer.restoreGrant(SqlColumns::term(Vocabulary::role(SqlColumns::text(row, 0)), "role"),
                                  SqlColumns::term(Vocabulary::permission(SqlColumns::text(row, 1)), "permission"));
        }
    }

} // namespace hubflow::identity::adapters
