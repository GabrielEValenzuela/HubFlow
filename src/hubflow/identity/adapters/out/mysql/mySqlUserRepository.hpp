#ifndef HUBFLOW_IDENTITY_ADAPTERS_OUT_MYSQL_MY_SQL_USER_REPOSITORY_HPP
#define HUBFLOW_IDENTITY_ADAPTERS_OUT_MYSQL_MY_SQL_USER_REPOSITORY_HPP

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"

namespace hubflow::identity::adapters
{

/// @brief Usuarios en la tabla @c users.
class MySqlUserRepository final : public application::UserRepository
{
public:
    explicit MySqlUserRepository(platform::MySqlConnection& connection);

private:
    std::uint64_t                    nextIdentity() override;
    void                             add(const domain::UserState& state) override;
    void                             update(const domain::UserState& state) override;
    std::optional<domain::UserState> byId(std::uint64_t id) const override;
    std::optional<domain::UserState> byEmail(const std::string& email) const override;
    void                             forEach(application::UserVisitor& visitor) const override;
    std::size_t                      count() const override;

    platform::MySqlConnection& m_connection;
};

} // namespace hubflow::identity::adapters

#endif // HUBFLOW_IDENTITY_ADAPTERS_OUT_MYSQL_MY_SQL_USER_REPOSITORY_HPP
