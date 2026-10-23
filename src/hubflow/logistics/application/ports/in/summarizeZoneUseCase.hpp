#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_SUMMARIZE_ZONE_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_SUMMARIZE_ZONE_USE_CASE_HPP

#include "hubflow/logistics/domain/services/zoneReport.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Resume los envíos pendientes de una zona (cantidad, peso total y detalle por prioridad) recorriendo la lista
 * de forma recursiva.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 * - NotFound: la zona no existe.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class SummarizeZoneUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~SummarizeZoneUseCase() = default;

    virtual domain::ZoneSummary execute(const shared::application::Principal& principal,
                                        const std::string&                    zoneCode) const = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_SUMMARIZE_ZONE_USE_CASE_HPP
