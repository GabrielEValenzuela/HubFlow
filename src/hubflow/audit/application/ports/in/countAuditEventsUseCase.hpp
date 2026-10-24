#ifndef HUBFLOW_AUDIT_APPLICATION_PORTS_IN_COUNT_AUDIT_EVENTS_USE_CASE_HPP
#define HUBFLOW_AUDIT_APPLICATION_PORTS_IN_COUNT_AUDIT_EVENTS_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <cstddef>

namespace hubflow::audit::application
{

    /**
     * @brief Cuenta los registros de auditoría existentes (para paginar el listado).
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
    class CountAuditEventsUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~CountAuditEventsUseCase() = default;

        virtual std::size_t execute(const shared::application::Principal& principal) const = 0;
    };

} // namespace hubflow::audit::application

#endif // HUBFLOW_AUDIT_APPLICATION_PORTS_IN_COUNT_AUDIT_EVENTS_USE_CASE_HPP
