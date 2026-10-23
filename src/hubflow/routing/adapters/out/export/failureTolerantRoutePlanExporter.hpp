#ifndef HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_FAILURE_TOLERANT_ROUTE_PLAN_EXPORTER_HPP
#define HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_FAILURE_TOLERANT_ROUTE_PLAN_EXPORTER_HPP

#include "hubflow/routing/application/ports/out/routePlanExporter.hpp"

namespace hubflow::routing::adapters
{

/**
 * @brief Entrega el plan de la ruta al exportador real, pero nunca deja que un fallo lo interrumpa.
 *
 * Asignar una ruta ya es un hecho consumado cuando se exporta: el lote se despachó, los envíos salieron
 * a reparto y la hoja de ruta quedó guardada. Exportar el plan es un servicio accesorio para el
 * simulador; si el disco está lleno o la carpeta no tiene permisos, fallar ahora dejaría al cliente
 * con un error pese a que la operación ocurrió y lo empujaría a reintentar. Por eso el fallo se
 * informa por el canal de errores del proceso y la operación continúa.
 */
class FailureTolerantRoutePlanExporter final : public application::RoutePlanExporter
{
public:
    /// @param inner exportador real; debe vivir al menos tanto como este objeto.
    explicit FailureTolerantRoutePlanExporter(application::RoutePlanExporter& inner);

private:
    void exportPlan(const domain::RoadMap& map, const domain::DeliveryRoute& route) override;

    application::RoutePlanExporter& m_inner;
};

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_FAILURE_TOLERANT_ROUTE_PLAN_EXPORTER_HPP
