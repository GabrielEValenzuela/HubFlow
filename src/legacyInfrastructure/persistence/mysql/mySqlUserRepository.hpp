#ifndef HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_USER_REPOSITORY_HPP
#define HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_USER_REPOSITORY_HPP

#include "application/audit/auditTrail.hpp"
#include "domain/identity/repositories/userRepository.hpp"
#include "infrastructure/persistence/mysql/mySqlConnection.hpp"

namespace hubflow::infrastructure
{

/// @brief Usuarios en la tabla @c users.
class MySqlUserRepository final : public domain::identity::UserRepository
{
public:
    explicit MySqlUserRepository(MySqlConnection& connection);

private:
    std::uint64_t                              nextIdentity() override;
    void                                       add(const domain::identity::UserState& state) override;
    void                                       update(const domain::identity::UserState& state) override;
    std::optional<domain::identity::UserState> byId(std::uint64_t id) const override;
    std::optional<domain::identity::UserState> byEmail(const std::string& email) const override;
    void                                       forEach(domain::identity::UserVisitor& visitor) const override;
    std::size_t                                count() const override;

    MySqlConnection& m_connection;
};

/// @brief Auditoría en la tabla @c audit_events.
class MySqlAuditTrail final : public application::AuditTrail
{
public:
    explicit MySqlAuditTrail(MySqlConnection& connection);

private:
    void append(const application::AuditEntry& entry) override;
    void forEachRecent(std::size_t limit, std::size_t offset, application::AuditEntryVisitor& visitor) const override;
    std::size_t count() const override;

    MySqlConnection& m_connection;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_PERSISTENCE_MYSQL_USER_REPOSITORY_HPP
