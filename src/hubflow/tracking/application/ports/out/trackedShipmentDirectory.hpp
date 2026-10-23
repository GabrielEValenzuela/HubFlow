#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_TRACKED_SHIPMENT_DIRECTORY_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_TRACKED_SHIPMENT_DIRECTORY_HPP

#include "hubflow/tracking/domain/valueobjects/milestone.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace hubflow::tracking::application
{

/**
 * @brief Lo que el seguimiento necesita saber de un paquete, ya traducido al lenguaje del cliente.
 *
 * @c phoneLast4 y @c postalCode son los datos con los que se confirma la identidad de quien consulta: sirven para
 * comparar y nunca se muestran. Las notas internas y el nombre de quien operó el paquete no forman parte de esta
 * vista a propósito.
 */
struct TrackedShipment
{
    std::string                         code;
    std::vector<domain::MilestoneEvent> milestones;
    std::string                         phoneLast4;
    std::string                         postalCode;
    std::string                         recipientFirstName;
    std::string                         zoneName;
    std::string                         neighbourhood;
    std::size_t                         parcelCount {0};
    std::size_t                         parcelsDelivered {0};
};

/**
 * @brief Consulta, sin exponer el catálogo de envíos, la vista de seguimiento de un paquete.
 *
 * Quien lo implemente resuelve en el contexto de logística los movimientos del paquete y los traduce a hitos para
 * el cliente.
 */
class TrackedShipmentDirectory
{
public:
    virtual ~TrackedShipmentDirectory() = default;

    /// @brief La vista del paquete con ese código de seguimiento, o nada si no existe.
    virtual std::optional<TrackedShipment> find(const std::string& code) const = 0;
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_TRACKED_SHIPMENT_DIRECTORY_HPP
