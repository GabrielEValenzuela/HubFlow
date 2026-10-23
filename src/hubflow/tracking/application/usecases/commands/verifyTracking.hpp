#ifndef HUBFLOW_TRACKING_APPLICATION_USECASES_COMMANDS_VERIFY_TRACKING_HPP
#define HUBFLOW_TRACKING_APPLICATION_USECASES_COMMANDS_VERIFY_TRACKING_HPP

#include "hubflow/shared/application/ports/out/clock.hpp"
#include "hubflow/shared/application/ports/out/rateLimiter.hpp"
#include "hubflow/tracking/application/ports/in/verifyTrackingUseCase.hpp"
#include "hubflow/tracking/application/ports/out/trackedShipmentDirectory.hpp"
#include "hubflow/tracking/application/ports/out/trackingTokenIssuer.hpp"

namespace hubflow::tracking::application
{

/// @brief Implementación de VerifyTrackingUseCase.
class VerifyTracking final : public VerifyTrackingUseCase
{
public:
    /// @brief Segundos de vigencia de una constancia: 15 minutos.
    static constexpr std::int64_t TOKEN_LIFETIME_SECONDS {15 * 60};

    VerifyTracking(const TrackedShipmentDirectory&   directory,
                   shared::application::RateLimiter& limiter,
                   const TrackingTokenIssuer&        tokens,
                   const shared::application::Clock& clock);

private:
    TrackingAccess execute(const VerificationRequest& request) override;

    const TrackedShipmentDirectory&   m_directory;
    shared::application::RateLimiter& m_limiter;
    const TrackingTokenIssuer&        m_tokens;
    const shared::application::Clock& m_clock;
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_USECASES_COMMANDS_VERIFY_TRACKING_HPP
