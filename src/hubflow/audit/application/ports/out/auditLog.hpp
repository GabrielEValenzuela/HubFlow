#ifndef HUBFLOW_AUDIT_APPLICATION_PORTS_OUT_AUDIT_LOG_HPP
#define HUBFLOW_AUDIT_APPLICATION_PORTS_OUT_AUDIT_LOG_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace hubflow::audit::application
{

    /// @brief Registro de auditoría: quién hizo qué, sobre qué y cuándo.
    struct AuditEntry
    {
        std::uint64_t id {0};
        std::int64_t occurredAt {0};
        std::uint64_t actorId {0};
        std::string actorName;
        std::string type;
        std::string subject;
        std::string detail;
    };

    /// @brief Receptor de entradas de auditoría.
    class AuditEntryVisitor
    {
    public:
        virtual ~AuditEntryVisitor() = default;

        virtual void visit(const AuditEntry& entry) = 0;
    };

    /// @brief Puerto de almacenamiento de la auditoría.
    class AuditLog
    {
    public:
        virtual ~AuditLog() = default;

        virtual void append(const AuditEntry& entry) = 0;

        /// @brief Entradas más recientes primero.
        virtual void forEachRecent(std::size_t limit, std::size_t offset, AuditEntryVisitor& visitor) const = 0;

        virtual std::size_t count() const = 0;
    };

} // namespace hubflow::audit::application

#endif // HUBFLOW_AUDIT_APPLICATION_PORTS_OUT_AUDIT_LOG_HPP
