#include "hubflow/routing/application/usecases/requireCourier.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::application
{

domain::Courier requireCourier(const identity::application::UserRepository& users, const std::uint64_t courierId)
{
    const auto user = users.byId(courierId);
    if (!user.has_value() || !user->active || user->role != identity::domain::Role::Pum)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation, "route.courier_invalid", "El repartidor no es un PUM activo"};
    }
    return domain::Courier {user->id, user->name};
}

} // namespace hubflow::routing::application
