#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_USER_COLLECTOR_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_USER_COLLECTOR_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"

namespace hubflow::identity::application
{

    /// @brief Junta los usuarios que entrega el repositorio, opcionalmente sólo los PUM activos.
    class UserCollector final : public UserVisitor
    {
    public:
        explicit UserCollector(const bool onlyActiveCouriers)
            : m_onlyActiveCouriers {onlyActiveCouriers}
        {
        }

        ds::DynamicArray<domain::UserState>& users() noexcept
        {
            return m_users;
        }

    private:
        void visit(const domain::UserState& state) override
        {
            if (!m_onlyActiveCouriers || (state.active && state.role == domain::Role::Pum))
            {
                m_users.pushBack(state);
            }
        }

        bool m_onlyActiveCouriers;
        ds::DynamicArray<domain::UserState> m_users;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_USER_COLLECTOR_HPP
