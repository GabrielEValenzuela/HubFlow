#include "hubflow/shared/application/principal.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::shared::application
{

    void Principal::require(const identity::domain::Permission permission) const
    {
        if (pPolicy == nullptr)
        {
            throw domain::BusinessError {
                domain::ErrorKind::Forbidden, "auth.forbidden", "No tenés permiso para esta operación"};
        }
        pPolicy->require(role, permission);
    }

    domain::ActorContext Principal::contextAt(const Clock& clock) const
    {
        return domain::ActorContext {userId, name, clock.now()};
    }

} // namespace hubflow::shared::application
