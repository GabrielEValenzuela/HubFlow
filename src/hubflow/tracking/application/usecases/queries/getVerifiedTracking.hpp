#ifndef HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_GET_VERIFIED_TRACKING_HPP
#define HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_GET_VERIFIED_TRACKING_HPP

#include "hubflow/shared/application/ports/out/clock.hpp"
#include "hubflow/tracking/application/ports/in/getVerifiedTrackingUseCase.hpp"
#include "hubflow/tracking/application/ports/out/deliveryProgressSource.hpp"
#include "hubflow/tracking/application/ports/out/trackedShipmentDirectory.hpp"
#include "hubflow/tracking/application/ports/out/trackingTokenIssuer.hpp"

namespace hubflow::tracking::application
{

    /// @brief Implementación de GetVerifiedTrackingUseCase.
    class GetVerifiedTracking final : public GetVerifiedTrackingUseCase
    {
    public:
        GetVerifiedTracking(const TrackedShipmentDirectory& directory,
                            const DeliveryProgressSource& progress,
                            const TrackingTokenIssuer& tokens,
                            const shared::application::Clock& clock);

    private:
        VerifiedTracking execute(const std::string& code, const std::string& token) const override;

        const TrackedShipmentDirectory& m_directory;
        const DeliveryProgressSource& m_progress;
        const TrackingTokenIssuer& m_tokens;
        const shared::application::Clock& m_clock;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_GET_VERIFIED_TRACKING_HPP
