#include "infrastructure/http/controllers/shipmentController.hpp"

#include "application/shared/vocabulary.hpp"
#include "domain/shared/businessError.hpp"
#include "infrastructure/http/presenters/jsonPresenter.hpp"
#include "infrastructure/http/validation/inputValidator.hpp"

namespace hubflow::infrastructure
{
namespace
{
using application::LookupIndex;
using application::Vocabulary;
using domain::logistics::ShipmentStatus;

constexpr const char* ZONES {"CENTRO, NORTE, SUR"};
constexpr const char* SERVICES {"EXPRESS, PRIORITARIO, ESTANDAR"};
constexpr const char* MOVEMENT_STATUSES {"CLASIFICADO, REPROGRAMADO, ENTREGADO"};
constexpr const char* INDEXES {"list, bst, bplus"};
constexpr const char* ORDERS {"asc, desc"};
constexpr std::size_t MAX_NOTE {200};

std::optional<LookupIndex> lookupIndex(const std::string& text)
{
    std::optional<LookupIndex> index;
    if (text == "list")
    {
        index = LookupIndex::LinkedList;
    }
    else if (text == "bst")
    {
        index = LookupIndex::Bst;
    }
    else if (text == "bplus")
    {
        index = LookupIndex::BPlusTree;
    }
    return index;
}

std::optional<bool> newestFirst(const std::string& text)
{
    return text == "asc" || text == "desc" ? std::optional<bool> {text == "desc"} : std::nullopt;
}

std::string indexName(const LookupIndex index)
{
    return index == LookupIndex::LinkedList ? "list" : index == LookupIndex::Bst ? "bst" : "bplus";
}
} // namespace

ShipmentController::ShipmentController(ShipmentUseCases useCases)
    : m_useCases {useCases}
    , m_transitions {
          {ShipmentStatus::Clasificado,
           [this](const auto& principal, const auto& code, const auto& note) -> const domain::logistics::Shipment&
           { return m_useCases.classify.execute(principal, code, note); }},
          {ShipmentStatus::Reprogramado,
           [this](const auto& principal, const auto& code, const auto& note) -> const domain::logistics::Shipment&
           { return m_useCases.reschedule.execute(principal, code, note); }},
          {ShipmentStatus::Entregado,
           [this](const auto& principal, const auto& code, const auto& note) -> const domain::logistics::Shipment&
           { return m_useCases.deliver.execute(principal, code, note); }}}
{
}

void ShipmentController::registerRoutes(ApiRouter& router)
{
    router.get("/shipments",
               [this](const RequestContext& context)
               {
                   auto input           = InputValidator::parameters(context);
                   const auto [limit, offset] = input.page();
                   input.validate();
                   Json items = Json::array();
                   m_useCases.queries.catalog(context.user())
                       .forEach([&items](const auto& shipment) { items.push_back(JsonPresenter::shipmentSummary(shipment)); });
                   return HttpResponse::ok(page(items, limit, offset));
               });
    router.post("/shipments",
                [this](const RequestContext& context)
                {
                    auto                             input = InputValidator::body(context.request);
                    domain::logistics::ShipmentDraft draft;
                    draft.code          = input.trackingCode("code");
                    draft.recipient     = input.requiredText("recipient", 2, 80);
                    draft.zone          = input.requiredTerm<domain::logistics::Zone>("zone", Vocabulary::zone, ZONES);
                    draft.weightKg      = input.requiredNumber("weightKg", 0.01, domain::logistics::Weight::MAX_KILOGRAMS);
                    draft.service       = input.requiredTerm<domain::logistics::ServiceLevel>("service", Vocabulary::serviceLevel, SERVICES);
                    draft.deliveryPoint = input.requiredText("deliveryPoint", 1, 16);
                    input.validate();
                    const auto& shipment = m_useCases.registerShipment.execute(context.user(), draft);
                    return HttpResponse::created(JsonPresenter::shipment(shipment), "/api/v1/shipments/" + shipment.code());
                });
    router.get("/shipments/:code",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string code {input.trackingCode("code")};
                   const auto index = input.optionalTerm<LookupIndex>("index", lookupIndex, INDEXES).value_or(LookupIndex::BPlusTree);
                   input.validate();
                   const auto result = m_useCases.queries.find(context.user(), code, index);
                   if (result.pShipment == nullptr)
                   {
                       throw domain::business_error {domain::ErrorKind::NotFound, "shipment.not_found", "Envío no encontrado."};
                   }
                   Json body      = JsonPresenter::shipment(*result.pShipment);
                   body["lookup"] = Json {{"index", indexName(index)}, {"visited", result.visited}};
                   return HttpResponse::ok(body);
               });
    router.get("/shipments/:code/movements",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string code {input.trackingCode("code")};
                   const bool        reverse {input.optionalTerm<bool>("order", newestFirst, ORDERS).value_or(false)};
                   input.validate();
                   return HttpResponse::ok(JsonPresenter::history(m_useCases.queries.require(context.user(), code), reverse));
               });
    router.post("/shipments/:code/movements", [this](const RequestContext& context) { return recordMovement(context); });
    router.get("/shipments/:code/state-history",
               [this](const RequestContext& context)
               {
                   auto              input = InputValidator::parameters(context);
                   const std::string code {input.trackingCode("code")};
                   input.validate();
                   const auto& shipment = m_useCases.queries.require(context.user(), code);
                   Json        body     = JsonPresenter::stateHistory(shipment.stateHistory());
                   body["code"]         = shipment.code();
                   return HttpResponse::ok(body);
               });
    router.get("/pending-shipments",
               [this](const RequestContext& context)
               {
                   auto input           = InputValidator::parameters(context);
                   const auto [limit, offset] = input.page();
                   input.validate();
                   Json items = Json::array();
                   m_useCases.queries.pending(context.user())
                       .forEach([&items](const auto& shipment) { items.push_back(JsonPresenter::shipmentSummary(shipment)); });
                   return HttpResponse::ok(page(items, limit, offset));
               });
    router.post("/dispatches",
                [this](const RequestContext& context)
                {
                    auto              input = InputValidator::body(context.request);
                    const std::string note {input.optionalText("note", MAX_NOTE).value_or("")};
                    input.validate();
                    const auto& shipment = m_useCases.dispatchNext.execute(context.user(), note);
                    return HttpResponse::created(JsonPresenter::shipment(shipment), "/api/v1/shipments/" + shipment.code());
                });
    router.get("/zones/:zone/summary",
               [this](const RequestContext& context)
               {
                   auto       input = InputValidator::parameters(context);
                   const auto zone  = input.requiredTerm<domain::logistics::Zone>("zone", Vocabulary::zone, ZONES);
                   input.validate();
                   return HttpResponse::ok(JsonPresenter::zoneSummary(zone, m_useCases.queries.zoneSummary(context.user(), zone)));
               });
    router.get("/zones/:zone/heaviest-shipment",
               [this](const RequestContext& context)
               {
                   auto       input = InputValidator::parameters(context);
                   const auto zone  = input.requiredTerm<domain::logistics::Zone>("zone", Vocabulary::zone, ZONES);
                   input.validate();
                   const auto* pShipment = m_useCases.queries.heaviest(context.user(), zone);
                   if (pShipment == nullptr)
                   {
                       throw domain::business_error {
                           domain::ErrorKind::NotFound, "zone.empty", "No hay envíos pendientes para la zona seleccionada."};
                   }
                   return HttpResponse::ok(JsonPresenter::shipmentSummary(*pShipment));
               });
    router.get("/metrics",
               [this](const RequestContext& context)
               { return HttpResponse::ok(JsonPresenter::metrics(m_useCases.queries.metrics(context.user()))); });
}

HttpResponse ShipmentController::recordMovement(const RequestContext& context)
{
    auto              path = InputValidator::parameters(context);
    const std::string code {path.trackingCode("code")};
    path.validate();
    auto              input  = InputValidator::body(context.request);
    const auto        status = input.requiredTerm<ShipmentStatus>("status", Vocabulary::shipmentStatus, MOVEMENT_STATUSES);
    const std::string note {input.requiredText("note", 3, MAX_NOTE)};
    input.validate();
    const auto transition = m_transitions.find(status);
    if (transition == m_transitions.end())
    {
        throw validation_error {HttpStatus::UnprocessableEntity,
                                {{"status", "Usá POST /dispatches o una hoja de ruta para EN_REPARTO; RECIBIDO es inicial"}}};
    }
    const domain::logistics::Shipment* pShipment = &transition->second(context.user(), code, note);
    return HttpResponse::created(JsonPresenter::movement(*pShipment->history().latest()),
                                 "/api/v1/shipments/" + code + "/movements");
}

} // namespace hubflow::infrastructure
