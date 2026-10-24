#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_USER_HOLDER_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_USER_HOLDER_HPP

#include "hubflow/identity/domain/entities/user.hpp"

namespace hubflow::identity::application
{

    /// @brief Dueño temporal de un usuario reconstituido: lo libera al terminar el caso de uso.
    class UserHolder final
    {
    public:
        explicit UserHolder(domain::User* pUser)
            : m_pUser {pUser}
        {
        }

        ~UserHolder()
        {
            delete m_pUser;
        }

        UserHolder(const UserHolder&) = delete;
        UserHolder& operator=(const UserHolder&) = delete;

        domain::User& user() const noexcept
        {
            return *m_pUser;
        }

    private:
        domain::User* m_pUser;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_USER_HOLDER_HPP
