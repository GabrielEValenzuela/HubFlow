#include "hubflow/identity/adapters/in/http/sessionController.hpp"

#include "hubflow/identity/adapters/in/http/identityJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"

namespace hubflow::identity::adapters
{
namespace
{
constexpr std::size_t MAX_PASSWORD {72};
} // namespace

SessionController::SessionController(const application::SignInUseCase&   signIn,
                                     const application::FindUserUseCase& findUser)
    : m_signIn {signIn}
    , m_findUser {findUser}
{
}

void SessionController::registerRoutes(platform::ApiRouter& router)
{
    router.post(
        "/sessions",
        [this](const platform::RequestContext& context)
        {
            auto              input = platform::InputValidator::body(context.request);
            const std::string email {input.email("email")};
            const std::string password {input.requiredText("password", 1, MAX_PASSWORD)};
            input.validate();
            return platform::HttpResponse::created(IdentityJsonPresenter::session(m_signIn.execute(email, password)),
                                                   locationOf("/sessions/current"));
        },
        application::SignInUseCase::IDEMPOTENCY,
        platform::Access::Public);
    router.get("/sessions/current",
               [this](const platform::RequestContext& context)
               {
                   return platform::HttpResponse::ok(
                       IdentityJsonPresenter::user(m_findUser.execute(context.user(), context.user().userId)));
               });
}

} // namespace hubflow::identity::adapters
