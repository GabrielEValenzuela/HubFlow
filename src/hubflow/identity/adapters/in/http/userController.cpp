#include "hubflow/identity/adapters/in/http/userController.hpp"

#include "hubflow/identity/adapters/in/http/identityJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::identity::adapters
{
namespace
{
using platform::InputValidator;
using platform::RequestContext;
using shared::application::Vocabulary;

constexpr const char* ROLES {"ADMIN, OPERADOR, PUM"};
constexpr std::size_t MIN_PASSWORD {8};
constexpr std::size_t MAX_PASSWORD {72};
constexpr std::size_t MIN_NAME {2};
constexpr std::size_t MAX_NAME {80};
} // namespace

UserController::UserController(UserUseCases useCases)
    : m_useCases {useCases}
{
}

void UserController::registerRoutes(platform::ApiRouter& router)
{
    router.get("/users",
               [this](const RequestContext& context)
               {
                   auto       input  = InputValidator::parameters(context);
                   const auto paging = input.page();
                   input.validate();
                   return platform::HttpResponse::ok(
                       page(IdentityJsonPresenter::users(m_useCases.listUsers.execute(context.user())),
                            paging.first,
                            paging.second));
               });
    router.get("/couriers",
               [this](const RequestContext& context)
               {
                   return platform::HttpResponse::ok(platform::Json {
                       {"items", IdentityJsonPresenter::users(m_useCases.listCouriers.execute(context.user()))}});
               });
    router.post(
        "/users",
        [this](const RequestContext& context)
        {
            auto                             input = InputValidator::body(context.request);
            application::RegisterUserRequest request;
            request.email    = input.email("email");
            request.name     = input.requiredText("name", MIN_NAME, MAX_NAME);
            request.role     = input.requiredTerm<domain::Role>("role", Vocabulary::role, ROLES);
            request.password = input.requiredText("password", MIN_PASSWORD, MAX_PASSWORD);
            input.validate();
            const auto user = m_useCases.registerUser.execute(context.user(), request);
            return platform::HttpResponse::created(IdentityJsonPresenter::user(user),
                                                   locationOf("/users/" + std::to_string(user.id)));
        },
        application::RegisterUserUseCase::IDEMPOTENCY);
    router.get("/users/:id",
               [this](const RequestContext& context)
               {
                   auto                input = InputValidator::parameters(context);
                   const std::uint64_t id {input.requiredIdentifier("id")};
                   input.validate();
                   return platform::HttpResponse::ok(
                       IdentityJsonPresenter::user(m_useCases.findUser.execute(context.user(), id)));
               });
    router.patch("/users/:id",
                 [this](const RequestContext& context)
                 {
                     return update(context);
                 });
}

platform::HttpResponse UserController::update(const RequestContext& context) const
{
    auto                path = InputValidator::parameters(context);
    const std::uint64_t id {path.requiredIdentifier("id")};
    path.validate();
    auto                           input = InputValidator::body(context.request);
    application::UpdateUserRequest request;
    if (input.has("name"))
    {
        request.name = input.requiredText("name", MIN_NAME, MAX_NAME);
    }
    request.role   = input.optionalTerm<domain::Role>("role", Vocabulary::role, ROLES);
    request.active = input.optionalFlag("active");
    if (input.has("password"))
    {
        request.password = input.requiredText("password", MIN_PASSWORD, MAX_PASSWORD);
    }
    input.validate();
    return platform::HttpResponse::ok(
        IdentityJsonPresenter::user(m_useCases.updateUser.execute(context.user(), id, request)));
}

} // namespace hubflow::identity::adapters
