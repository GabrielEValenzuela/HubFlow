#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_LIST_USERS_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_LIST_USERS_HPP

#include "hubflow/identity/application/ports/in/listUsersUseCase.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"

namespace hubflow::identity::application
{

    /// @brief Implementación de ListUsersUseCase.
    class ListUsers final : public ListUsersUseCase
    {
    public:
        explicit ListUsers(const UserRepository& users);

    private:
        ds::DynamicArray<domain::UserState> execute(const shared::application::Principal& principal) const override;

        const UserRepository& m_users;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_LIST_USERS_HPP
