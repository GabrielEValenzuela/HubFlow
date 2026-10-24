#include "hubflow/identity/application/subscribers/userPersistenceSubscriber.hpp"

#include "hubflow/identity/domain/events/userEvents.hpp"

namespace hubflow::identity::application
{

    UserPersistenceSubscriber::UserPersistenceSubscriber(UserRepository& repository)
        : m_repository {repository}
    {
    }

    void UserPersistenceSubscriber::on(const domain::UserRegistered& event)
    {
        m_repository.add(event.state());
    }

    void UserPersistenceSubscriber::on(const domain::UserProfileChanged& event)
    {
        m_repository.update(event.state());
    }

} // namespace hubflow::identity::application
