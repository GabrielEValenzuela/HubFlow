#include "infrastructure/http/controllers/insightController.hpp"

#include "application/shared/vocabulary.hpp"
#include "domain/shared/businessError.hpp"
#include "infrastructure/http/presenters/jsonPresenter.hpp"
#include "infrastructure/http/validation/inputValidator.hpp"

namespace hubflow::infrastructure
{
namespace
{
using application::SetOperationKind;
using application::SetSelector;
using application::Vocabulary;

constexpr const char* OPERATIONS {"UNION, INTERSECTION, DIFFERENCE"};
constexpr const char* SELECTORS {"zone:CENTRO|NORTE|SUR o service:EXPRESS|PRIORITARIO|ESTANDAR"};

std::optional<SetOperationKind> operationKind(const std::string& text)
{
    std::optional<SetOperationKind> kind;
    if (text == "UNION")
    {
        kind = SetOperationKind::Union;
    }
    else if (text == "INTERSECTION")
    {
        kind = SetOperationKind::Intersection;
    }
    else if (text == "DIFFERENCE")
    {
        kind = SetOperationKind::Difference;
    }
    return kind;
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
        if (kind == "zone" && zone.has_value())
        {
            result = SetSelector {SetSelector::Kind::Zone, *zone, {}};
        }
        else if (kind == "service" && service.has_value())
        {
            result = SetSelector {SetSelector::Kind::Service, {}, *service};
        }
    }
    return result;
}

Json describe(const SetSelector& selector)
{
    return selector.kind == SetSelector::Kind::Zone ? Json {{"zone", Vocabulary::name(selector.zone)}}
                                                    : Json {{"service", Vocabulary::name(selector.service)}};
}
} // namespace

InsightController::InsightController(const application::InsightQueries& queries)
    : m_queries {queries}
{
}

void InsightController::registerRoutes(ApiRouter& router)
{
    router.get("/indexes/bst",
               [this](const RequestContext& context) { return HttpResponse::ok(JsonPresenter::bst(m_queries.bst(context.user()))); });
    router.get("/indexes/bst/entries/:code",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string code {input.trackingCode("code")};
                   input.validate();
                   return HttpResponse::ok(JsonPresenter::bstLookup(m_queries.bstLookup(context.user(), code)));
               });
    router.get("/indexes/bplus",
               [this](const RequestContext& context)
               { return HttpResponse::ok(JsonPresenter::bPlusTree(m_queries.bPlusTree(context.user()))); });
    router.get("/indexes/bplus/entries/:code",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string code {input.trackingCode("code")};
                   input.validate();
                   const auto lookup = m_queries.bPlusLookup(context.user(), code);
                   return HttpResponse::ok(Json {
                       {"found", lookup.pShipment != nullptr},
                       {"visited", lookup.visited},
                       {"shipment", lookup.pShipment == nullptr ? Json(nullptr) : JsonPresenter::shipment(*lookup.pShipment)}});
               });
    router.get("/indexes/bplus/entries",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string from {input.trackingCode("from")};
                   const std::string to {input.trackingCode("to")};
                   input.validate();
                   const auto range = m_queries.bPlusRange(context.user(), from, to);
                   return HttpResponse::ok(Json {{"from", from},
                                                 {"to", to},
                                                 {"visited", range.visited},
                                                 {"items", JsonPresenter::shipments(range.shipments)}});
               });
    router.get("/state-history-comparisons",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string first {input.trackingCode("first")};
                   const std::string second {input.trackingCode("second")};
                   input.validate();
                   Json body      = JsonPresenter::stateComparison(m_queries.compareStates(context.user(), first, second));
                   body["first"]  = first;
                   body["second"] = second;
                   return HttpResponse::ok(body);
               });
    router.get("/shipment-sets",
               [this](const RequestContext& context)
               {
                   const auto zone    = context.query("zone");
                   const auto service = context.query("service");
                   const auto chosen  = selector(zone.has_value() ? "zone:" + *zone : "service:" + service.value_or(""));
                   if ((zone.has_value() == service.has_value()) || !chosen.has_value())
                   {
                       throw validation_error {HttpStatus::UnprocessableEntity,
                                               {{"zone|service", "Indicá exactamente uno: zone o service válido"}}};
                   }
                   Json body        = JsonPresenter::codeSet(m_queries.codeSet(context.user(), *chosen));
                   body["criteria"] = describe(*chosen);
                   return HttpResponse::ok(body);
               });
    router.get("/shipment-set-operations", [this](const RequestContext& context) { return setOperation(context); });
}

HttpResponse InsightController::setOperation(const RequestContext& context) const
{
    auto       input     = InputValidator::parameters(context);
    const auto operation = input.requiredTerm<SetOperationKind>("operation", operationKind, OPERATIONS);
    const auto first     = input.requiredTerm<SetSelector>("first", selector, SELECTORS);
    const auto second    = input.requiredTerm<SetSelector>("second", selector, SELECTORS);
    input.validate();
    const auto combination = m_queries.combine(context.user(), operation, first, second);
    return HttpResponse::ok(Json {{"first", JsonPresenter::codeSet(combination.first)},
                                  {"second", JsonPresenter::codeSet(combination.second)},
                                  {"firstCriteria", describe(first)},
                                  {"secondCriteria", describe(second)},
                                  {"result", JsonPresenter::codeSet(combination.result)},
                                  {"shipments", JsonPresenter::shipments(combination.shipments)}});
}

} // namespace hubflow::infrastructure
