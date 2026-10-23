#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_STOP_SHIPMENT_DIRECTORY_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_STOP_SHIPMENT_DIRECTORY_HPP

#include <optional>
#include <string>

namespace hubflow::routing::application
{

/// @brief Lo que el repartidor necesita saber del paquete de una parada para entregarlo.
struct StopShipmentDetails
{
    std::string recipient;
    std::string service;
    std::string zone;
    double      weightKg {0.0};
};

/**
 * @brief Consulta, sin exponer el catálogo de envíos, los datos de entrega del paquete de una parada.
 *
 * El contexto de ruteo sólo conoce los paquetes por su código; quien implemente este puerto resuelve
 * esos datos en el contexto de logística.
 */
class StopShipmentDirectory
{
public:
    virtual ~StopShipmentDirectory() = default;

    /// @brief Datos de entrega del paquete @p shipmentCode, o nada si no se conoce.
    virtual std::optional<StopShipmentDetails> detailsOf(const std::string& shipmentCode) const = 0;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_STOP_SHIPMENT_DIRECTORY_HPP
