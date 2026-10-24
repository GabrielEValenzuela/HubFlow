#include "hubflow/tracking/application/usecases/queries/getVerifiedTracking.hpp"

#include "hubflow/tracking/application/trackingErrors.hpp"
#include "hubflow/tracking/application/usecases/trackingViewBuilder.hpp"
#include "hubflow/tracking/domain/services/deliveryWindow.hpp"

namespace hubflow::tracking::application
{

    GetVerifiedTracking::GetVerifiedTracking(const TrackedShipmentDirectory& directory,
                                             const DeliveryProgressSource& progress,
                                             const TrackingTokenIssuer& tokens,
                                             const shared::application::Clock& clock)
        : m_directory {directory}
        , m_progress {progress}
        , m_tokens {tokens}
        , m_clock {clock}
    {
    }

    VerifiedTracking GetVerifiedTracking::execute(const std::string& code, const std::string& token) const
    {
        const std::string normalized {TrackingViewBuilder::normalize(code)};
        const auto now = m_clock.now();
        const auto grant = m_tokens.verify(token, now);
        const bool inScope = grant.has_value() && grant->code == normalized;
        const auto shipment = inScope ? m_directory.find(normalized) : std::optional<TrackedShipment> {};
        if (!shipment.has_value())
        {
            throw TrackingErrors::verificationRequired();
        }
        VerifiedTracking view;
        view.summary = TrackingViewBuilder::publicView(*shipment);
        view.recipientFirstName = shipment->recipientFirstName;
        view.zoneName = shipment->zoneName;
        view.neighbourhood = shipment->neighbourhood;
        view.parcelCount = shipment->parcelCount;
        view.parcelsDelivered = shipment->parcelsDelivered;
        const auto progress =
            view.summary.finished ? std::optional<DeliveryProgress> {} : m_progress.progressOf(normalized);
        if (progress.has_value())
        {
            view.arrival = EstimatedArrival {domain::DeliveryWindowPolicy::around(now, progress->minutesToArrival),
                                             progress->stopsBefore};
        }
        return view;
    }

} // namespace hubflow::tracking::application
