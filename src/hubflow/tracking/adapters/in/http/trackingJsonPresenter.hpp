#ifndef HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_TRACKING_JSON_PRESENTER_HPP
#define HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_TRACKING_JSON_PRESENTER_HPP

#include "hubflow/platform/http/httpTypes.hpp"
#include "hubflow/tracking/application/ports/out/customerNoticeLog.hpp"
#include "hubflow/tracking/application/trackingViews.hpp"

namespace hubflow::tracking::adapters
{

/// @brief Traduce las vistas de seguimiento a JSON. Es el único lugar que decide qué campos salen al cliente.
class TrackingJsonPresenter final
{
public:
    static platform::Json publicTracking(const application::PublicTracking& view);
    static platform::Json verifiedTracking(const application::VerifiedTracking& view);
    static platform::Json access(const application::TrackingAccess& access);
    static platform::Json notice(const application::StoredCustomerNotice& stored);
};

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_TRACKING_JSON_PRESENTER_HPP
