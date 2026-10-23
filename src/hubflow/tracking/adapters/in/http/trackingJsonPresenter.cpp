#include "hubflow/tracking/adapters/in/http/trackingJsonPresenter.hpp"

#include "hubflow/tracking/domain/services/customerWording.hpp"

namespace hubflow::tracking::adapters
{
namespace
{
using platform::Json;

Json stepOf(const domain::TimelineEntry& entry)
{
    return Json {
        {"stage", domain::CustomerWording::stage(entry.milestone)}, {"label", entry.text}, {"at", entry.occurredAt}};
}
} // namespace

Json TrackingJsonPresenter::publicTracking(const application::PublicTracking& view)
{
    Json timeline = Json::array();
    for (const auto& entry : view.timeline)
    {
        timeline.push_back(stepOf(entry));
    }
    return Json {
        {"code", view.code}, {"status", stepOf(view.current)}, {"finished", view.finished}, {"timeline", timeline}};
}

Json TrackingJsonPresenter::verifiedTracking(const application::VerifiedTracking& view)
{
    Json body       = publicTracking(view.summary);
    body["details"] = Json {{"recipientFirstName", view.recipientFirstName},
                            {"zone", view.zoneName},
                            {"neighbourhood", view.neighbourhood},
                            {"parcels", Json {{"total", view.parcelCount}, {"delivered", view.parcelsDelivered}}},
                            {"estimatedArrival", nullptr}};
    if (view.arrival.has_value())
    {
        body["details"]["estimatedArrival"] = Json {{"from", view.arrival->window.from},
                                                    {"to", view.arrival->window.to},
                                                    {"stopsBefore", view.arrival->stopsBefore}};
    }
    return body;
}

Json TrackingJsonPresenter::access(const application::TrackingAccess& access)
{
    return Json {{"token", access.token}, {"expiresAt", access.expiresAt}};
}

Json TrackingJsonPresenter::notice(const application::StoredCustomerNotice& stored)
{
    return Json {{"id", stored.id},
                 {"shipmentCode", stored.notice.shipmentCode},
                 {"channel", stored.notice.channel},
                 {"recipient", stored.notice.recipient},
                 {"message", stored.notice.message},
                 {"sentAt", stored.notice.sentAt}};
}

} // namespace hubflow::tracking::adapters
