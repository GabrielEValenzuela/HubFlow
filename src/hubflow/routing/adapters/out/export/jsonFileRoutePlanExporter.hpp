#ifndef HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_JSON_FILE_ROUTE_PLAN_EXPORTER_HPP
#define HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_JSON_FILE_ROUTE_PLAN_EXPORTER_HPP

#include "hubflow/routing/application/ports/out/routePlanExporter.hpp"

#include <string>

namespace hubflow::routing::adapters
{

/**
 * @brief Escribe el plan de cada ruta en archivos JSON para el simulador.
 *
 * Crea la carpeta @c route-&lt;id&gt; dentro del directorio de exportación con @c initial_state.json y
 * @c sequence.json. Exportar de nuevo la misma ruta reemplaza los archivos.
 */
class JsonFileRoutePlanExporter final : public application::RoutePlanExporter
{
public:
    explicit JsonFileRoutePlanExporter(std::string exportDirectory);

private:
    /// @throws std::runtime_error si no se puede crear la carpeta o escribir un archivo.
    void exportPlan(const domain::RoadMap& map, const domain::DeliveryRoute& route) override;

    std::string m_exportDirectory;
};

} // namespace hubflow::routing::adapters

#endif // HUBFLOW_ROUTING_ADAPTERS_OUT_EXPORT_JSON_FILE_ROUTE_PLAN_EXPORTER_HPP
