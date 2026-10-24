#include "hubflow/tracking/application/usecases/commands/verifyTracking.hpp"

#include "hubflow/tracking/application/trackingErrors.hpp"
#include "hubflow/tracking/application/usecases/trackingViewBuilder.hpp"
#include "hubflow/tracking/domain/services/verificationProof.hpp"

namespace hubflow::tracking::application
{

    VerifyTracking::VerifyTracking(const TrackedShipmentDirectory& directory,
                                   shared::application::RateLimiter& limiter,
                                   const TrackingTokenIssuer& tokens,
                                   const shared::application::Clock& clock)
        : m_directory {directory}
        , m_limiter {limiter}
        , m_tokens {tokens}
        , m_clock {clock}
    {
    }

    TrackingAccess VerifyTracking::execute(const VerificationRequest& request)
    {
        const std::string code {TrackingViewBuilder::normalize(request.code)};
        const auto now = m_clock.now();
        const auto decision = m_limiter.attempt(request.clientAddress + "|" + code, now);
        if (!decision.allowed)
        {
            throw TrackingErrors::tooManyAttempts(decision.retryAfterSeconds);
        }
        const domain::VerificationProof proof {request.phoneLast4, request.postalCode};
        if (proof.isEmpty())
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "tracking.proof_missing",
                                                  "Indicá los últimos 4 dígitos del teléfono o el código postal"};
        }
        const auto shipment =
            TrackingViewBuilder::looksLikeCode(code) ? m_directory.find(code) : std::optional<TrackedShipment> {};
        const bool verified {domain::ProofCheck::matches(proof,
                                                         shipment.has_value() ? shipment->phoneLast4 : std::string {},
                                                         shipment.has_value() ? shipment->postalCode : std::string {})};
        if (!verified)
        {
            throw TrackingErrors::notFound();
        }
        const std::int64_t expiresAt {now + TOKEN_LIFETIME_SECONDS};
        return TrackingAccess {m_tokens.issue(TrackingGrant {code, expiresAt}), expiresAt};
    }

} // namespace hubflow::tracking::application
