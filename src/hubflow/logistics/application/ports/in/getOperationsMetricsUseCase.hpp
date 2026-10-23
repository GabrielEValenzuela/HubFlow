#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_OPERATIONS_METRICS_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_OPERATIONS_METRICS_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/logistics/domain/services/zoneReport.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentStatus.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Indicadores operativos del centro de distribución.
struct OperationsMetrics
{
    /// @brief Resumen de pendientes de una zona del catálogo.
    struct ZoneEntry
    {
        std::string         code;
        std::string         name;
        bool                active {true};
        domain::ZoneSummary summary;
    };

    std::size_t                 totalShipments {0};
    std::size_t                 byStatus[domain::STATUS_COUNT] {};
    std::size_t                 totalAttempts {0};
    ds::DynamicArray<ZoneEntry> pendingByZone;
};

/**
 * @brief Calcula los indicadores operativos del centro: total de envíos, envíos por estado, intentos acumulados y
 * pendientes por cada zona del catálogo.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class GetOperationsMetricsUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~GetOperationsMetricsUseCase() = default;

    virtual OperationsMetrics execute(const shared::application::Principal& principal) const = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_OPERATIONS_METRICS_USE_CASE_HPP
