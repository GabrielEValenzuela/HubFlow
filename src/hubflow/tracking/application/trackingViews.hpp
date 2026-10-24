#ifndef HUBFLOW_TRACKING_APPLICATION_TRACKING_VIEWS_HPP
#define HUBFLOW_TRACKING_APPLICATION_TRACKING_VIEWS_HPP

#include "hubflow/tracking/domain/services/customerTimeline.hpp"
#include "hubflow/tracking/domain/services/deliveryWindow.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace hubflow::tracking::application
{

    /**
     * @brief Lo que cualquiera que tenga el código puede ver: dónde está el paquete y su recorrido.
     *
     * No incluye notas internas, nombres de quien operó el paquete ni datos del destinatario.
     */
    struct PublicTracking
    {
        std::string code;
        domain::TimelineEntry current;
        bool finished {false};
        std::vector<domain::TimelineEntry> timeline;
    };

    /// @brief Franja de entrega estimada y cuántas paradas faltan antes de llegar.
    struct EstimatedArrival
    {
        domain::DeliveryWindow window;
        std::size_t stopsBefore {0};
    };

    /**
     * @brief Detalle ampliado para quien confirmó que espera el paquete.
     *
     * Del destinatario sólo se muestra el primer nombre. La franja estimada aparece cuando el paquete va en camino.
     */
    struct VerifiedTracking
    {
        PublicTracking summary;
        std::string recipientFirstName;
        std::string zoneName;
        std::string neighbourhood;
        std::size_t parcelCount {0};
        std::size_t parcelsDelivered {0};
        std::optional<EstimatedArrival> arrival;
    };

    /// @brief Constancia de verificación: el token y cuándo vence.
    struct TrackingAccess
    {
        std::string token;
        std::int64_t expiresAt {0};
    };

    /// @brief Intento de verificación: desde dónde llega, sobre qué código y con qué dato.
    struct VerificationRequest
    {
        std::string clientAddress;
        std::string code;
        std::string phoneLast4;
        std::string postalCode;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_TRACKING_VIEWS_HPP
