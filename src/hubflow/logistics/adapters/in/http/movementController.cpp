#include "hubflow/logistics/adapters/in/http/movementController.hpp"

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::logistics::adapters
{
    namespace
    {
        using domain::ShipmentStatus;
        using platform::InputValidator;
        using platform::RequestContext;
        using shared::application::Vocabulary;

        constexpr const char* MOVEMENT_STATUSES {"CLASIFICADO, REPROGRAMADO, ENTREGADO"};
        constexpr const char* ORDERS {"asc, desc"};
        constexpr std::size_t MIN_NOTE {3};
        constexpr std::size_t MAX_NOTE {200};

        std::optional<bool> newestFirst(const std::string& text)
        {
            return text == "asc" || text == "desc" ? std::optional<bool> {text == "desc"} : std::nullopt;
        }
    } // namespace

    MovementController::MovementController(const MovementUseCases& useCases)
        : m_useCases {useCases}
        , m_transitions {
              {ShipmentStatus::Clasificado,
               [this](const auto& principal, const auto& code, const auto& note, const bool) -> const domain::Shipment&
               { return m_useCases.classify.execute(principal, code, note); }},
              {ShipmentStatus::Reprogramado,
               [this](const auto& principal, const auto& code, const auto& note, const bool) -> const domain::Shipment&
               { return m_useCases.reschedule.execute(principal, code, note); }},
              {ShipmentStatus::Entregado,
               [this](const auto& principal, const auto& code, const auto& note, const bool confirmAll)
                   -> const domain::Shipment&
               { return m_useCases.deliver.execute(principal, code, note, confirmAll); }}}
    {
    }

    void MovementController::registerRoutes(platform::ApiRouter& router)
    {
        router.get("/shipments/:code/movements",
                   [this](const RequestContext& context)
                   {
                       auto input = InputValidator::parameters(context);
                       const std::string code {input.trackingCode("code")};
                       const bool reverse {input.optionalTerm<bool>("order", newestFirst, ORDERS).value_or(false)};
                       input.validate();
                       return platform::HttpResponse::ok(LogisticsJsonPresenter::history(
                           m_useCases.getShipment.execute(context.user(), code), reverse));
                   });
        router.post(
            "/shipments/:code/movements",
            [this](const RequestContext& context) { return record(context); },
            application::ClassifyShipmentUseCase::IDEMPOTENCY);
    }

    platform::HttpResponse MovementController::record(const RequestContext& context) const
    {
        auto path = InputValidator::parameters(context);
        const std::string code {path.trackingCode("code")};
        path.validate();
        auto input = InputValidator::body(context.request);
        const auto status = input.requiredTerm<ShipmentStatus>("status", Vocabulary::shipmentStatus, MOVEMENT_STATUSES);
        const std::string note {input.requiredText("note", MIN_NOTE, MAX_NOTE)};
        const bool confirmAll {input.optionalFlag("confirmAllParcels").value_or(false)};
        input.validate();
        const auto transition = m_transitions.find(status);
        if (transition == m_transitions.end())
        {
            throw platform::ValidationError {
                platform::HttpStatus::UnprocessableEntity,
                {{"status", "Usá POST /dispatches o una hoja de ruta para EN_REPARTO; RECIBIDO es inicial"}}};
        }
        const domain::Shipment& shipment = transition->second(context.user(), code, note, confirmAll);
        return platform::HttpResponse::created(LogisticsJsonPresenter::movement(*shipment.history().latest()),
                                               locationOf("/shipments/" + code + "/movements"));
    }

} // namespace hubflow::logistics::adapters
