#include "bootstrap/modules/trackingModule.hpp"

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/platform/config/settings.hpp"
#include "hubflow/platform/http/apiRouter.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/shared/adapters/out/memory/slidingWindowRateLimiter.hpp"
#include "hubflow/tracking/adapters/in/http/customerNoticeController.hpp"
#include "hubflow/tracking/adapters/in/http/trackingController.hpp"
#include "hubflow/tracking/adapters/out/logistics/plantTrackedShipments.hpp"
#include "hubflow/tracking/adapters/out/mysql/mySqlCustomerNoticeLog.hpp"
#include "hubflow/tracking/adapters/out/notification/compositeCustomerNotifier.hpp"
#include "hubflow/tracking/adapters/out/notification/logCustomerNotifier.hpp"
#include "hubflow/tracking/adapters/out/notification/recordingCustomerNotifier.hpp"
#include "hubflow/tracking/adapters/out/routing/routingDeliveryProgress.hpp"
#include "hubflow/tracking/adapters/out/security/hmacTrackingTokenIssuer.hpp"
#include "hubflow/tracking/application/subscribers/customerNoticeSubscriber.hpp"
#include "hubflow/tracking/application/usecases/commands/verifyTracking.hpp"
#include "hubflow/tracking/application/usecases/queries/getPublicTracking.hpp"
#include "hubflow/tracking/application/usecases/queries/getVerifiedTracking.hpp"
#include "hubflow/tracking/application/usecases/queries/listCustomerNotices.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace hubflow::bootstrap
{
    namespace
    {
        constexpr std::size_t VERIFICATION_ATTEMPTS {5};
        constexpr std::int64_t VERIFICATION_WINDOW_SECONDS {15 * 60};
    } // namespace

    struct TrackingModule::Parts
    {
        Parts(const SharedServices& shared,
              const logistics::application::LogisticsModel& logistics,
              const routing::application::RoutingModel& routing)
            : trackingLinkBase {shared.settings.trackingLinkBase()}
            , noticeLog {shared.connection}
            , trackingTokens {shared.settings.jwtSecret}
            , verificationLimiter {VERIFICATION_ATTEMPTS, VERIFICATION_WINDOW_SECONDS}
            , trackedShipments {logistics, routing}
            , deliveryProgress {routing}
            , recordingNotifier {noticeLog}
            , logNotifier {std::cout}
            , customerNotices {customerNotifier, trackingLinkBase}
            , getPublicTracking {trackedShipments}
            , verifyTracking {trackedShipments, verificationLimiter, trackingTokens, shared.clock}
            , getVerifiedTracking {trackedShipments, deliveryProgress, trackingTokens, shared.clock}
            , listCustomerNotices {noticeLog}
        {
            customerNotifier.add(logNotifier);
            customerNotifier.add(recordingNotifier);
            subscribers.add(SubscriptionPhase::CustomerNotice, customerNotices);
        }

        std::string trackingLinkBase;
        tracking::adapters::MySqlCustomerNoticeLog noticeLog;
        tracking::adapters::HmacTrackingTokenIssuer trackingTokens;
        shared::adapters::SlidingWindowRateLimiter verificationLimiter;
        tracking::adapters::PlantTrackedShipments trackedShipments;
        tracking::adapters::RoutingDeliveryProgress deliveryProgress;
        tracking::adapters::RecordingCustomerNotifier recordingNotifier;
        tracking::adapters::LogCustomerNotifier logNotifier;
        tracking::adapters::CompositeCustomerNotifier customerNotifier;
        tracking::application::CustomerNoticeSubscriber customerNotices;
        tracking::application::GetPublicTracking getPublicTracking;
        tracking::application::VerifyTracking verifyTracking;
        tracking::application::GetVerifiedTracking getVerifiedTracking;
        tracking::application::ListCustomerNotices listCustomerNotices;
        PhasedSubscribers subscribers;
        std::vector<std::unique_ptr<platform::Controller>> controllers;
    };

    TrackingModule::TrackingModule(const SharedServices& shared,
                                   const logistics::application::LogisticsModel& logistics,
                                   const routing::application::RoutingModel& routing)
        : m_spParts {std::make_unique<Parts>(shared, logistics, routing)}
    {
    }

    TrackingModule::~TrackingModule() = default;

    void TrackingModule::subscribe(const SubscriptionPhase phase, shared::application::EventBus& bus)
    {
        m_spParts->subscribers.subscribe(phase, bus);
    }

    void TrackingModule::registerRoutes(platform::ApiRouter& router)
    {
        Parts& parts = *m_spParts;
        parts.controllers.push_back(std::make_unique<tracking::adapters::TrackingController>(
            tracking::adapters::TrackingUseCases {
                parts.getPublicTracking, parts.verifyTracking, parts.getVerifiedTracking},
            parts.trackingLinkBase));
        parts.controllers.push_back(
            std::make_unique<tracking::adapters::CustomerNoticeController>(parts.listCustomerNotices));
        for (auto& spController : parts.controllers)
        {
            spController->registerRoutes(router);
        }
    }

} // namespace hubflow::bootstrap
