#include "hubflow/insights/adapters/in/http/insightController.hpp"

#include "hubflow/insights/adapters/in/http/insightsJsonPresenter.hpp"
#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::insights::adapters
{
namespace
{
using application::SetOperationKind;
using application::SetSelector;
using logistics::adapters::LogisticsJsonPresenter;
using platform::InputValidator;
using platform::Json;
using platform::RequestContext;
using shared::application::Vocabulary;

constexpr const char* OPERATIONS {"UNION, INTERSECTION, DIFFERENCE"};
constexpr const char* SELECTORS {"zone:CENTRO|NORTE|SUR o service:EXPRESS|PRIORITARIO|ESTANDAR"};
constexpr const char* ZONE_PREFIX {"zone"};
constexpr const char* SERVICE_PREFIX {"service"};

struct OperationName
{
    const char*      text;
    SetOperationKind kind;
};

constexpr OperationName OPERATION_NAMES[] {{"UNION", SetOperationKind::Union},
                                           {"INTERSECTION", SetOperationKind::Intersection},
                                           {"DIFFERENCE", SetOperationKind::Difference}};

std::optional<SetOperationKind> operationKind(const std::string& text)
{
    std::optional<SetOperationKind> found;
    for (const auto& entry : OPERATION_NAMES)
    {
        if (text == entry.text)
        {
            found = entry.kind;
        }
    }
    return found;
}

std::optional<SetSelector> selector(const std::string& text)
{
    std::optional<SetSelector> result;
    const auto                 separator = text.find(':');
    if (separator != std::string::npos)
    {
        const std::string kind {text.substr(0, separator)};
        const std::string value {text.substr(separator + 1)};
        const auto        zone    = Vocabulary::zone(value);
        const auto        service = Vocabulary::serviceLevel(value);
        if (kind == ZONE_PREFIX && zone.has_value())
        {
            result = SetSelector {SetSelector::Kind::Zone, *zone, {}};
        }
        else if (kind == SERVICE_PREFIX && service.has_value())
        {
            result = SetSelector {SetSelector::Kind::Service, {}, *service};
        }
    }
    return result;
}
} // namespace

InsightController::InsightController(InsightUseCases useCases)
    : m_useCases {useCases}
{
}

void InsightController::registerRoutes(platform::ApiRouter& router)
{
    router.get("/indexes/bst",
               [this](const RequestContext& context)
               {
                   return platform::HttpResponse::ok(
                       InsightsJsonPresenter::bst(m_useCases.getBst.execute(context.user())));
               });
    router.get("/indexes/bst/entries/:code",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string code {input.trackingCode("code")};
                   input.validate();
                   return platform::HttpResponse::ok(
                       InsightsJsonPresenter::bstLookup(m_useCases.lookupInBst.execute(context.user(), code)));
               });
    router.get("/indexes/bplus",
               [this](const RequestContext& context)
               {
                   return platform::HttpResponse::ok(
                       InsightsJsonPresenter::bPlusTree(m_useCases.getBPlusTree.execute(context.user())));
               });
    router.get("/indexes/bplus/entries/:code",
               [this](const RequestContext& context)
               {
                   return bPlusEntry(context);
               });
    router.get("/indexes/bplus/entries",
               [this](const RequestContext& context)
               {
                   return bPlusRange(context);
               });
    router.get("/state-history-comparisons",
               [this](const RequestContext& context)
               {
                   return stateComparison(context);
               });
    router.get("/shipment-sets",
               [this](const RequestContext& context)
               {
                   return shipmentSet(context);
               });
    router.get("/shipment-set-operations",
               [this](const RequestContext& context)
               {
                   return setOperation(context);
               });
}

platform::HttpResponse InsightController::bPlusEntry(const RequestContext& context) const
{
    auto              input = InputValidator::parameters(context);
    const std::string code {input.trackingCode("code")};
    input.validate();
    const auto lookup = m_useCases.lookupInBPlusTree.execute(context.user(), code);
    return platform::HttpResponse::ok(
        Json {{"found", lookup.pShipment != nullptr},
              {"visited", lookup.visited},
              {"shipment",
               lookup.pShipment == nullptr ? Json(nullptr) : LogisticsJsonPresenter::shipment(*lookup.pShipment)}});
}

platform::HttpResponse InsightController::bPlusRange(const RequestContext& context) const
{
    auto              input = InputValidator::parameters(context);
    const std::string from {input.trackingCode("from")};
    const std::string to {input.trackingCode("to")};
    input.validate();
    const auto range = m_useCases.rangeInBPlusTree.execute(context.user(), from, to);
    return platform::HttpResponse::ok(Json {{"from", from},
                                            {"to", to},
                                            {"visited", range.visited},
                                            {"items", LogisticsJsonPresenter::shipments(range.shipments)}});
}

platform::HttpResponse InsightController::stateComparison(const RequestContext& context) const
{
    auto              input = InputValidator::parameters(context);
    const std::string first {input.trackingCode("first")};
    const std::string second {input.trackingCode("second")};
    input.validate();
    Json body = InsightsJsonPresenter::stateComparison(m_useCases.compareStates.execute(context.user(), first, second));
    body["first"]  = first;
    body["second"] = second;
    return platform::HttpResponse::ok(body);
}

platform::HttpResponse InsightController::shipmentSet(const RequestContext& context) const
{
    const bool                 hasZone {context.request.has_param(ZONE_PREFIX)};
    const bool                 hasService {context.request.has_param(SERVICE_PREFIX)};
    std::optional<SetSelector> chosen;
    if (hasZone != hasService)
    {
        const std::string prefix {hasZone ? ZONE_PREFIX : SERVICE_PREFIX};
        chosen = selector(prefix + ":" + context.request.get_param_value(prefix));
    }
    if (!chosen.has_value())
    {
        throw platform::validation_error {platform::HttpStatus::UnprocessableEntity,
                                          {{"zone|service", "Indicá exactamente uno: zone o service válido"}}};
    }
    Json body        = InsightsJsonPresenter::codeSet(m_useCases.getCodeSet.execute(context.user(), *chosen));
    body["criteria"] = InsightsJsonPresenter::selector(*chosen);
    return platform::HttpResponse::ok(body);
}

platform::HttpResponse InsightController::setOperation(const RequestContext& context) const
{
    auto       input     = InputValidator::parameters(context);
    const auto operation = input.requiredTerm<SetOperationKind>("operation", operationKind, OPERATIONS);
    const auto first     = input.requiredTerm<SetSelector>("first", selector, SELECTORS);
    const auto second    = input.requiredTerm<SetSelector>("second", selector, SELECTORS);
    input.validate();
    const auto combination = m_useCases.combineCodeSets.execute(context.user(), operation, first, second);
    return platform::HttpResponse::ok(Json {{"first", InsightsJsonPresenter::codeSet(combination.first)},
                                            {"second", InsightsJsonPresenter::codeSet(combination.second)},
                                            {"firstCriteria", InsightsJsonPresenter::selector(first)},
                                            {"secondCriteria", InsightsJsonPresenter::selector(second)},
                                            {"result", InsightsJsonPresenter::codeSet(combination.result)},
                                            {"shipments", LogisticsJsonPresenter::shipments(combination.shipments)}});
}

} // namespace hubflow::insights::adapters
