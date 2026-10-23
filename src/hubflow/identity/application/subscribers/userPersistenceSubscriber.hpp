#ifndef HUBFLOW_IDENTITY_APPLICATION_SUBSCRIBERS_USER_PERSISTENCE_SUBSCRIBER_HPP
#define HUBFLOW_IDENTITY_APPLICATION_SUBSCRIBERS_USER_PERSISTENCE_SUBSCRIBER_HPP

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::identity::application
{

/// @brief Persiste altas y cambios de usuarios a partir de sus eventos.
class UserPersistenceSubscriber final : public shared::domain::DomainEventVisitor
{
public:
    explicit UserPersistenceSubscriber(UserRepository& repository);

private:
    void on(const domain::UserRegistered& event) override;
    void on(const domain::UserProfileChanged& event) override;

    UserRepository& m_repository;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_SUBSCRIBERS_USER_PERSISTENCE_SUBSCRIBER_HPP
