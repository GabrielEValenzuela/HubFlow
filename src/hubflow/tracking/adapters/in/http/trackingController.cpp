#include "hubflow/tracking/adapters/in/http/trackingController.hpp"

#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/tracking/adapters/in/http/trackingJsonPresenter.hpp"
#include "hubflow/tracking/application/trackingErrors.hpp"

#include <utility>

namespace hubflow::tracking::adapters
{
namespace
{
using platform::Access;
using platform::HttpResponse;
using platform::Json;
using platform::RequestContext;

constexpr const char* BEARER_PREFIX {"Bearer "};
constexpr const char* NO_STORE {"no-store"};

HttpResponse uncached(HttpResponse response)
{
    response.headers["Cache-Control"] = NO_STORE;
    return response;
}

std::string bearerOf(const httplib::Request& request)
{
    const std::string header {request.get_header_value("Authorization")};
    return header.rfind(BEARER_PREFIX, 0) == 0 ? header.substr(std::char_traits<char>::length(BEARER_PREFIX))
                                               : std::string {};
}
} // namespace

TrackingController::TrackingController(TrackingUseCases useCases, std::string trackingLinkBase)
    : m_useCases {useCases}
    , m_trackingLinkBase {std::move(trackingLinkBase)}
{
}

void TrackingController::registerRoutes(platform::ApiRouter& router)
{
    router.get(
        "/public/tracking/:code",
        [this](const RequestContext& context)
        {
            return show(context);
        },
        Access::Public);
    router.post(
        "/public/tracking/:code/verifications",
        [this](const RequestContext& context)
        {
            return verify(context);
        },
        application::VerifyTrackingUseCase::IDEMPOTENCY,
        Access::Public);
    router.get(
        "/public/tracking-settings",
        [this](const RequestContext&)
        {
            return HttpResponse::ok(Json {{"trackingLinkBase", m_trackingLinkBase}});
        },
        Access::Public);
}

HttpResponse TrackingController::show(const RequestContext& context) const
{
    const std::string code {context.request.path_params.at("code")};
    const std::string token {bearerOf(context.request)};
    HttpResponse      response;
    if (context.request.has_header("Authorization"))
    {
        response = HttpResponse::ok(
            TrackingJsonPresenter::verifiedTracking(m_useCases.getVerifiedTracking.execute(code, token)));
    }
    else
    {
        response = HttpResponse::ok(TrackingJsonPresenter::publicTracking(m_useCases.getPublicTracking.execute(code)));
    }
    return uncached(std::move(response));
}

HttpResponse TrackingController::verify(const RequestContext& context) const
{
    auto       input      = platform::InputValidator::body(context.request);
    const auto phoneLast4 = input.optionalText("phoneLast4", 4);
    const auto postalCode = input.optionalText("postalCode", 12);
    input.validate();
    application::VerificationRequest request;
    request.clientAddress = context.request.remote_addr;
    request.code          = context.request.path_params.at("code");
    request.phoneLast4    = phoneLast4.value_or("");
    request.postalCode    = postalCode.value_or("");
    return uncached(HttpResponse::ok(TrackingJsonPresenter::access(m_useCases.verifyTracking.execute(request))));
}

} // namespace hubflow::tracking::adapters
