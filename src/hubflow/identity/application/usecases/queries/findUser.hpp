#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_FIND_USER_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_FIND_USER_HPP

#include "hubflow/identity/application/ports/in/findUserUseCase.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"

namespace hubflow::identity::application
{

/// @brief Implementación de FindUserUseCase.
class FindUser final : public FindUserUseCase
{
public:
    explicit FindUser(const UserRepository& users);

private:
    domain::UserState execute(const shared::application::Principal& principal, std::uint64_t userId) const override;

    const UserRepository& m_users;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_FIND_USER_HPP
