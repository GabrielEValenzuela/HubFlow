#ifndef HUBFLOW_AUDIT_APPLICATION_PORTS_IN_LIST_AUDIT_EVENTS_USE_CASE_HPP
#define HUBFLOW_AUDIT_APPLICATION_PORTS_IN_LIST_AUDIT_EVENTS_USE_CASE_HPP

#include "hubflow/audit/application/ports/out/auditLog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstddef>

namespace hubflow::audit::application
{

    /**
     * @brief Lista los registros de auditoría (quién hizo qué, sobre qué y cuándo), del más reciente al más antiguo.
     *
     * @par Quién puede ejecutarlo
     * Sólo ADMIN.
     *
     * @par Errores
     * - Forbidden: el solicitante no es ADMIN.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class ListAuditEventsUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ListAuditEventsUseCase() = default;

        virtual void execute(const shared::application::Principal& principal,
                             std::size_t limit,
                             std::size_t offset,
                             AuditEntryVisitor& visitor) const = 0;
    };

} // namespace hubflow::audit::application

#endif // HUBFLOW_AUDIT_APPLICATION_PORTS_IN_LIST_AUDIT_EVENTS_USE_CASE_HPP
