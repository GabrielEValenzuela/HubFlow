#include "infrastructure/http/controllers/identityControllers.hpp"

#include "application/shared/vocabulary.hpp"
#include "infrastructure/http/presenters/jsonPresenter.hpp"
#include "infrastructure/http/validation/inputValidator.hpp"

namespace hubflow::infrastructure
{
namespace
{
using application::Vocabulary;

constexpr const char* ROLES {"ADMIN, OPERADOR, PUM"};
constexpr std::size_t MIN_PASSWORD {8};
constexpr std::size_t MAX_PASSWORD {72};

Json usersJson(const ds::DynamicArray<domain::identity::UserState>& users)
{
    Json items = Json::array();
    for (std::size_t i = 0; i < users.size(); ++i)
    {
        items.push_back(JsonPresenter::user(users[i]));
    }
    return items;
}

class AuditCollector final : public application::AuditEntryVisitor
{
public:
    Json items = Json::array();

private:
    void visit(const application::AuditEntry& entry) override
    {
        items.push_back(JsonPresenter::auditEntry(entry));
    }
};
} // namespace

SessionController::SessionController(const application::SignIn& signIn, const application::UserQueries& users)
    : m_signIn {signIn}
    , m_users {users}
{
}

void SessionController::registerRoutes(ApiRouter& router)
{
    router.post(
        "/sessions",
        [this](const RequestContext& context)
        {
            auto              input = InputValidator::body(context.request);
            const std::string email {input.email("email")};
            const std::string password {input.requiredText("password", 1, MAX_PASSWORD)};
            input.validate();
            return HttpResponse::created(JsonPresenter::session(m_signIn.execute(email, password)), "/api/v1/sessions/current");
        },
        Access::Public);
    router.get("/sessions/current",
               [this](const RequestContext& context)
               { return HttpResponse::ok(JsonPresenter::user(m_users.byId(context.user(), context.user().userId))); });
}

UserController::UserController(application::RegisterUser&      registerUser,
                               application::UpdateUser&        updateUser,
                               const application::UserQueries& users)
    : m_registerUser {registerUser}
    , m_updateUser {updateUser}
    , m_users {users}
{
}

void UserController::registerRoutes(ApiRouter& router)
{
    router.get("/users",
               [this](const RequestContext& context)
               {
                   auto input           = InputValidator::parameters(context);
                   const auto [limit, offset] = input.page();
                   input.validate();
                   return HttpResponse::ok(page(usersJson(m_users.all(context.user())), limit, offset));
               });
    router.get("/couriers",
               [this](const RequestContext& context)
               { return HttpResponse::ok(Json {{"items", usersJson(m_users.couriers(context.user()))}}); });
    router.post("/users",
                [this](const RequestContext& context)
                {
                    auto                             input = InputValidator::body(context.request);
                    application::RegisterUserRequest request;
                    request.email    = input.email("email");
                    request.name     = input.requiredText("name", 2, 80);
                    request.role     = input.requiredTerm<domain::identity::Role>("role", Vocabulary::role, ROLES);
                    request.password = input.requiredText("password", MIN_PASSWORD, MAX_PASSWORD);
                    input.validate();
                    const auto user = m_registerUser.execute(context.user(), request);
                    return HttpResponse::created(JsonPresenter::user(user), "/api/v1/users/" + std::to_string(user.id));
                });
    router.get("/users/:id",
               [this](const RequestContext& context)
               {
                   auto                input = InputValidator::parameters(context);
                   const std::uint64_t id {input.requiredIdentifier("id")};
                   input.validate();
                   return HttpResponse::ok(JsonPresenter::user(m_users.byId(context.user(), id)));
               });
    router.patch("/users/:id",
                 [this](const RequestContext& context)
                 {
                     auto                path = InputValidator::parameters(context);
                     const std::uint64_t id {path.requiredIdentifier("id")};
                     path.validate();
                     auto                           input = InputValidator::body(context.request);
                     application::UpdateUserRequest request;
                     if (input.has("name"))
                     {
                         request.name = input.requiredText("name", 2, 80);
                     }
                     request.role   = input.optionalTerm<domain::identity::Role>("role", Vocabulary::role, ROLES);
                     request.active = input.optionalFlag("active");
                     if (input.has("password"))
                     {
                         request.password = input.requiredText("password", MIN_PASSWORD, MAX_PASSWORD);
                     }
                     input.validate();
                     return HttpResponse::ok(JsonPresenter::user(m_updateUser.execute(context.user(), id, request)));
                 });
}

AuditController::AuditController(const application::AuditQueries& audit)
    : m_audit {audit}
{
}

void AuditController::registerRoutes(ApiRouter& router)
{
    router.get("/audit-events",
               [this](const RequestContext& context)
               {
                   auto input           = InputValidator::parameters(context);
                   const auto [limit, offset] = input.page();
                   input.validate();
                   AuditCollector collector;
                   m_audit.recent(context.user(), limit, offset, collector);
                   return HttpResponse::ok(Json {{"items", collector.items},
                                                 {"total", m_audit.count(context.user())},
                                                 {"limit", limit},
                                                 {"offset", offset}});
               });
}

} // namespace hubflow::infrastructure
