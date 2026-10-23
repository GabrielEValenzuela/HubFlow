#ifndef HUBFLOW_SHARED_APPLICATION_PRINCIPAL_HPP
#define HUBFLOW_SHARED_APPLICATION_PRINCIPAL_HPP

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"
#include "hubflow/shared/domain/actorContext.hpp"

#include <string>

namespace hubflow::shared::application
{

/// @brief Usuario autenticado que invoca un caso de uso.
struct Principal
{
    std::uint64_t          userId {0};
    std::string            name;
    identity::domain::Role role {identity::domain::Role::Operador};

    /// @brief Política con la que se evalúan los permisos; debe sobrevivir al Principal.
    const identity::domain::AccessPolicy* pPolicy {nullptr};

    /// @throws business_error (Forbidden) si el rol no tiene @p permission según la política (o si no hay política).
    void require(identity::domain::Permission permission) const;

    /// @brief Contexto de dominio (autor + instante) para la operación actual.
    domain::ActorContext contextAt(const Clock& clock) const;
};

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_PRINCIPAL_HPP
