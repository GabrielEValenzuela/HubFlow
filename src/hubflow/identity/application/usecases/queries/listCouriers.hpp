#ifndef HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_LIST_COURIERS_HPP
#define HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_LIST_COURIERS_HPP

#include "hubflow/identity/application/ports/in/listCouriersUseCase.hpp"
#include "hubflow/identity/application/ports/out/userRepository.hpp"

namespace hubflow::identity::application
{

/// @brief Implementación de ListCouriersUseCase.
class ListCouriers final : public ListCouriersUseCase
{
public:
    explicit ListCouriers(const UserRepository& users);

private:
    ds::DynamicArray<domain::UserState> execute(const shared::application::Principal& principal) const override;

    const UserRepository& m_users;
};

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_USECASES_QUERIES_LIST_COURIERS_HPP
