#ifndef HUBFLOW_TRACKING_APPLICATION_USECASES_TRACKING_VIEW_BUILDER_HPP
#define HUBFLOW_TRACKING_APPLICATION_USECASES_TRACKING_VIEW_BUILDER_HPP

#include "hubflow/tracking/application/ports/out/trackedShipmentDirectory.hpp"
#include "hubflow/tracking/application/trackingViews.hpp"

#include <string>

namespace hubflow::tracking::application
{

/// @brief Arma la vista pública del seguimiento y normaliza el código que escribe el cliente.
class TrackingViewBuilder final
{
public:
    /// @brief Mayúsculas y sin espacios en los extremos: el cliente puede escribir el código como quiera.
    static std::string normalize(const std::string& code);

    /// @brief true si el texto tiene el formato de un código de seguimiento (público o secuencial).
    static bool looksLikeCode(const std::string& code);

    static PublicTracking publicView(const TrackedShipment& shipment);
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_USECASES_TRACKING_VIEW_BUILDER_HPP
