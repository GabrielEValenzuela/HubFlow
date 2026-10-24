#include "hubflow/tracking/application/usecases/queries/getPublicTracking.hpp"

#include "hubflow/tracking/application/trackingErrors.hpp"
#include "hubflow/tracking/application/usecases/trackingViewBuilder.hpp"

namespace hubflow::tracking::application
{

    GetPublicTracking::GetPublicTracking(const TrackedShipmentDirectory& directory)
        : m_directory {directory}
    {
    }

    PublicTracking GetPublicTracking::execute(const std::string& code) const
    {
        const std::string normalized {TrackingViewBuilder::normalize(code)};
        const auto shipment = TrackingViewBuilder::looksLikeCode(normalized) ? m_directory.find(normalized)
                                                                             : std::optional<TrackedShipment> {};
        if (!shipment.has_value())
        {
            throw TrackingErrors::notFound();
        }
        return TrackingViewBuilder::publicView(*shipment);
    }

} // namespace hubflow::tracking::application
