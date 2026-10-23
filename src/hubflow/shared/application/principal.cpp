#include "hubflow/shared/application/principal.hpp"

namespace hubflow::shared::application
{

void Principal::require(const identity::domain::Permission permission) const
{
    identity::domain::AccessPolicy::require(role, permission);
}

domain::ActorContext Principal::contextAt(const Clock& clock) const
{
    return domain::ActorContext {userId, name, clock.now()};
}

} // namespace hubflow::shared::application
