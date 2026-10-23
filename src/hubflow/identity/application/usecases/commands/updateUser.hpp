#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_UPDATE_USER_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_UPDATE_USER_HPP

#include "hubflow/identity/application/ports/in/updateUserUseCase.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/identity/domain/services/courierWorkload.hpp"
#include "hubflow/identity/domain/services/passwordHasher.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::identity::application
{

/// @brief Implementación de UpdateUserUseCase.
class UpdateUser final : public UpdateUserUseCase
{
public:
    UpdateUser(const UserRepository&             users,
               const domain::PasswordHasher&     hasher,
               const domain::CourierWorkload&    workload,
               shared::application::EventBus&    bus,
               const shared::application::Clock& clock);

private:
    domain::UserState execute(const shared::application::Principal& principal,
                              std::uint64_t                         userId,
                              const UpdateUserRequest&              request) override;

    const UserRepository&             m_users;
    const domain::PasswordHasher&     m_hasher;
    const domain::CourierWorkload&    m_workload;
    shared::application::EventBus&    m_bus;
    const shared::application::Clock& m_clock;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_COMMANDS_UPDATE_USER_HPP
