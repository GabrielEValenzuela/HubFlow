#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_LOGISTICS_JSON_PRESENTER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_LOGISTICS_JSON_PRESENTER_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/application/ports/in/getOperationsMetricsUseCase.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/logistics/domain/services/zoneReport.hpp"
#include "hubflow/platform/http/httpTypes.hpp"

namespace hubflow::logistics::adapters
{

/**
 * @brief Representación externa de envíos, movimientos, historial de estados y métricas.
 *
 * Los demás contextos la reutilizan para mostrar envíos de forma consistente.
 */
class LogisticsJsonPresenter final
{
public:
    /// @brief Datos principales del envío.
    static platform::Json shipmentSummary(const domain::Shipment& shipment);

    /// @brief Datos principales más historial de estados y último movimiento.
    static platform::Json shipment(const domain::Shipment& shipment);

    /// @brief Un movimiento del historial.
    static platform::Json movement(const domain::Movement& movement);

    /// @brief Movimientos del envío, del más antiguo al más reciente o al revés.
    static platform::Json history(const domain::Shipment& shipment, bool newestFirst);

    /// @brief Estados por los que pasó el envío, como bits, valor numérico y nombres.
    static platform::Json stateHistory(const domain::StateHistory& history);

    /// @brief Cantidad, peso total y envíos urgentes (de la mayor prioridad) de una zona.
    static platform::Json zoneSummary(const std::string& zoneCode, const domain::ZoneSummary& summary);

    /// @brief Totales de la operación por estado y por zona.
    static platform::Json metrics(const application::OperationsMetrics& metrics);

    /// @brief Lista de resúmenes de envíos.
    static platform::Json shipments(const ds::DynamicArray<const domain::Shipment*>& shipments);
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_LOGISTICS_JSON_PRESENTER_HPP
