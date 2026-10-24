#include "hubflow/identity/adapters/in/http/identityJsonPresenter.hpp"

#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::identity::adapters
{

    platform::Json IdentityJsonPresenter::user(const domain::UserState& user)
    {
        return platform::Json {{"id", user.id},
                               {"email", user.email},
                               {"name", user.name},
                               {"role", shared::application::Vocabulary::name(user.role)},
                               {"active", user.active}};
    }

    platform::Json IdentityJsonPresenter::session(const application::Session& session)
    {
        return platform::Json {
            {"token", session.token}, {"expiresAt", session.expiresAt}, {"user", user(session.user)}};
    }

    platform::Json IdentityJsonPresenter::users(const ds::DynamicArray<domain::UserState>& userStates)
    {
        platform::Json items = platform::Json::array();
        for (std::size_t userIndex = 0; userIndex < userStates.size(); ++userIndex)
        {
            items.push_back(user(userStates[userIndex]));
        }
        return items;
    }

} // namespace hubflow::identity::adapters
