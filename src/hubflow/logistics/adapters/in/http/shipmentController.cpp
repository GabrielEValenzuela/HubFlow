#include "hubflow/logistics/adapters/in/http/shipmentController.hpp"

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/shared/application/vocabulary.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::adapters
{
namespace
{
using application::LookupIndex;
using platform::InputValidator;
using platform::Json;
using platform::RequestContext;
using shared::application::Vocabulary;

constexpr const char* ZONES {"CENTRO, NORTE, SUR"};
constexpr const char* SERVICES {"EXPRESS, PRIORITARIO, ESTANDAR"};
constexpr const char* INDEXES {"list, bst, bplus"};

struct IndexName
{
    const char* text;
    LookupIndex index;
};

constexpr IndexName INDEX_NAMES[] {{"list", LookupIndex::LinkedList},
                                   {"bst", LookupIndex::Bst},
                                   {"bplus", LookupIndex::BPlusTree}};

std::optional<LookupIndex> lookupIndex(const std::string& text)
{
    std::optional<LookupIndex> found;
    for (const auto& entry : INDEX_NAMES)
    {
        if (text == entry.text)
        {
            found = entry.index;
        }
    }
    return found;
}

std::string indexName(const LookupIndex index)
{
    std::string text;
    for (const auto& entry : INDEX_NAMES)
    {
        if (entry.index == index)
        {
            text = entry.text;
        }
    }
    return text;
}
} // namespace

ShipmentController::ShipmentController(ShipmentUseCases useCases)
    : m_useCases {useCases}
{
}

void ShipmentController::registerRoutes(platform::ApiRouter& router)
{
    router.get("/shipments",
               [this](const RequestContext& context)
               {
                   auto       input  = InputValidator::parameters(context);
                   const auto paging = input.page();
                   input.validate();
                   Json items = Json::array();
                   m_useCases.listShipments.execute(context.user())
                       .forEach(
                           [&items](const auto& shipment)
                           {
                               items.push_back(LogisticsJsonPresenter::shipmentSummary(shipment));
                           });
                   return platform::HttpResponse::ok(page(items, paging.first, paging.second));
               });
    router.post(
        "/shipments",
        [this](const RequestContext& context)
        {
            return registerShipment(context);
        },
        application::RegisterShipmentUseCase::IDEMPOTENCY);
    router.get("/shipments/:code",
               [this](const RequestContext& context)
               {
                   return find(context);
               });
    router.get("/shipments/:code/state-history",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string code {input.trackingCode("code")};
                   input.validate();
                   const auto& shipment = m_useCases.getShipment.execute(context.user(), code);
                   Json        body     = LogisticsJsonPresenter::stateHistory(shipment.stateHistory());
                   body["code"]         = shipment.code();
                   return platform::HttpResponse::ok(body);
               });
}

platform::HttpResponse ShipmentController::registerShipment(const RequestContext& context) const
{
    auto                  input = InputValidator::body(context.request);
    domain::ShipmentDraft draft;
    draft.code          = input.trackingCode("code");
    draft.recipient     = input.requiredText("recipient", 2, 80);
    draft.zone          = input.requiredTerm<domain::Zone>("zone", Vocabulary::zone, ZONES);
    draft.weightKg      = input.requiredNumber("weightKg", 0.01, domain::Weight::MAX_KILOGRAMS);
    draft.service       = input.requiredTerm<domain::ServiceLevel>("service", Vocabulary::serviceLevel, SERVICES);
    draft.deliveryPoint = input.requiredText("deliveryPoint", 1, 16);
    input.validate();
    const auto& shipment = m_useCases.registerShipment.execute(context.user(), draft);
    return platform::HttpResponse::created(LogisticsJsonPresenter::shipment(shipment),
                                           locationOf("/shipments/" + shipment.code()));
}

platform::HttpResponse ShipmentController::find(const RequestContext& context) const
{
    auto              input = InputValidator::parameters(context);
    const std::string code {input.trackingCode("code")};
    const auto index = input.optionalTerm<LookupIndex>("index", lookupIndex, INDEXES).value_or(LookupIndex::BPlusTree);
    input.validate();
    const auto result = m_useCases.findShipment.execute(context.user(), code, index);
    if (result.pShipment == nullptr)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::NotFound, "shipment.not_found", "Envío no encontrado."};
    }
    Json body      = LogisticsJsonPresenter::shipment(*result.pShipment);
    body["lookup"] = Json {{"index", indexName(index)}, {"visited", result.visited}};
    return platform::HttpResponse::ok(body);
}

} // namespace hubflow::logistics::adapters
