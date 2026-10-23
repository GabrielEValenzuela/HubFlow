#ifndef HUBFLOW_AUDIT_ADAPTERS_IN_HTTP_AUDIT_JSON_PRESENTER_HPP
#define HUBFLOW_AUDIT_ADAPTERS_IN_HTTP_AUDIT_JSON_PRESENTER_HPP

#include "hubflow/audit/application/ports/out/auditLog.hpp"
#include "hubflow/platform/http/httpTypes.hpp"

namespace hubflow::audit::adapters
{

/// @brief Representación externa de las entradas de auditoría.
class AuditJsonPresenter final
{
public:
    /// @brief Una entrada: instante, autor, tipo de hecho, sujeto y detalle.
    static platform::Json entry(const application::AuditEntry& entry);
};

} // namespace hubflow::audit::adapters

#endif // HUBFLOW_AUDIT_ADAPTERS_IN_HTTP_AUDIT_JSON_PRESENTER_HPP
